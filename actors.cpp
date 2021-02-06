#include "actors.h"

#include "backend.h"
#include "inventorymodel.h"

#include <QJsonArray>
#include <QJsonObject>

namespace GameOne {

Actor::Actor(QJsonObject spec, Backend *backend)
    : QObject{backend}
    , m_name{spec["name"].toString()}
    , m_origin{spec["x"].toInt(), spec["y"].toInt()}
    , m_position{m_origin}
    , m_energyLevels{makeEnergyLevels(spec["energyLevels"].toArray())}
    , m_minimumEnergy{spec["minimumEnergy"].toInt()}
    , m_maximumEnergy{qMax(spec["maximumEnergy"].toInt(), 1)}
    , m_maximumLifes{qMax(spec["maximumLifes"].toInt(), 1)}
    , m_lives{m_maximumLifes}
    , m_imageSource{Backend::imageUrl(spec["image"].toString())}
    , m_imageCount{spec["imageCount"].toInt()}
    , m_rotationSteps{spec["rotationSteps"].toInt()}
{
    respawn();
}

void Actor::setName(QString name)
{
    if (std::exchange(m_name, name) != name)
        emit nameChanged(m_name);
}

QUrl Actor::imageSource() const
{
    if (const auto level = currentEnergyLevel(); level != m_energyLevels.end() && !level->imageSource.isEmpty())
        return level->imageSource;

    return m_imageSource;
}

int Actor::imageCount() const
{
    if (const auto level = currentEnergyLevel(); level != m_energyLevels.end() && level->imageCount > 0)
        return level->imageCount;

    return m_imageCount;
}

void Actor::giveBonus(Actor *actor, int amount)
{
    actor->giveEnergy(amount);
}

Backend *Actor::backend() const
{
    return static_cast<Backend *>(parent());
}

void Actor::setEnergy(int energy)
{
    if (std::exchange(m_energy, energy) != m_energy) {
        const auto oldImageSource = imageSource();
        const auto oldImageCount = imageCount();

        m_energy = energy;
        emit energyChanged(m_energy);

        if (const auto newImageSource = imageSource(); newImageSource != oldImageSource)
            emit imageSourceChanged(newImageSource);
        if (const auto newImageCount = imageCount(); newImageCount != oldImageCount)
            emit imageCountChanged(newImageCount);

        if (m_energy == 0)
            die();
    }
}

QList<Actor::EnergyLevel> Actor::makeEnergyLevels(QJsonArray array)
{
    QList<Actor::EnergyLevel> levels;

    for (const auto &value: array) {
        const auto spec = value.toObject();
        const auto minimumEnergy = static_cast<qreal>(spec["minimumEnergy"].toDouble());
        const auto imageSource = Backend::imageUrl(spec["image"].toString());
        const auto imageCount = spec["imageCount"].toInt();
        levels.append({minimumEnergy, imageSource, imageCount});
    }

    const auto byDescendingEnergy = [](const auto &lhs, const auto &rhs) {
        return lhs.minimumEnergy > rhs.minimumEnergy;
    };

    std::sort(levels.begin(), levels.end(), byDescendingEnergy);

    return levels;
}

QList<Actor::EnergyLevel>::ConstIterator Actor::currentEnergyLevel() const
{
    const auto fitsCurrentLevel = [current = energy(), maximum = maximumEnergy()](const auto &level) {
        return current >= level.minimumEnergy * maximum;
    };

    return std::find_if(m_energyLevels.begin(), m_energyLevels.end(), fitsCurrentLevel);
}

void Actor::moveTo(QPoint destination)
{
    m_position = destination;
    emit positionChanged(m_position);
}

void Actor::tryMoveTo(QPoint destination)
{
    if (backend()->canMoveTo(this, destination))
        moveTo(std::move(destination));
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
    m_position = m_origin;
    setEnergy(m_maximumEnergy);
    emit positionChanged(m_position);
}

int Actor::attack(Actor *)
{
    return 0;
}

void Actor::stealEnergy(int amount)
{
    setEnergy(qMax(m_minimumEnergy, m_energy - amount));
}

void Actor::giveEnergy(int amount)
{
    setEnergy(qMin(m_maximumEnergy, m_energy + amount));
}

void Actor::die()
{
    if (m_lives > 0) {
        --m_lives;
        emit livesChanged(m_lives);
    }
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

Player::Player(QJsonObject spec, Backend *backend)
    : Actor{std::move(spec), backend}
    , m_inventory{new InventoryModel{this}}
{}


bool Player::canAttack(const Actor *opponent) const
{
    return !dynamic_cast<const Player *>(opponent)/* && m_hitEnergy > 0*/;
}

int Player::attack(Actor *opponent)
{
    if (canAttack(opponent)) {
        opponent->stealEnergy(1);
//        m_hitEnergy--;
        return std::rand() % 2; // FIXME: use proper generator from std::random
    }

    return 0;
}

Item::Item(QJsonObject spec, Backend *backend)
    : Actor(spec, backend)
    , m_type{spec["type"].toString()}
    , m_color{spec["color"].toString()}
{}

Chest::Chest(QJsonObject spec, Backend *backend)
    : Item{applyDefaults(spec), backend}
    , m_item{backend->item(spec["item"].toString())}
    , m_amount{qMax(spec["amount"].toInt(), 1)}
{}

QJsonObject GameOne::Chest::applyDefaults(QJsonObject json)
{
    if (!json.contains("color"))
        json.insert("color", "#b29764");
    if (!json.contains("type"))
        json.insert("type", "Chest");
    if (!json.contains("minimumEnergy"))
        json.insert("minimumEnergy", 1);
    if (!json.contains("maximumEnergy"))
        json.insert("maximumEnergy", 2);

    return json;
}

void Chest::giveBonus(Actor *actor, int)
{
    if (const auto player = backend()->player(); player == actor) {
        player->inventory()->updateItem(m_item, std::exchange(m_amount, 0));
        emit countChanged(m_amount);
    }
}

bool Chest::canAttack(const Actor *) const
{
    return false;
}

int Chest::attack(Actor *)
{
    return 0;
}

InventoryItem *Chest::item() const
{
    return m_item.data();
}

Ladder::Ladder(QJsonObject spec, Backend *backend)
    : Item{applyDefaults(spec), backend}
    , m_level{spec["level"].toInt()}
    , m_destination{spec["dx"].toInt(), spec["dy"].toInt()}
{}

QJsonObject GameOne::Ladder::applyDefaults(QJsonObject json)
{
    if (!json.contains("color"))
        json.insert("color", "#625507");
    if (!json.contains("type"))
        json.insert("type", "Ladder");
    if (!json.contains("minimumEnergy"))
        json.insert("minimumEnergy", 1);
    if (!json.contains("maximumEnergy"))
        json.insert("maximumEnergy", 2);

    return json;
}

void Ladder::giveBonus(Actor *, int)
{
    if (m_level > 0) {
        backend()->load(Backend::levelFileName(m_level), m_destination);
    } else {
        backend()->player()->tryMoveTo(m_destination);
    }
}

bool Ladder::canAttack(const Actor *actor) const
{
    return false;
}

int Ladder::attack(Actor *opponent)
{
    return 0;
}

} // namespace GameOne

#include "moc_actors.cpp"
