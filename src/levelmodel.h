#ifndef GAMEONE_LEVELMODEL_H
#define GAMEONE_LEVELMODEL_H

#include <QAbstractListModel>

namespace GameOne {

class LevelModel : public QAbstractListModel
{
    Q_OBJECT

public:
    static constexpr int LIMBO_LEVEL = -1;
    static constexpr int DEFAULT_LEVEL = 1;

    enum Role {
        LevelNameRole = Qt::DisplayRole,
        FileNameRole = Qt::UserRole + 1,
    };

    Q_ENUM(Role)

    explicit LevelModel(QObject *parent = {});

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();

    static QString levelFileName [[nodiscard]](int index);

private:
    struct Level {
        int index;
        QString name;
        QString fileName;
    };

    QList<Level> m_levels;
};

} // namespace GameOne

#endif // GAMEONE_LEVELMODEL_H

#if 0
in Zeile 27 einfügen:
    int playedLevels[std::size(m_levels)]; //DER KOMMT NICHT AN M_LEVELS RAN!!!
#endif
