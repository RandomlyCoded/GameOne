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
        case PositionRole:
            return QPoint{column, row};
        case ColumnRole:
            return column;
        case RowRole:
            return row;
        case TypeRole:
            return tile.type.name;
        case ItemTypeRole:
            return tile.item.name;
        case TileColorRole:
            return tile.type.color;
        case TileImageSourceRole:
            return tile.type.imageSource;
        case TileImageCountRole:
            return tile.type.imageCount;
        case ItemColorRole:
            return tile.item.color;
        case ItemImageSourceRole:
            return tile.item.imageSource;
        case ItemImageCountRole:
            return tile.item.imageCount;
        case IsStartRole:
            return tile.item.isStart;
        case WalkableRole:
            return tile.isWalkable();
        }
    }

    return {};
}

bool MapModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (hasIndex(index.row(), index.column(), index.parent())) {
        if (role == IsStartRole && value.canConvert(QVariant::Bool)) {
            m_tiles[index.row()].item.isStart = value.toBool();
            return true;
        }
    }

    return false;
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
        {PositionRole, "position"},
        {ColumnRole, "column"},
        {RowRole, "row"},
        {TypeRole, "type"},
        {ItemTypeRole, "itemType"},
        {TileColorRole, "tileColor"},
        {TileImageSourceRole, "tileImageSource"},
        {TileImageCountRole, "tileImageCount"},
        {ItemColorRole, "itemColor"},
        {ItemImageSourceRole, "itemImageSource"},
        {ItemImageCountRole, "itemImageCount"},
        {IsStartRole, "isStart"},
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
            const QUrl imageSource = Backend::imageUrl(tile["image"].toString());
            const auto imageCount = tile["imageCount"].toInt(1);
            const auto walkable = tile["walkable"].toBool();
            const auto isStart = tile["isStart"].toBool();

            types.insert(key.toLatin1(), {it.key(), color, imageSource, imageCount, walkable, isStart});
        }
    }

    return types;
}

bool MapModel::load(QString fileName, Format format)
{
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
