#include "actors.h"

#include "backend.h"
#include "inventorymodel.h"

#include <QJsonObject>

namespace GameOne {

Actor::Actor(QJsonObject spec, Backend *backend)
    : QObject{backend}
    , m_name{spec["name"].toString()}
    , m_origin{spec["x"].toInt(), spec["y"].toInt()}
    , m_position{m_origin}
    , m_maximumEnergy{qMax(spec["maximumEnergy"].toInt(), 1)}
    , m_maximumLifes{qMax(spec["maximumLifes"].toInt(), 1)}
    , m_lives{m_maximumLifes}
    , m_imageSource{Backend::imageUrl(spec["image"].toString())}
{
    respawn();
}

void Actor::setName(QString name)
{
    if (std::exchange(m_name, name) != name)
        emit nameChanged(m_name);
}

void Actor::giveBonus(Actor *actor, int amount)
{
    actor->giveEnergy(amount);
}

Backend *Actor::backend() const
{
    return static_cast<Backend *>(parent());
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

Player::Player(QJsonObject spec, Backend *backend)
    : Actor{std::move(spec), backend}
    , m_inventory{new InventoryModel{this}}
{}

QString Player::type() const
{
    return "Player";
}

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

Chest::Chest(QJsonObject spec, Backend *backend)
    : Actor{spec, backend}
{
    const auto itemType = spec["item"].toString();
    m_item = backend->item(itemType);
    m_amount = spec["amount"].toInt();
}

QString Chest::type() const
{
    return "chest";
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

int Chest::attack(Actor *actor)
{
    return 0;
}

InventoryItem *Chest::item() const
{
    return m_item.data();
}

Ladder::Ladder(QJsonObject spec, Backend *backend)
    : Actor{spec, backend}
{
    m_level = spec["level"].toInt();
    m_destination = {spec["dx"].toInt(), spec["dy"].toInt()};
}

QString Ladder::type() const
{
    return "ladder";
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
