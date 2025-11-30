#ifndef GAMEONE_MAPMODEL_H
#define GAMEONE_MAPMODEL_H

#include <QAbstractListModel>
#include <QColor>
#include <QJsonObject>
#include <QPointer>
#include <QUrl>

namespace GameOne {

class Backend;

class MapModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(GameOne::Backend *backend READ backend WRITE setBackend NOTIFY backendChanged FINAL)
    Q_PROPERTY(int columns READ columns NOTIFY columnsChanged FINAL)
    Q_PROPERTY(int rows READ rows NOTIFY rowsChanged FINAL)

public:
    enum Role {
        TypeRole = Qt::UserRole + 1,
        PositionRole,
        ColumnRole,
        RowRole,
        ItemTypeRole,
        TileColorRole,
        TileImageSourceRole,
        TileImageCountRole,
        ItemColorRole,
        ItemImageSourceRole,
        ItemImageCountRole,
        IsStartRole,
        WalkableRole,
    };

    Q_ENUM(Role)

    enum Format {
        LegacyFormat = 1,
        CurrentFormat = 2,
    };

    Q_ENUM(Format)

    using QAbstractListModel::QAbstractListModel;
    explicit MapModel(Backend *backend);

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    Backend *backend() const { return m_backend.data(); }
    int columns() const { return m_columns; }
    int rows() const { return m_rows; }

    Q_INVOKABLE bool load(const QString &fileName, Format format);

    QModelIndex indexByPoint(QPoint point) const;
    QVariant dataByPoint(QPoint point, Role role) const;

public slots:
    void setBackend(GameOne::Backend *backend);

signals:
    void backendChanged(GameOne::Backend * backend);
    void columnsChanged(int columns);
    void rowsChanged(int rows);

private:
    struct Tile
    {
        struct Type
        {
            QString name;
            QColor color;
            QUrl imageSource;
            int imageCount = 0;
            bool walkable = false;
            bool isStart = false;

            bool isValid() const { return !name.isEmpty(); }
        };

        explicit Tile(const QHash<char, Type> &types, QByteArray spec);

        bool isWalkable() const;

        Type type;
        Type item;
    };

    QHash<char, Tile::Type> makeTypes() const;

    QPointer<Backend> m_backend;
    QJsonObject m_tileInfo;
    QList<Tile> m_tiles;

    int m_columns = 0;
    int m_rows = 0;
};

} // namespace GameOne

#endif // GAMEONE_MAPMODEL_H
