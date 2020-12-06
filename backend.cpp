#include "backend.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QTimer>

namespace GameOne {

namespace {

Q_LOGGING_CATEGORY(lcBackend, "GameOne.backend");
Q_LOGGING_CATEGORY(lcMap, "GameOne.map");

auto dataFileName(QString fileName)
{
    return QDir{":/GameOne/data"}.filePath(fileName);
}

} // namespace

Actor::Actor(QJsonObject spec, Backend *backend)
    : QObject{backend}
    , m_name{spec["name"].toString()}
    , m_origin{spec["x"].toInt(), spec["y"].toInt()}
    , m_position{m_origin}
    , m_maximumEnergy{qMax(spec["maximumEnergy"].toInt(), 1)}
    , m_maximumLifes{qMax(spec["maximumLifes"].toInt(), 1)}
    , m_lives{m_maximumLifes}
{
    respawn();
}

void Actor::setName(QString name)
{
    if (std::exchange(m_name, name) != name)
        emit nameChanged(m_name);
}

QString Actor::name() const
{
    if (m_name.isEmpty())
        return tr("no name");

    return m_name;
}

Backend *Actor::backend() const
{
    return static_cast<Backend *>(parent());
}

void Actor::tryMoveTo(QPoint destination)
{
    if (backend()->canMoveTo(this, destination)) {
        m_position = destination;
        emit positionChanged(m_position);
    }
}

void Actor::moveLeft()
{
    tryMoveTo(position() + QPoint{-1, 0});
}

void Actor::moveUp()
{
    tryMoveTo(position() + QPoint{0, -1});
}

void Actor::moveDown()
{
    tryMoveTo(position() + QPoint{0, +1});
}

void Actor::moveRight()
{
    tryMoveTo(position() + QPoint{+1, 0});
}

void Actor::respawn()
{
    m_energy = m_maximumEnergy;
    m_position = m_origin;

    emit positionChanged(m_position);
    emit energyChanged(m_energy);
}

int Actor::attack(Actor *)
{
    return 0;
}

void Actor::stealEnergy(int amount)
{
    m_energy = qMax(0, m_energy - amount);
    emit energyChanged(m_energy);

    if (m_energy == 0)
        die();
}

void Actor::giveEnergy(int amount)
{
    m_energy = qMin(m_maximumEnergy, m_energy + amount);
    emit energyChanged(m_energy);
}

void Actor::die()
{
    if (m_lives > 0) {
        --m_lives;
        emit livesChanged(m_lives);
    }
}

QString Enemy::type() const
{
    return "Enemy";
}

bool Enemy::canAttack(const Actor *opponent) const
{
    return dynamic_cast<const Player *>(opponent);
}

int Enemy::attack(Actor *opponent)
{
    if (canAttack(opponent)) {
        opponent->stealEnergy(1);
        return std::rand() % 2; // FIXME: use proper generator from std::random
    }

    return 0;
}

void Enemy::act()
{
    switch (std::rand() % 4) { // FIXME: use proper generator from std::random
    case 0:
        if (backend()->player()->x() < x())
            moveLeft();

        break;

    case 1:
        if (backend()->player()->y() < y())
            moveUp();

        break;

    case 2:
        if (backend()->player()->x() > x())
            moveRight();

        break;

    case 3:
        if (backend()->player()->y() > y())
            moveDown();

        break;
    }
}

QString Player::type() const
{
    return "Player";
}

bool Player::canAttack(const Actor *opponent) const
{
    return dynamic_cast<const Enemy *>(opponent);
}

int Player::attack(Actor *opponent)
{
    if (canAttack(opponent)) {
        opponent->stealEnergy(1);
        return std::rand() % 2; // FIXME: use proper generator from std::random
    }

    return 0;
}

MapModel::Tile::Tile(QByteArray spec)
{
    auto tspec = spec[0];
    auto ispec = spec.length() > 1 ? spec[1] : ' ';

    if (tspec == 'T') {
        tspec = 'G';
        ispec = '@';
    } else if (tspec == 'F') {
        tspec = 'G';
        ispec = '#';
    }

    static const QHash<char, Type> types = {
        {'G',   {"Grass",       true}},
        {'W',   {"DeepWater",   false}},
        {'w',   {"Water",       true}},
        {'H',   {"Hill",        true}},
        {'M',   {"Mountain",    false}},
        {'S',   {"Sand",        true}},
        {'I',   {"Ice",         true}},
        {'L',   {"Lava",        true}},
        {'@',   {"Tree",        false}},
        {'#',   {"Fence",       false}},
        {'-',   {"Fence",       false}},
        {'|',   {"Fence",       false}},
        {'/',   {"Fence",       false}},
        {'\\',  {"Fence",       false}},
    };

    type = types.value(tspec);
    item = types.value(ispec);
}

bool MapModel::Tile::isWalkable() const
{
    if (item.isValid() && !item.walkable)
        return false;

    return type.walkable;
}

QVariant MapModel::data(const QModelIndex &index, int role) const
{
    if (hasIndex(index.row(), index.column(), index.parent())) {
        const auto row = index.row() / m_columns;
        const auto column = index.row() % m_columns;
        const auto tile = m_tiles[index.row()];

        switch (static_cast<Role>(role)) {
        case Column:
            return column;
        case Row:
            return row;
        case Type:
            return tile.type.name;
        case Item:
            return tile.item.name;
        case Walkable:
            return tile.isWalkable();
        }
    }

    return {};
}

int MapModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_columns * m_rows;
}

