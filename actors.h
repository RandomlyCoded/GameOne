#ifndef GAMEONE_ACTORS_H
#define GAMEONE_ACTORS_H

#include <QColor>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QUrl>

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
    Q_PROPERTY(int minimumEnergy READ minimumEnergy NOTIFY minimumEnergyChanged FINAL)
    Q_PROPERTY(int maximumEnergy READ maximumEnergy NOTIFY maximumEnergyChanged FINAL)
    Q_PROPERTY(bool energyVisible READ energyVisible CONSTANT FINAL)

    Q_PROPERTY(QColor color READ color CONSTANT FINAL)
    Q_PROPERTY(QUrl imageSource READ imageSource NOTIFY imageSourceChanged FINAL)
    Q_PROPERTY(int imageCount READ imageCount NOTIFY imageCountChanged FINAL)

    Q_PROPERTY(int rotationSteps READ rotationSteps CONSTANT FINAL)

public:
    explicit Actor(QJsonObject spec, Backend *backend);

    virtual QString type() const = 0;

    auto x() const { return m_position.x(); }
    auto y() const { return m_position.y(); }
    auto position() const { return m_position; }

    void setName(QString name);
    auto name() const { return m_name; }

    virtual QColor color() const = 0;

    QUrl imageSource() const;
    int imageCount() const;

    auto rotationSteps() const { return m_rotationSteps; }

    auto lives() const { return m_lives; }
    auto energy() const { return m_energy; }
    auto minimumEnergy() const { return m_minimumEnergy; }
    auto maximumEnergy() const { return m_maximumEnergy; }
    auto isAlive() const { return m_lives > 0 && m_energy > 0; }

    virtual bool energyVisible() const = 0;
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
    void minimumEnergyChanged(int minimumEnergy);
    void maximumEnergyChanged(int maximumEnergy);
    void imageSourceChanged(QUrl imageSource);
    void imageCountChanged(int imageCount);

protected:
    Backend *backend() const;

private:
    struct EnergyLevel {
        qreal minimumEnergy;
        QUrl imageSource;
        int imageCount;
    };

    void setEnergy(int energy);

    static QList<EnergyLevel> makeEnergyLevels(QJsonArray array);
    QList<EnergyLevel>::ConstIterator currentEnergyLevel() const;

    QString m_name;
    QPoint m_origin;
    QPoint m_position;

    QList<EnergyLevel> m_energyLevels;
    int m_minimumEnergy;
    int m_maximumEnergy;
    int m_energy;

    int m_maximumLives;
    int m_lives;

    QUrl m_imageSource;
    int m_imageCount;
    int m_rotationSteps;
};

class Enemy : public Actor
{
    Q_OBJECT

public:
    using Actor::Actor;

    QString type() const override { return "Enemy"; }
    QColor color() const override { return Qt::red; }

    bool energyVisible() const override { return true; };
    bool canAttack(const Actor *opponent) const override;
    int attack(Actor *opponent) override;

    void act();
};

class Tentaklon : public Enemy // Tentaklon is the "Schleimpilz"(look at "Issues/1/0008" for more info)
{
    Q_OBJECT

public:
    using Enemy::Enemy;

    QString type() const override { return "Tentaklon"; }
    QColor color() const override { return Qt::red;}

    bool energyVisible() const override { return true; };
    bool canAttack(const Actor *opponent) const override;
    int attack(Actor *opponent) override;
    char myMoveCard() { for(char tile: m_moveCard) { return tile; } };

    void act();

private:
    bool hasMoveCard = false; // jur vorübergehend, denn wenn wir dann die Karte haben, können wir
    // die if-Bedingung aus Move rausnehmen und brauchen diese Konstante nicht mehr.
    char m_moveCard[4] = {'d', 'l', 'u', 'r'};
    char m_possibilities[4] = {'r', 'l', 'u', 'd'};
    bool moveCardFinished = false;
    void buildMoveCard();
    QPoint buildPosition = position();
};

//class IceGhost : public Enemy
//{
//    Q_OBJECT

//public:
//    using Enemy::Enemy;

//    QString type() const override { return "IceGhost"; }
//    QColor color() const override { return Qt::red;}

//    bool energyVisible() const override { return true; };
//    bool canAttack(const Actor *opponent) const override;
//    int attack(Actor *opponent) override;

//    void act();
//};

//class FireGhost : public Enemy
//{
//    Q_OBJECT

//public:
//    using Enemy::Enemy;

//    QString type() const override { return "FireGhost"; }
//    QColor color() const override { return Qt::red;}

//    bool energyVisible() const override { return true; };
//    bool canAttack(const Actor *opponent) const override;
//    int attack(Actor *opponent) override;

//    void act();
//}; Add this both Enemies later

class Player : public Actor
{
    Q_OBJECT
    Q_PROPERTY(GameOne::InventoryModel *inventory READ inventory CONSTANT FINAL)

public:
    explicit Player(QJsonObject spec, Backend *backend);

    QString type() const override { return "Player"; }
    QColor color() const override { return "pink"; }

    bool energyVisible() const override { return true; };
//    bool canHit(int m_hitEnergy);
    bool canAttack(const Actor *opponent) const override;
    int attack(Actor *opponent) override;

    InventoryModel *inventory() const { return m_inventory; }

private:
    InventoryModel *const m_inventory;
//    int m_hitEnergy;
};

class Item : public Actor
{
    Q_OBJECT

public:
    explicit Item(QJsonObject spec, Backend *backend);

    QString type() const override { return m_type; }
    QColor color() const override { return m_color; }

private:
    QString m_type;
    QColor m_color;
};

class Chest : public Item
{
    Q_OBJECT
    Q_PROPERTY(GameOne::InventoryItem *item READ item NOTIFY itemChanged FINAL)
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)

public:
    explicit Chest(QJsonObject spec, Backend *backend);

    bool energyVisible() const override { return false; };
    bool canAttack(const Actor *opponent) const override;
    int attack(Actor *actor) override;
    void giveBonus(Actor *actor, int amount) override;

    InventoryItem *item() const;
    auto count() const { return m_amount; }

signals:
    void itemChanged(GameOne::InventoryItem *item);
    void countChanged(int count);

private:
    static QJsonObject applyDefaults(QJsonObject json);

    QPointer<InventoryItem> m_item;
    int m_amount = 0;
};

class Ladder : public Item
{
    Q_OBJECT
    Q_PROPERTY(int level READ level CONSTANT FINAL)

public:
    explicit Ladder(QJsonObject spec, Backend *backend);

    bool energyVisible() const override { return false; };
    bool canAttack(const Actor *opponent) const override;
    int attack(Actor *opponent) override;
    void giveBonus(Actor *actor, int amount) override;

    auto level() const { return m_level; }

private:
    static QJsonObject applyDefaults(QJsonObject json);

    int m_level = 0;
    QPoint m_destination;
};

class WitchShop : public Actor // ist für Sean, er will einen Laden wo man Elexiere kaufen kann
{
    Q_OBJECT

public:
    using Actor::Actor;

    bool energyVisible() const override { return false; };
    bool canAttack(const Actor *opponent) const override;
    int attack(Actor *opponent) override;
//    void giveBonus(Actor *actor, int amount) override;

private:
    QPoint m_position;
};

// die Datei wird langsam unübersichtlich sollte man sie aufspalten?
} // namespace GameOne

#endif // GAMEONE_ACTORS_H
