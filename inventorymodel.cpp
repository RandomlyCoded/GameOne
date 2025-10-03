#include "inventorymodel.h"

#include "backend.h"

namespace GameOne {

InventoryItem::InventoryItem(QString name, QObject *parent)
    : InventoryItem{std::move(name), {}, parent}
{}

InventoryItem::InventoryItem(QString name, const QUrl &imageSource, QObject *parent)
    : QObject{parent}
    , m_name{std::move(name)}
    , m_imageSource{Backend::imageUrl(imageSource)}
{}

QVariant InventoryModel::data(const QModelIndex &index, int role) const
{
    if (checkIndex(index)) {
        const auto &slot = m_slots[index.row()];

        switch (static_cast<Role>(role)) {
        case ItemNameRole:
            if (slot.item != nullptr)
                return slot.item->name();

            break;

        case ItemRole:
            return QVariant::fromValue(slot.item.data());

        case ImageSourceRole:
            if (slot.item != nullptr)
                return slot.item->imageSource();

            break;

        case AmountRole:
            return slot.amount;
        }
    }

    return {};
}

int InventoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(m_slots.count());
}

QHash<int, QByteArray> InventoryModel::roleNames() const
{
    return {
        {ItemRole, "item"},
        {ItemNameRole, "itemName"},
        {ImageSourceRole, "imageSource"},
        {AmountRole, "amount"},
    };
}

void InventoryModel::updateItem(InventoryItem *item, int amount)
{
    const auto sameItem = [item](const Slot &slot) { return slot.item == item; };
    const auto it = std::find_if(m_slots.begin(), m_slots.end(), sameItem);

    if (it != m_slots.end()) {
        it->amount += amount;

        const auto row = static_cast<int>(it - m_slots.begin());
        const auto slotIndex = index(row);

        emit dataChanged(slotIndex, slotIndex, {AmountRole});
    } else {
        const auto row = static_cast<int>(m_slots.count());
        beginInsertRows({}, row, row);
        m_slots.append({item, amount});
        endInsertRows();
    }
}

} // namespace GameOne

#include "moc_inventorymodel.cpp"
