#ifndef GAMEONE_INVENTORYMODEL_H
#define GAMEONE_INVENTORYMODEL_H

#include <QAbstractListModel>
#include <QPointer>
#include <QUrl>

namespace GameOne {

class InventoryItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT FINAL)
    Q_PROPERTY(QUrl imageSource READ imageSource CONSTANT FINAL)

public:
    explicit InventoryItem(QString name, QObject *parent = {});
    explicit InventoryItem(QString name, const QUrl &imageSource, QObject *parent = {});

    auto name() const { return m_name; }
    auto imageSource() const { return m_imageSource; }

private:
    QString m_name;
    QUrl m_imageSource;
};

class InventoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        ItemNameRole = Qt::DisplayRole,
        ItemRole = Qt::UserRole + 1,
        ImageSourceRole,
        AmountRole,
    };

    Q_ENUM(Role)

    using QAbstractListModel::QAbstractListModel;

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    void updateItem(InventoryItem *item, int amount = 1);

private:
    struct Slot {
        QPointer<InventoryItem> item;
        int amount;
    };

    QList<Slot> m_slots;
};

} // namespace GameOne

#endif // GAMEONE_INVENTORYMODEL_H
