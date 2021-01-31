#include "actors.h"

#include "backend.h"

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
    return dynamic_cast<const Enemy *>(opponent)/* && m_hitEnergy > 0*/;
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

} // namespace GameOne
