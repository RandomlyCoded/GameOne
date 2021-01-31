#include "mapmodel.h"

#include "backend.h"

#include <QFile>
#include <QPoint>
#include <QLoggingCategory>

namespace GameOne {

namespace {
Q_LOGGING_CATEGORY(lcMap, "GameOne.map");
} // namespace

MapModel::Tile::Tile(QByteArray spec)
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

    static const QHash<char, Type> types = {
        {'G',   {"Grass",       true}},
        {'W',   {"DeepWater",   false}},
        {'w',   {"Water",       true}},
        {'H',   {"Hill",        true}},
        {'M',   {"Mountain",    false}},
        {'S',   {"Sand",        true}},
        {'I',   {"Ice",         true}},
        {'L',   {"Lava",        true}},
        {'@',   {"Tree",        false}},
        {'#',   {"Fence",       false}},
        {'-',   {"Fence",       false}},
        {'|',   {"Fence",       false}},
        {'/',   {"Fence",       false}},
        {'\\',  {"Fence",       false}},
    };

    type = types.value(tspec);
    item = types.value(ispec);
}

bool MapModel::Tile::isWalkable() const
{
    if (item.isValid() && !item.walkable)
        return false;

    return type.walkable;
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
        {TypeRole, "type"},
        {ItemRole, "item"},
        {ColumnRole, "column"},
        {RowRole, "row"},
        {WalkableRole, "walkable"},
    };
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

    QList<Tile> tiles;

    if (format == CurrentFormat) {
        for (const auto &row: rows) {
            for (int i = 0; i < row.length(); i+= 2)
                tiles += Tile{row.mid(i, 2)};
        }
    } else if (format == LegacyFormat) {
        for (const auto &row: rows) {
            for (int i = 0; i < row.length(); ++i)
                tiles += Tile{row.mid(i, 1)};
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
