#ifndef GAMEONE_BACKEND_H
#define GAMEONE_BACKEND_H

#include "actors.h"

#include <memory>

class QDir;
class QTimer;

namespace GameOne {

class MapModel;

class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString levelFileName READ levelFileName NOTIFY levelFileNameChanged FINAL)
    Q_PROPERTY(QString levelName READ levelName NOTIFY levelNameChanged FINAL)
    Q_PROPERTY(int columns READ columns NOTIFY columnsChanged FINAL)
    Q_PROPERTY(int rows READ rows NOTIFY rowsChanged FINAL)
    Q_PROPERTY(QList<GameOne::Actor *> actors READ actors NOTIFY actorsChanged FINAL)
    Q_PROPERTY(QList<GameOne::Enemy *> enemies READ enemies NOTIFY enemiesChanged FINAL)
    Q_PROPERTY(GameOne::Player *player READ player NOTIFY playerChanged FINAL)
    Q_PROPERTY(GameOne::MapModel *map READ map CONSTANT FINAL)

public:
    explicit Backend(QObject *parent = {});

    QString levelFileName() const { return m_levelFileName; }
    QString levelName() const { return m_levelName; }
    int columns() const;
    int rows() const;

    QList<Actor *> actors() const;
    QList<Enemy *> enemies() const;
    Player *player() const { return m_player.get(); }
    MapModel *map() const { return m_map; }

    Q_INVOKABLE bool load(QString fileName);
    Q_INVOKABLE void respawn();

    bool canMoveTo(Actor *actor, QPoint destination) const;

    static QDir dataDir();
    static QString dataFileName(QString fileName);

signals:
    void levelFileNameChanged(QString levelFileName);
    void levelNameChanged(QString levelName);
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
    QString m_levelFileName;
    QString m_levelName;
};

} // namespace GameOne

#endif // GAMEONE_BACKEND_H
