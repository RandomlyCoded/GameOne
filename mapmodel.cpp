#include "mapmodel.h"

#include "backend.h"

#include <QFile>
#include <QPoint>
#include <QLoggingCategory>

namespace GameOne {

namespace {
Q_LOGGING_CATEGORY(lcMap, "GameOne.map");
} // namespace

MapModel::Tile::Tile(const QHash<char, Type> &types, QByteArray spec)
{
    auto tspec = spec[0];
    auto ispec = spec.length() > 1 ? spec[1] : ' ';

    if (tspec == 'T') {
        tspec = 'G';
        ispec = '@';
    } else if (tspec == 'F') {
        tspec = 'G';
        ispec = '#';
    }

    type = types.value(tspec);
    item = types.value(ispec);
}

bool MapModel::Tile::isWalkable() const
{
    if (item.isValid() && !item.walkable)
        return false;

    return type.walkable;
}

QUrl MapModel::Tile::imageSource() const
{
    if (item.isValid() && item.imageSource().isValid())
        return item.imageSource();

    return type.imageSource();
}

MapModel::MapModel(Backend *backend)
    : MapModel{static_cast<QObject *>(backend)}
{
    setBackend(backend);
}

QVariant MapModel::data(const QModelIndex &index, int role) const
{
    if (hasIndex(index.row(), index.column(), index.parent())) {
        const auto row = index.row() / m_columns;
        const auto column = index.row() % m_columns;
        const auto tile = m_tiles[index.row()];

        switch (static_cast<Role>(role)) {
        case ColumnRole:
            return column;
        case RowRole:
            return row;
        case TypeRole:
            return tile.type.name;
        case ItemRole:
            return tile.item.name;
        case TileColorRole:
            return tile.type.color;
        case ItemColorRole:
            return tile.item.color;
        case ImageSourceRole:
            return tile.imageSource();
        case WalkableRole:
            return tile.isWalkable();
        }
    }

    return {};
}

int MapModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_columns * m_rows;
}

QHash<int, QByteArray> MapModel::roleNames() const
{
    return {
        {ColumnRole, "column"},
        {RowRole, "row"},
        {TypeRole, "type"},
        {ItemRole, "item"},
        {TileColorRole, "tileColor"},
        {ItemColorRole, "itemColor"},
        {ImageSourceRole, "imageSource"},
        {WalkableRole, "walkable"},
    };
}

void MapModel::setBackend(Backend *backend)
{
    if (std::exchange(m_backend, backend) != m_backend) {
        if (m_backend)
            m_tileInfo = backend->resolve(QUrl{"tiles.json"});
        else
            m_tileInfo = {};

        beginResetModel();
        m_tiles.clear();
        endResetModel();

        emit backendChanged(m_backend);
    }
}

QHash<char, MapModel::Tile::Type> MapModel::makeTypes() const
{
    QHash<char, Tile::Type> types;

    for (auto it = m_tileInfo.begin(); it != m_tileInfo.end(); ++it) {
        const auto tile = it->toObject();

        for (const auto key: tile["keys"].toString()) {
            const QColor color{tile["color"].toString()};
            const auto walkable = tile["walkable"].toBool();
            types.insert(key.toLatin1(), {color, it.key(), walkable});
        }
    }

    return types;
}

bool MapModel::load(QString fileName, Format format)
{
    qCInfo(lcMap, "Loading map from %ls", qUtf16Printable(fileName));

    fileName = Backend::dataFileName(std::move(fileName));

    QFile file{fileName};

    if (!file.open(QFile::ReadOnly)) {
        qCWarning(lcMap, "Could not open %ls: %ls", qUtf16Printable(fileName), qUtf16Printable(file.errorString()));
        return false;
    }

    auto rows = file.readAll().trimmed().split('\n');

    if (format == CurrentFormat)
        rows.removeLast();

    for (auto &row: rows)
        row = row.trimmed();

    const auto types = makeTypes();
    QList<Tile> tiles;

    if (format == CurrentFormat) {
        for (const auto &row: rows) {
            for (int i = 0; i < row.length(); i+= 2)
                tiles += Tile{types, row.mid(i, 2)};
        }
    } else if (format == LegacyFormat) {
        for (const auto &row: rows) {
            for (int i = 0; i < row.length(); ++i)
                tiles += Tile{types, row.mid(i, 1)};
        }
    }

    beginResetModel();
    m_tiles = std::move(tiles);
    m_rows = rows.count();
    m_columns = m_tiles.size() / m_rows;
    endResetModel();

    emit columnsChanged(m_columns);
    emit rowsChanged(m_rows);

    return true;
}

QModelIndex MapModel::indexByPoint(QPoint point) const
{
    return index(point.y() * columns() + point.x());
}

QVariant MapModel::dataByPoint(QPoint point, MapModel::Role role) const
{
    return data(indexByPoint(point), role);
}

} // namespace GameOne

#include "moc_mapmodel.cpp"
