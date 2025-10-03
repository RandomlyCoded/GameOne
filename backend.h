#ifndef GAMEONE_BACKEND_H
#define GAMEONE_BACKEND_H

#include "actors.h"

#include <QElapsedTimer>
#include <QJsonDocument>
#include <QMap>

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
    Q_PROPERTY(qint64 ticks READ ticks NOTIFY ticksChanged FINAL)

public:
    explicit Backend(QObject *parent = {});

    auto levelFileName() const { return m_levelFileName; }
    auto levelName() const { return m_levelName; }
    int columns() const;
    int rows() const;

    qint64 ticks() const;

    QList<Actor *> actors() const;
    QList<Enemy *> enemies() const;
    Player *player() const { return m_player.get(); }
    MapModel *map() const { return m_map; }

    InventoryItem *item(QString id) const;

    Q_INVOKABLE bool load(QString fileName, std::optional<QPoint> playerPosition = {});
    Q_INVOKABLE void respawn();

    bool canMoveTo(Actor *actor, QPoint destination) const;

    static QDir dataDir();
    static QString dataFileName(QString fileName);

    static QUrl imageUrl(QString fileName);
    static QUrl imageUrl(QUrl imageUrl);

    Q_INVOKABLE static QUrl imageUrl(QUrl imageUrl, int imageCount, qint64 tick);

    static QString levelFileName(int index);

    QJsonObject resolve(QJsonObject object) const;
    QJsonObject resolve(QUrl ref) const;

signals:
    void levelFileNameChanged(QString levelFileName);
    void levelNameChanged(QString levelName);
    void columnsChanged(int columns);
    void rowsChanged(int rows);

    void actorsChanged();
    void enemiesChanged();
    void playerChanged(GameOne::Player *player);

    void ticksChanged(qint64 ticks);

private:
    QJsonDocument cachedDocument(QUrl url) const;

    void loadItems();
    void validateActors(QString levelFileName, QString mapFileName) const;

    void onActionTimeout();
    void onTicksTimeout();

    QTimer *const m_actionTimer;
    QTimer *const m_ticksTimer;
    QElapsedTimer m_ticks;

    QList<Actor *> m_actors;
    QMap<QString, InventoryItem *> m_items;
    QList<std::shared_ptr<Ladder>> m_ladders;
    QList<std::shared_ptr<Chest>> m_chests;
    QList<std::shared_ptr<Enemy>> m_enemies;
    std::unique_ptr<Player> m_player;

    QString m_levelFileName;
    QString m_levelName;

    mutable QHash<QUrl, QJsonDocument> m_jsonCache;
    MapModel *const m_map;
};

} // namespace GameOne

#endif // GAMEONE_BACKEND_H
