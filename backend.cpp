#include "backend.h"
#include "mapmodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QLoggingCategory>
#include <QTimer>

namespace GameOne {

namespace {
Q_LOGGING_CATEGORY(lcBackend, "GameOne.backend");
} // namespace

Backend::Backend(QObject *parent)
    : QObject{parent}
    , m_timer{new QTimer{this}}
    , m_map{new MapModel{this}}
{
    m_timer->setInterval(100);
    connect(m_timer, &QTimer::timeout, this, &Backend::onTimeout);

    connect(m_map, &MapModel::columnsChanged, this, &Backend::columnsChanged);
    connect(m_map, &MapModel::rowsChanged, this, &Backend::rowsChanged);

    load("level1.json");
}

int Backend::columns() const
{
    return m_map->columns();
}

int Backend::rows() const
{
    return m_map->rows();
}

QList<Actor *> Backend::actors() const
{
    QList<Actor *> actors;
    actors.reserve(m_enemies.size() + 1);

    if (m_player)
        actors += m_player.get();
    for (auto enemy: m_enemies)
        actors += enemy.get();

    return actors;
}

QList<Enemy *> Backend::enemies() const
{
    QList<Enemy *> enemies;
    enemies.reserve(m_enemies.size());

    for (auto enemy: m_enemies)
        enemies += enemy.get();

    return enemies;
}

bool Backend::load(QString fileName)
{
    qCInfo(lcBackend, "Loading level from %ls", qUtf16Printable(fileName));

    m_timer->stop();

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

        m_enemies.clear();

        for (const auto value: level["enemies"].toArray())
            m_enemies += std::make_shared<Enemy>(value.toObject(), this);

        const auto playerData = level["player"].toObject();
        m_player = std::make_unique<Player>(playerData, this);

        connect(m_player.get(), &Player::positionChanged, this, [this] { m_timer->start(); });
        connect(m_player.get(), &Player::livesChanged, this, [this] { m_timer->stop(); });

        emit actorsChanged();
        emit enemiesChanged();
        emit playerChanged(m_player.get());
        emit levelFileNameChanged(m_levelFileName);
        emit levelNameChanged(m_levelName);
        emit columnsChanged(columns());
        emit rowsChanged(rows());

        return true;
    } else if (fileName.endsWith(".txt")) {
        return load("level1.json") && m_map->load(fileName, MapModel::LegacyFormat);
    } else {
        qCWarning(lcBackend, "Unsupported filename: %ls", qUtf16Printable(fileName));
        return false;
    }
}

void Backend::respawn()
{
    if (m_player)
        m_player->respawn();

    m_timer->stop();
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
                actor->giveEnergy(actor->attack(opponent));

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

void Backend::onTimeout()
{
    for (auto enemy: m_enemies)
        enemy->act();
}

} // namespace GameOne
