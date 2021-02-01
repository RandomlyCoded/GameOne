#ifndef GAMEONE_ACTORS_H
#define GAMEONE_ACTORS_H

#include <QObject>
#include <QPoint>
#include <QPointer>

namespace GameOne {

class Backend;
class InventoryItem;
class InventoryModel;

class Actor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString type READ type CONSTANT FINAL)
    Q_PROPERTY(int x READ x NOTIFY positionChanged FINAL)
    Q_PROPERTY(int y READ y NOTIFY positionChanged FINAL)
    Q_PROPERTY(QPoint position READ position NOTIFY positionChanged FINAL)

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(int lives READ lives NOTIFY livesChanged FINAL)
    Q_PROPERTY(bool isAlive READ isAlive NOTIFY energyChanged FINAL)

    Q_PROPERTY(int energy READ energy NOTIFY energyChanged FINAL)
    Q_PROPERTY(int maximumEnergy READ maximumEnergy NOTIFY maximumEnergyChanged FINAL)

public:
    explicit Actor(QJsonObject spec, Backend *backend);

    virtual QString type() const = 0;

    auto x() const { return m_position.x(); }
    auto y() const { return m_position.y(); }
    auto position() const { return m_position; }

    void setName(QString name);
    QString name() const;

    auto lives() const { return m_lives; }
    auto energy() const { return m_energy; }
    auto maximumEnergy() const { return m_maximumEnergy; }
    auto isAlive() const { return m_lives > 0 && m_energy > 0; }

    virtual bool canAttack(const Actor *opponent) const = 0;

    virtual int attack(Actor *opponent) = 0;
    virtual void giveBonus(Actor *actor, int amount);

    void moveTo(QPoint destination);
    void tryMoveTo(QPoint destination);
    void stealEnergy(int amount);
    void giveEnergy(int amount);
    void die();

public slots:
    void moveLeft();
    void moveUp();
    void moveDown();
    void moveRight();

    void respawn();

signals:
    void positionChanged(QPoint position);
    void nameChanged(QString name);
    void livesChanged(int lives);
    void energyChanged(int energy);
    void maximumEnergyChanged(int maximumEnergy);

protected:
    Backend *backend() const;

private:

    QString m_name;
    QPoint m_origin;
    QPoint m_position;

    int m_maximumEnergy;
    int m_maximumLifes;
    int m_energy;
    int m_lives;
};

class Enemy : public Actor
{
    Q_OBJECT

public:
    using Actor::Actor;

    QString type() const override;

    bool canAttack(const Actor *opponent) const override;
    int attack(Actor *opponent) override;

    void act();
};

class Player : public Actor
{
    Q_OBJECT
    Q_PROPERTY(GameOne::InventoryModel *inventory READ inventory CONSTANT FINAL)

public:
    explicit Player(QJsonObject spec, Backend *backend);

    QString type() const override;

//    bool canHit(int m_hitEnergy);
    bool canAttack(const Actor *opponent) const override;
    int attack(Actor *opponent) override;

    InventoryModel *inventory() const { return m_inventory; }

private:
    InventoryModel *const m_inventory;
//    int m_hitEnergy;
};

class Chest : public Actor
{
    Q_OBJECT
    Q_PROPERTY(GameOne::InventoryItem *item READ item NOTIFY itemChanged FINAL)
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)

public:
    explicit Chest(QJsonObject spec, Backend *backend);

    QString type() const override;

    void giveBonus(Actor *actor, int amount) override;
    bool canAttack(const Actor *opponent) const override;
    int attack(Actor *actor) override;

    InventoryItem *item() const;
    auto count() const { return m_amount; }

signals:
    void itemChanged(GameOne::InventoryItem *item);
    void countChanged(int count);

private:
    QPointer<InventoryItem> m_item;
    int m_amount = 0;
};

class Ladder : public Actor
{
    Q_OBJECT
    Q_PROPERTY(int level READ level CONSTANT FINAL)

public:
    explicit Ladder(QJsonObject spec, Backend *backend);

    QString type() const override;

    void giveBonus(Actor *actor, int amount) override;
    bool canAttack(const Actor *opponent) const override;
    int attack(Actor *opponent) override;

    auto level() const { return m_level; }

private:
    int m_level = 0;
    QPoint m_destination;
};

} // namespace GameOne

#endif // GAMEONE_ACTORS_H
