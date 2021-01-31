#ifndef GAMEONE_ACTORS_H
#define GAMEONE_ACTORS_H

#include <QObject>
#include <QPoint>

namespace GameOne {

class Backend;

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
    void tryMoveTo(QPoint destination);

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

public:
/*    bool canHit(int m_hitEnergy);
    int m_hitEnergy;
*/
    using Actor::Actor;

    QString type() const override;

    bool canAttack(const Actor *opponent) const override;
    int attack(Actor *opponent) override;
};

} // namespace GameOne

#endif // GAMEONE_ACTORS_H