QHash<int, QByteArray> MapModel::roleNames() const
{
    return {
        {Type, "type"},
        {Item, "item"},
        {Column, "column"},
        {Row, "row"},
        {Walkable, "walkable"},
    };
}

bool MapModel::load(QString fileName, Format format)
{
    qCInfo(lcMap, "Loading map from %ls", qUtf16Printable(fileName));

    fileName = dataFileName(std::move(fileName));

    QFile file{fileName};

    if (!file.open(QFile::ReadOnly)) {
        qCWarning(lcMap, "Could not open %ls: %ls", qUtf16Printable(fileName), qUtf16Printable(file.errorString()));
        return false;
    }

    auto rows = file.readAll().trimmed().split('\n');

    if (format == CurrentFormat)
        rows.removeLast();

    for (auto &row: rows)
        row = row.trimmed();

    QList<Tile> tiles;

    if (format == CurrentFormat) {
        for (const auto &row: rows) {
            for (int i = 0; i < row.length(); i+= 2)
                tiles += Tile{row.mid(i, 2)};
        }
    } else if (format == LegacyFormat) {
        for (const auto &row: rows) {
            for (int i = 0; i < row.length(); ++i)
                tiles += Tile{row.mid(i, 1)};
        }
    }

    beginResetModel();
    m_tiles = std::move(tiles);
    m_rows = rows.count();
    m_columns = m_tiles.size() / m_rows;
    endResetModel();

    emit columnsChanged(m_columns);
    emit rowsChanged(m_rows);

    return true;
}

QModelIndex MapModel::indexByPoint(QPoint point) const
{
    return index(point.y() * columns() + point.x());
}

QVariant MapModel::dataByPoint(QPoint point, MapModel::Role role) const
{
    return data(indexByPoint(point), role);
}

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
        m_map->load(mapData["filename"].toString(), static_cast<MapModel::Format>(mapData["format"].toInt()));

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

        return true;
    } else if (fileName.endsWith(".txt")) {
        return load("level1.json") && m_map->load(fileName, MapModel::LegacyFormat);
    } else {
        qCWarning(lcBackend, "Unsupported filename: %ls", qUtf16Printable(fileName));
        return false;
    }
}

bool Backend::canMoveTo(Actor *actor, QPoint destination) const
{
    if (!actor->isAlive())
        return false;

    if (destination.x() < 0 || destination.x() >= columns())
        return false;
    if (destination.y() < 0 || destination.y() >= rows())
        return false;
    if (!m_map->dataByPoint(destination, MapModel::Walkable).toBool())
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

void Backend::onTimeout()
{
    for (auto enemy: m_enemies)
        enemy->act();
}

} // namespace GameOne
