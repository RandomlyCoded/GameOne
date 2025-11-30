#include "levelmodel.h"

#include "backend.h"

#include <QDir>


#include <QDebug>

namespace GameOne {

namespace {

template<class T>
std::optional<int> toInt(const T &str)
{
    auto isNumber = false;
    const auto number = str.toInt(&isNumber);

    if (isNumber)
        return {number};

    return {};
}

} // namespace

LevelModel::LevelModel(QObject *parent)
    : QAbstractListModel{parent}
{
    refresh();
}

QVariant LevelModel::data(const QModelIndex &index, int role) const
{
    if (checkIndex(index)) {
        switch (static_cast<Role>(role)) {
        case LevelNameRole:
            return m_levels[index.row()].name;
        case FileNameRole:
            return m_levels[index.row()].fileName;
        }
    }

    return {};
}

int LevelModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(m_levels.count());
}

QHash<int, QByteArray> LevelModel::roleNames() const
{
    return {
        {LevelNameRole, "levelName"},
        {FileNameRole, "fileName"},
    };
}

void LevelModel::refresh()
{
    Backend levelReader;

    beginResetModel();
    m_levels.clear();

    for (const auto levelList = Backend::dataDir().entryInfoList({"*.level.json"});
         const auto &fileInfo : levelList) {
        if (const auto index = toInt(fileInfo.baseName())) {
            if (levelReader.load(fileInfo.fileName()))
                m_levels += {*index, levelReader.levelName(), fileInfo.filePath()};
        }
    }

    const auto lessByIndexAndName = [](const auto &lhs, const auto &rhs) {
        return std::tie(lhs.index, lhs.name) < std::tie(rhs.index, rhs.name);
    };

    std::sort(m_levels.begin(), m_levels.end(), lessByIndexAndName);

    endResetModel();
}

} // namespace GameOne

#include "moc_levelmodel.cpp"
