#ifndef GAMEONE_BACKEND_H
#define GAMEONE_BACKEND_H

#include <QAbstractListModel>
#include <QPoint>

#include <memory>

class QTimer;

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
    using Actor::Actor;

    QString type() const override;

    bool canAttack(const Actor *opponent) const override;
    int attack(Actor *opponent) override;
};

class MapModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int columns READ columns NOTIFY columnsChanged FINAL)
    Q_PROPERTY(int rows READ rows NOTIFY rowsChanged FINAL)

public:
    enum Role {
        Type = Qt::UserRole + 1,
        Column,
        Row,
        Walkable,
        Item,
    };

    Q_ENUM(Role)

    enum Format {
        LegacyFormat = 1,
        CurrentFormat = 2,
    };

    Q_ENUM(Format)

    using QAbstractListModel::QAbstractListModel;

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;
    QHash<int, QByteArray> roleNames() const override;

    int columns() const { return m_columns; }
    int rows() const { return m_rows; }

    Q_INVOKABLE bool load(QString fileName, Format format);

    QModelIndex indexByPoint(QPoint point) const;
    QVariant dataByPoint(QPoint point, Role role) const;

signals:
    void columnsChanged(int columns);
    void rowsChanged(int rows);

private:
    struct Tile
    {
        struct Type
        {
            QString name;
            bool walkable = false;

            bool isValid() const { return !name.isEmpty(); }
        };

        explicit Tile(QByteArray spec);
        bool isWalkable() const;

        Type type;
        Type item;
    };

    QList<Tile> m_tiles;
    int m_columns = 0;
    int m_rows = 0;
};

class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int columns READ columns NOTIFY columnsChanged FINAL)
    Q_PROPERTY(int rows READ rows NOTIFY rowsChanged FINAL)
    Q_PROPERTY(QList<Actor *> actors READ actors NOTIFY actorsChanged FINAL)
    Q_PROPERTY(QList<Enemy *> enemies READ enemies NOTIFY enemiesChanged FINAL)
    Q_PROPERTY(Player *player READ player NOTIFY playerChanged FINAL)
    Q_PROPERTY(MapModel *map READ map CONSTANT FINAL)

public:
    explicit Backend(QObject *parent = {});

    int columns() const { return m_map->columns(); }
    int rows() const { return m_map->rows(); }

    QList<Actor *> actors() const;
    QList<Enemy *> enemies() const;
    Player *player() const { return m_player.get(); }
    MapModel *map() const { return m_map; }

    Q_INVOKABLE bool load(QString fileName);

    bool canMoveTo(Actor *actor, QPoint destination) const;

signals:
    void columnsChanged(int columns);
    void rowsChanged(int rows);

    void actorsChanged();
    void enemiesChanged();
    void playerChanged(Player *player);

private:
    void onTimeout();

    QTimer *const m_timer;
    MapModel *const m_map;

    QList<std::shared_ptr<Enemy>> m_enemies;
    std::unique_ptr<Player> m_player;
};

} // namespace GameOne

#endif // GAMEONE_BACKEND_H
