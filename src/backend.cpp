#include "backend.h"
#include "inventorymodel.h"
#include "mapmodel.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QTimer>

using namespace std::chrono_literals;

namespace GameOne {

namespace {

Q_LOGGING_CATEGORY(lcBackend, "GameOne.backend");

QJsonDocument readJson(const QString &fileName)
{
    auto file = QFile{fileName};

    if (!file.open(QFile::ReadOnly)) {
        qCWarning(lcBackend, "Could not open %ls: %ls",
                  qUtf16Printable(fileName),
                  qUtf16Printable(file.errorString()));

        return {};
    }

    auto status = QJsonParseError{};
    auto document = QJsonDocument::fromJson(file.readAll(), &status);

    if (status.error != QJsonParseError::NoError) {
        qCWarning(lcBackend, "Could not read %ls: %ls",
                  qUtf16Printable(fileName),
                  qUtf16Printable(status.errorString()));

        return {};
    }

    return document;
}

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

    loadItemTypes();
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

InventoryItem *Backend::item(const QString &id) const
{
    return m_items.value(id);
}

bool Backend::load(QString fileName, std::optional<QPoint> playerPosition)
{
    qInfo() << "loading" << fileName;

    m_actionTimer->stop();

    fileName = dataFileName(fileName);

    if (fileName.endsWith(".json")) {
        const auto &level = readJson(fileName);

        if (!level.isObject())
            return false;

        const auto mapData = level["map"].toObject();
        const auto mapFileName = mapData["filename"].toString();

        if (!m_map->load(mapFileName, static_cast<MapModel::Format>(mapData["format"].toInt())))
            return false;

        m_levelFileName = fileName;
        m_levelName = level["levelName"].toString();

        if (m_levelName.isEmpty())
            m_levelName = QFileInfo{fileName}.baseName();

        loadItems(level.object(), playerPosition);
        validateActors(fileName, mapFileName);

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
    }

    if (fileName.endsWith(".txt")) {
        return load(LevelModel::levelFileName(LevelModel::DEFAULT_LEVEL))
               && m_map->load(fileName, MapModel::LegacyFormat);
    }

    qCWarning(lcBackend, "Unsupported filename: %ls", qUtf16Printable(fileName));
    return false;
}

void Backend::loadItems(const QJsonObject &level, const std::optional<QPoint> &playerPosition)
{
    m_actors.clear();
    m_chests.clear();
    m_ladders.clear();
    m_enemies.clear();

    const auto chests = level["chests"].toArray();
    const auto ladders = level["ladders"].toArray();
    const auto enemies = level["enemies"].toArray();
    const auto tentaklons = level["tentaklons"].toArray();

    for (const auto &value: chests)
        m_chests += std::make_shared<Chest>(resolve(value.toObject()), this);
    for (const auto &value: ladders)
        m_ladders += std::make_shared<Ladder>(resolve(value.toObject()), this);
    for (const auto &value: enemies)
        m_enemies += std::make_shared<Enemy>(resolve(value.toObject()), this);
    for (const auto &value: tentaklons)
        m_enemies += std::make_shared<Tentaklon>(resolve(value.toObject()), this);

    const auto playerData = resolve(level["player"].toObject());
    m_player = std::make_unique<Player>(playerData, this);

    if (playerPosition)
        m_player->moveTo(*playerPosition);

    const auto toRawPointer = [](auto ptr) { return ptr.get(); };
    std::transform(m_ladders.begin(), m_ladders.end(), std::back_inserter(m_actors), toRawPointer);
    std::transform(m_chests.begin(), m_chests.end(), std::back_inserter(m_actors), toRawPointer);
    std::transform(m_enemies.begin(), m_enemies.end(), std::back_inserter(m_actors), toRawPointer);
    m_actors += m_player.get();
}

void Backend::respawn()
{
    if (m_player)
        m_player->respawn();

    m_actionTimer->stop();
}

bool Backend::canMoveTo(Actor *actor, QPoint destination) const
{
    if (actor == m_player.get())
        m_actionTimer->start();
    if (!actor->isAlive())
        return false;

    if (destination.x() < 0 || destination.x() >= columns())
        return false;
    if (destination.y() < 0 || destination.y() >= rows())
        return false;
    if (!m_map->dataByPoint(destination, MapModel::WalkableRole).toBool())
        return false;

    for (const auto &opponentList = actors(); const auto &opponent : opponentList) {
        if (opponent == actor)
            continue;
        if (!opponent->isAlive())
            continue;

        if (destination == opponent->position()) {
            if (opponent->energy() == opponent->minimumEnergy())
                return true;

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

QString Backend::dataFileName(const QString &fileName)
{
    return dataDir().filePath(fileName);
}

QUrl Backend::imageUrl(const QString &fileName)
{
    return imageUrl(QUrl{fileName});
}

QUrl Backend::imageUrl(const QUrl &imageUrl)
{
    if (imageUrl.isEmpty())
        return {};

    return QUrl{"image://assets/"}.resolved(imageUrl);
}

QUrl Backend::imageUrl(QUrl imageUrl, int imageCount, qint64 tick)
{
    if (imageCount > 1) {
        static const auto pattern = QRegularExpression{R"(\(t([+-]\d+)?\))"};
        const auto inputQueryString = imageUrl.query();

        auto start = qsizetype{0};
        QString outputQueryString;

        for (auto it = pattern.globalMatch(inputQueryString); it.hasNext(); ) {
            const auto match = it.next();

            outputQueryString += inputQueryString.mid(start, match.capturedStart() - start);
            outputQueryString += QString::number((tick + match.captured(1).toInt()) % imageCount);
            start = match.capturedEnd();
        }

        outputQueryString += inputQueryString.mid(start);
        imageUrl.setQuery(outputQueryString);
    }

    return imageUrl;
}

void Backend::loadItemTypes()
{
    // TODO: Load from JSON
    m_items["Arrow"] = new InventoryItem{"Arrow", QUrl{"weapons/Arrow.svg"}, this};
    m_items["BottleLargeEmpty"] = new InventoryItem{"empty large bottle", QUrl{"inventory/BottleLargeEmpty.svg"}, this};
    m_items["BottleLargeEndurancedrink"] = new InventoryItem{"large endurancedrink", QUrl{"inventory/BottleLargeEndurancedrink.svg"}, this};
    m_items["BottleLargeEnergydrink"] = new InventoryItem{"large energydrink", QUrl{"inventory/BottleLargeEnergydrink.svg"}, this};
    m_items["BottleSmallEmpty"] = new InventoryItem{"empty small bottle", QUrl{"inventory/BottleSmallEmpty.svg"}, this};
    m_items["BottleSmallEndurancedrink"] = new InventoryItem{"Endurance", QUrl{"inventory/BottleSmallEndurancedrink.svg"}, this};
    m_items["BottleSmallEnergydrink"] = new InventoryItem{"small bottle of Energy", QUrl{"inventory/BottleSmallEnergydrink.svg"}, this};
    m_items["Bow"] = new InventoryItem{"Bow", QUrl{"weapons/Bow.svg"}, this};
    m_items["BowBroken"] = new InventoryItem{"broken Bow", QUrl{"inventory/BowBroken.svg"}, this};
    m_items["Dagger"] = new InventoryItem{"Dagger", QUrl{"weapons/Dagger.svg"}, this};
    m_items["FireArrow"] = new InventoryItem{"Fire Arrow", QUrl{"weapons/FireArrow.svg"}, this};
    m_items["IceArrow"] = new InventoryItem{"Ice Arrow", QUrl{"weapons/IceArrow.svg"}, this};
    m_items["LightningArrow"] = new InventoryItem{"electric Arrow", QUrl{"weapons/LightningArrow.svg"}, this};
    m_items["LongSword"] = new InventoryItem{"Long Sword", QUrl{"weapons/LongSword.svg"}, this};
    m_items["SmallSword"] = new InventoryItem{"Small Sword", QUrl{"weapons/SmallSword.svg"}, this};
    m_items["StarArrow"] = new InventoryItem{"Star Arrow", QUrl{"weapons/StarArrow.svg"}, this}; // weapons/StarArrow.svg
    m_items["TwoHandSword"] = new InventoryItem{"Two Hand Sword", QUrl{"weapons/TwohandSword.svg"}, this};
    m_items[""] = new InventoryItem{{}, QUrl{}, this};
}

void Backend::validateActors(const QString &levelFileName, const QString &mapFileName) const
{
    QHash<int, QString> actorTypes;

    // verify that actors are declared in the map
    for (auto *const actor : m_actors) {
        const auto mapIndex = m_map->indexByPoint(actor->position());
        const auto itemType = mapIndex.data(MapModel::ItemTypeRole).toString();

        if (itemType.isEmpty()) {
            qCWarning(lcBackend,
                      "%ls: No item at supposed start position (%d,%d) of %ls \"%ls\"",
                      qUtf16Printable(levelFileName), actor->x(), actor->y(),
                      qUtf16Printable(actor->type()), qUtf16Printable(actor->name()));
        } else if (actor->type() != itemType) {
            qCWarning(lcBackend,
                      "%ls: Unexpected item of type %ls at start position (%d,%d) of %ls \"%ls\"",
                      qUtf16Printable(levelFileName), qUtf16Printable(itemType), actor->x(), actor->y(),
                      qUtf16Printable(actor->type()), qUtf16Printable(actor->name()));
        }

        actorTypes.insert(mapIndex.row(), actor->type());
    }

    // verify that actors declared in the map also are declared in the JSON
    for (auto row = 0, rowCount = m_map->rowCount(); row < rowCount; ++row) {
        const auto mapIndex =  m_map->index(row);
        const auto isStart = mapIndex.data(MapModel::IsStartRole).toBool();

        if (!isStart)
            continue;

        const auto itemType = mapIndex.data(MapModel::ItemTypeRole).toString();

        if (actorTypes[mapIndex.row()] != itemType) {
            const auto position = mapIndex.data(MapModel::PositionRole).toPoint();
            m_map->setData(mapIndex, false, MapModel::IsStartRole);

            qCWarning(lcBackend,
                      "%ls: Map contains a start position of an actor of type %ls "
                      "at (%d,%d), but the details are missing in the JSON",
                      qUtf16Printable(mapFileName), qUtf16Printable(itemType),
                      position.x(), position.y());
        }
    }
}

void Backend::onActionTimeout()
{
    for (const auto &enemy : std::as_const(m_enemies))
        enemy->act();
}

void Backend::onTicksTimeout()
{
    emit ticksChanged(ticks());
}

QJsonDocument Backend::cachedDocument(const QUrl &url) const
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
    if (const auto ref = object.find("$ref"); ref != object.end()) {
        auto json = resolve(ref->toString());
        object.erase(ref);

        for (auto it = json.begin(); it != json.end(); ++it) {
            if (!object.contains(it.key()))
                object.insert(it.key(), it.value());
        }
    }

    return object;
}

QJsonObject Backend::resolve(QUrl ref) const
{
    static const auto s_basicsUrl = QUrl{"qrc:/GameOne/data/basics.json"};

    if (ref.isEmpty())
        return {};

    if (ref.path().isEmpty()) {
        ref.setScheme(s_basicsUrl.scheme());
        ref.setPath(s_basicsUrl.path());
    } else if (ref.isRelative()) {
        ref = s_basicsUrl.resolved(ref);
    }

    const auto path = ref.fragment().split('/', Qt::SkipEmptyParts);
    auto object = cachedDocument(ref).object();

    for (const auto &component : path)
        object = resolve(object[component].toObject());

    return object;
}

} // namespace GameOne

#include "moc_backend.cpp"
