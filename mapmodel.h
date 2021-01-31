#ifndef GAMEONE_MAPMODEL_H
#define GAMEONE_MAPMODEL_H

#include <QAbstractListModel>

namespace GameOne {

class MapModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int columns READ columns NOTIFY columnsChanged FINAL)
    Q_PROPERTY(int rows READ rows NOTIFY rowsChanged FINAL)

public:
    enum Role {
        TypeRole = Qt::UserRole + 1,
        ColumnRole,
        RowRole,
        WalkableRole,
        ItemRole,
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

} // namespace GameOne

#endif // GAMEONE_MAPMODEL_H
