#include "inventorymodel.h"

namespace GameOne {

InventoryItem::InventoryItem(QString name, QObject *parent)
    : QObject{parent}
    , m_name{std::move(name)}
{}

QVariant InventoryModel::data(const QModelIndex &index, int role) const
{
    if (checkIndex(index)) {
        const auto &slot = m_slots[index.row()];

        switch (static_cast<Role>(role)) {
        case ItemNameRole:
            if (slot.item)
                return slot.item->name();

            break;

        case ItemRole:
            return QVariant::fromValue(slot.item.data());

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

    return m_slots.count();
}

QHash<int, QByteArray> InventoryModel::roleNames() const
{
    return {
        {ItemRole, "item"},
        {ItemNameRole, "itemName"},
        {AmountRole, "amount"},
    };
}

void InventoryModel::updateItem(InventoryItem *item, int amount)
{
    const auto sameItem = [item](const Slot &slot) { return slot.item == item; };
    const auto it = std::find_if(m_slots.begin(), m_slots.end(), sameItem);

    if (it != m_slots.end()) {
        it->amount += amount;
        const auto slotIndex = index(it - m_slots.begin());
        emit dataChanged(slotIndex, slotIndex, {AmountRole});
    } else {
        beginInsertRows({}, m_slots.count(), m_slots.count());
        m_slots.append({item, amount});
        endInsertRows();
    }
}

} // namespace GameOne

#include "moc_inventorymodel.cpp"
