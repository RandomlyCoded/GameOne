#include "backend.h"
#include "inventorymodel.h"
#include "mapmodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QLoggingCategory>
#include <QTimer>

using namespace std::chrono_literals;

namespace GameOne {

namespace {
Q_LOGGING_CATEGORY(lcBackend, "GameOne.backend");
} // namespace

Backend::Backend(QObject *parent)
    : QObject{parent}
    , m_actionTimer{new QTimer{this}}
    , m_ticksTimer{new QTimer{this}}
    , m_map{new MapModel{this}}
{
    connect(m_actionTimer, &QTimer::timeout, this, &Backend::onActionTimeout);
    m_actionTimer->setInterval(100ms);

    connect(m_ticksTimer, &QTimer::timeout, this, &Backend::onTicksTimeout);
    m_ticksTimer->start(100ms);

    connect(m_map, &MapModel::columnsChanged, this, &Backend::columnsChanged);
    connect(m_map, &MapModel::rowsChanged, this, &Backend::rowsChanged);

    loadItems();
}

int Backend::columns() const
{
    return m_map->columns();
}

int Backend::rows() const
{
    return m_map->rows();
}

qint64 Backend::ticks() const
{
    return m_ticks.elapsed() / m_ticksTimer->interval();
}

QList<Actor *> Backend::actors() const
{
    return m_actors;
}

QList<Enemy *> Backend::enemies() const
{
    QList<Enemy *> enemies;
    enemies.reserve(m_enemies.size());

    for (const auto &enemy: m_enemies)
        enemies += enemy.get();

    return enemies;
}

InventoryItem *Backend::item(QString id) const
{
    return m_items.value(id);
}

bool Backend::load(QString fileName, std::optional<QPoint> playerPosition)
{
    qCInfo(lcBackend, "Loading level from %ls", qUtf16Printable(fileName));

    m_actionTimer->stop();

    fileName = dataFileName(std::move(fileName));
    if (fileName.endsWith(".json")) {
        QFile file{fileName};

        if (!file.open(QFile::ReadOnly)) {
            qCWarning(lcBackend, "Could not open %ls: %ls", qUtf16Printable(fileName), qUtf16Printable(file.errorString()));
            return false;
        }

        QJsonParseError status;
        const auto level = QJsonDocument::fromJson(file.readAll(), &status).object();

        if (status.error != QJsonParseError::NoError) {
            qCWarning(lcBackend, "Could not read %ls: %ls", qUtf16Printable(fileName), qUtf16Printable(status.errorString()));
            return false;
        }

        const auto mapData = level["map"].toObject();
        if (!m_map->load(mapData["filename"].toString(), static_cast<MapModel::Format>(mapData["format"].toInt())))
            return false;

        m_levelFileName = fileName;
        m_levelName = level["levelName"].toString();

        if (m_levelName.isEmpty())
            m_levelName = QFileInfo{file.fileName()}.baseName();

        m_actors.clear();
        m_chests.clear();
        m_ladders.clear();
        m_enemies.clear();

        for (const auto chests = level["chests"].toArray(); const auto &value: chests)
            m_chests += std::make_shared<Chest>(resolve(value.toObject()), this);
        for (const auto ladders = level["ladders"].toArray(); const auto &value: ladders)
            m_ladders += std::make_shared<Ladder>(resolve(value.toObject()), this);
        for (const auto enemies = level["enemies"].toArray(); const auto &value: enemies)
            m_enemies += std::make_shared<Enemy>(resolve(value.toObject()), this);

        const auto playerData = level["player"].toObject();
        m_player = std::make_unique<Player>(std::move(playerData), this);

        if (playerPosition.has_value())
            m_player->moveTo(*playerPosition);

        m_actors += m_player.get();
        const auto toRawPointer = [](auto ptr) { return ptr.get(); };
        std::transform(m_enemies.begin(), m_enemies.end(), std::back_inserter(m_actors), toRawPointer);
        std::transform(m_ladders.begin(), m_ladders.end(), std::back_inserter(m_actors), toRawPointer);
        std::transform(m_chests.begin(), m_chests.end(), std::back_inserter(m_actors), toRawPointer);

        connect(m_player.get(), &Player::positionChanged, this, [this] { m_actionTimer->start(); });
        connect(m_player.get(), &Player::livesChanged, this, [this] { m_actionTimer->stop(); });

        emit actorsChanged();
        emit enemiesChanged();
        emit playerChanged(m_player.get());
        emit levelFileNameChanged(m_levelFileName);
        emit levelNameChanged(m_levelName);
        emit columnsChanged(columns());
        emit rowsChanged(rows());

        return true;
    } else if (fileName.endsWith(".txt")) {
        return load(levelFileName(1)) && m_map->load(fileName, MapModel::LegacyFormat);
    } else {
        qCWarning(lcBackend, "Unsupported filename: %ls", qUtf16Printable(fileName));
        return false;
    }
}

void Backend::respawn()
{
    if (m_player)
        m_player->respawn();

    m_actionTimer->stop();
}

bool Backend::canMoveTo(Actor *actor, QPoint destination) const
{
    if (!actor->isAlive())
        return false;

    if (destination.x() < 0 || destination.x() >= columns())
        return false;
    if (destination.y() < 0 || destination.y() >= rows())
        return false;
    if (!m_map->dataByPoint(destination, MapModel::WalkableRole).toBool())
        return false;

    for (auto opponent: actors()) {
        if (opponent == actor)
            continue;
        if (!opponent->isAlive())
            continue;

        if (destination == opponent->position()) {
            if (actor->canAttack(opponent))
                opponent->giveBonus(actor, actor->attack(opponent));

            return false;
        }
    }

    return true;
}

QDir Backend::dataDir()
{
    return {":/GameOne/data"};
}

QString Backend::dataFileName(QString fileName)
{
    return dataDir().filePath(fileName);
}

QUrl Backend::imageUrl(QString fileName)
{
    return imageUrl(QUrl{std::move(fileName)});
}

QUrl Backend::imageUrl(QUrl imageUrl)
{
    if (imageUrl.isEmpty())
        return {};

    return QUrl{"image://assets/"}.resolved(std::move(imageUrl));
}

QUrl Backend::imageUrl(QUrl imageUrl, int imageCount, qint64 tick)
{
    if (imageCount > 1)
        imageUrl.setQuery(imageUrl.query().replace("(t)", QString::number(tick % imageCount)));

    return imageUrl;
}

QString Backend::levelFileName(int index)
{
    return QString::number(index) + ".level.json";
}

void Backend::loadItems()
{
    // TODO: Load from JSON
    m_items["Arrow"] = new InventoryItem{"Arrow", QUrl{"weapons/Arrow.svg"}, this};
    m_items["BottleLargeEmpty"] = new InventoryItem{"empty large bottle", QUrl{"inventory/BottleLargeEmpty.svg"}, this};
    m_items["BottleLargeEndurancedrink"] = new InventoryItem{"large endurancedrink", QUrl{"inventory/BottleLargeEndurancedrink.svg"}, this};
    m_items["BottleLargeEnergydrink"] = new InventoryItem{"large energydrink", QUrl{"inventory/BottleLargeEnergydrink.svg"}, this};
    m_items["BottleSmallEmpty"] = new InventoryItem{"empty small bottle", QUrl{"inventory/BottleSmallEmpty.svg"}, this};
    m_items["BottleSmallEndurancedrink"] = new InventoryItem{"small endurancedrink", QUrl{"inventory/BottleSmallEndurancedrink.svg"}, this};
    m_items["BottleSmallEnergydrink"] = new InventoryItem{"small energydrink", QUrl{"inventory/BottleSmallEnergydrink.svg"}, this};
    m_items["Bow"] = new InventoryItem{"Bow", QUrl{"weapons/Bow.svg"}, this};
    m_items["BowBroken"] = new InventoryItem{"broken Bow", QUrl{"inventory/BowBroken.svg"}, this};
    m_items["Dagger"] = new InventoryItem{"Dagger", QUrl{"weapons/Dagger.svg"}, this};
    m_items["FireArrow"] = new InventoryItem{"Fire Arrow", QUrl{"weapons/FireArrow.svg"}, this};
    m_items["IceArrow"] = new InventoryItem{"Ice Arrow", QUrl{"weapons/IceArrow.svg"}, this};
    m_items["LightningArrow"] = new InventoryItem{"electric Arrow", QUrl{"weapons/LightningArrow.svg"}, this};
    m_items["LongSword"] = new InventoryItem{"Long Sword", QUrl{"weapons/LongSword.svg"}, this};
    m_items["SmallSword"] = new InventoryItem{"Small Sword", QUrl{"weapons/SmallSword.svg"}, this};
    m_items["StarArrow"] = new InventoryItem{"Star Arrow", QUrl{"weapons/StarArrow.svg"}, this};
    m_items["TwoHandSword"] = new InventoryItem{"Two Hand Sword", QUrl{"weapons/TwohandSword.svg"}, this};
    m_items[""] = new InventoryItem{{}, QUrl{}, this};
}

void Backend::onActionTimeout()
{
    for (auto enemy: qAsConst(m_enemies))
        enemy->act();
}

void Backend::onTicksTimeout()
{
    emit ticksChanged(ticks());
}

QJsonDocument Backend::cachedDocument(QUrl url) const
{
    if (const auto it = m_jsonCache.find(url); it != m_jsonCache.end())
        return *it;

    if (url.scheme() != "qrc") {
        qCWarning(lcBackend, "%ls: Unsupported URL", qUtf16Printable(url.toDisplayString()));
        return {};
    }

    QFile file{":" + url.path()};

    if (!file.open(QFile::ReadOnly)) {
        qCWarning(lcBackend, "%ls: %ls", qUtf16Printable(url.toDisplayString()), qUtf16Printable(file.errorString()));
        return {};
    }

    QJsonParseError status;
    const auto document = QJsonDocument::fromJson(file.readAll(), &status);

    if (status.error != QJsonParseError::NoError) {
        qCWarning(lcBackend, "%ls: %ls", qUtf16Printable(url.toDisplayString()), qUtf16Printable(status.errorString()));
        return {};
    }

    return *m_jsonCache.insert(url, document);
}

QJsonObject Backend::resolve(QJsonObject object) const
{
    QUrl ref{object["$ref"].toString()};

    if (!ref.isEmpty()) {
        if (ref.path().isEmpty()) {
            ref.setPath("/GameOne/data/basics.json");
            ref.setScheme("qrc");
        }

        auto json = cachedDocument(ref).object();
        if (const auto id = ref.fragment(); !id.isEmpty())
            json = resolve(json[id].toObject());

        for (auto it = json.begin(); it != json.end(); ++it) {
            if (!object.contains(it.key()))
                object.insert(it.key(), it.value());
        }

        object.remove("$ref");
    }

    return object;
}

} // namespace GameOne

#include "moc_backend.cpp"
