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
    , m_maximumLives{qMax(spec["maximumLives"].toInt(), 1)}
    , m_lives{m_maximumLives}
    , m_imageSource{Backend::imageUrl(spec["image"].toString())}
    , m_imageCount{spec["imageCount"].toInt()}
    , m_rotationSteps{spec["rotationSteps"].toInt()}
{
    respawn();
}

void Actor::setName(const QString &name)
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
    return dynamic_cast<Backend *>(parent());
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

QList<Actor::EnergyLevel> Actor::makeEnergyLevels(const QJsonArray &array)
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
        moveTo(destination);
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

int Actor::attack(Actor */*opponent*/)
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
    return dynamic_cast<const Player *>(opponent) != nullptr;
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
    // FIXME: use proper generator from std::random
    const auto direction = Direction{std::rand() % 4};

    switch (direction) {
    case Direction::Left:
        if (backend()->player()->x() < x())
            moveLeft();

        break;

    case Direction::Up:
        if (backend()->player()->y() < y())
            moveUp();

        break;

    case Direction::Right:
        if (backend()->player()->x() > x())
            moveRight();

        break;

    case Direction::Down:
        if (backend()->player()->y() > y())
            moveDown();

        break;

    case Direction::None:
        break;
    }
}

bool Tentaklon::canAttack(const Actor *opponent) const
{
    return Enemy::canAttack(opponent);
}

int Tentaklon::attack(Actor *opponent)
{
    return Enemy::attack(opponent);
}

void Tentaklon::buildMoveCard()
{
    if (!hasMoveCard()) {
        for (auto &moveCard : m_moveCard) {
            if (const auto right = m_builtPosition + QPoint{+1, 0};
                backend()->canMoveTo(this, right)) {
                moveCard        = Direction::Right;
                m_builtPosition = right;
            } else if (const auto left = m_builtPosition + QPoint{-1, 0};
                       backend()->canMoveTo(this, left)) {
                moveCard        = Direction::Left;
                m_builtPosition = left;
            } else if (const auto upwards = m_builtPosition + QPoint{0, +1};
                       backend()->canMoveTo(this, upwards)) {
                moveCard        = Direction::Up;
                m_builtPosition = upwards;
            } else if (const auto downwards = m_builtPosition + QPoint{0, -1};
                       backend()->canMoveTo(this, downwards)) {
                moveCard        = Direction::Down;
                m_builtPosition = downwards;
            }
        }

        m_currentMove = 0;
    }

    act();

    /* here it should build a card for moving (siehe oben)
     *'r', Move right
     *'u', Move up
     *'l', Move left
     *'d', Move down
     *
     *leider baut er irgendeine Scheiße.
    */
}

void Tentaklon::act()
{
    if (hasMoveCard()) {
        switch(m_moveCard[m_currentMove++]) {
        case Direction::Right:
            moveRight();
            break;
        case Direction::Up:
            moveUp();
            break;
        case Direction::Left:
            moveLeft();
            break;
        case Direction::Down:
            moveDown();
            break;

        case Direction::None:
            m_currentMove = m_moveCard.size();
            buildMoveCard();
            return;
        }
    } else {
        buildMoveCard();
//        Enemy::act(); // just until i have a script how the Tentaklon acts...
    }
}

Player::Player(QJsonObject spec, Backend *backend)
    : Actor{std::move(spec), backend}
    , m_inventory{new InventoryModel{this}}
{}


bool Player::canAttack(const Actor *opponent) const
{
    return dynamic_cast<const Player *>(opponent) == nullptr;
        // && m_hitEnergy > 0
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

void Chest::giveBonus(Actor *actor, int /*amount*/)
{
    if (auto *const player = backend()->player(); player == actor) {
        player->inventory()->updateItem(m_item, std::exchange(m_amount, 0));
        emit countChanged(m_amount);
    }
}

bool Chest::canAttack(const Actor */*opponent*/) const
{
    return false;
}

int Chest::attack(Actor */*actor*/)
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

void Ladder::giveBonus(Actor */*actor*/, int /*amount*/)
{
    if (m_level >= LevelModel::LIMBO_LEVEL) {
        backend()->load(LevelModel::levelFileName(m_level), m_destination);
    } else {
        backend()->player()->tryMoveTo(m_destination);
    }
}

bool Ladder::canAttack(const Actor */*opponent*/) const
{
    return false;
}

int Ladder::attack(Actor */*opponent*/)
{
    return 0;
}

bool WitchShop::canAttack(const Actor */*opponent*/) const
{
    return false;
}

int WitchShop::attack(Actor */*opponent*/)
{
    return 0;
}

} // namespace GameOne

#include "moc_actors.cpp"
