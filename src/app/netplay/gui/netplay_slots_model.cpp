#include "netplay_slots_model.hpp"

namespace firelight::gui {

NetplaySlotsModel::NetplaySlotsModel(netplay::NetplayService &service,
                                     QObject *parent)
    : QAbstractListModel(parent), m_service(service) {}

int NetplaySlotsModel::rowCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : netplay::MAX_SLOTS;
}

QVariant NetplaySlotsModel::data(const QModelIndex &index,
                                 const int role) const {
  if (!index.isValid() || index.row() < 0 ||
      index.row() >= netplay::MAX_SLOTS) {
    return {};
  }
  const auto &slot = m_slots.slot(index.row());

  switch (role) {
  case SlotNumber:
    return index.row() + 1;
  case Occupied:
    return slot.has_value();
  case MemberId:
    return slot ? QString::number(slot->memberId) : QString();
  case DisplayName:
    return slot ? QString::fromStdString(slot->displayName) : QString();
  case IsSelf:
    return slot && slot->memberId == m_selfId;
  case IsHostMember:
    return slot && slot->memberId == m_hostId;
  case Ready:
    return slot && slot->ready;
  case LocalPadIndex:
    return slot ? slot->localPadIndex : 0;
  default:
    return {};
  }
}

QHash<int, QByteArray> NetplaySlotsModel::roleNames() const {
  return {{SlotNumber, "slotNumber"},   {Occupied, "occupied"},
          {MemberId, "memberId"},       {DisplayName, "displayName"},
          {IsSelf, "isSelf"},           {IsHostMember, "isHostMember"},
          {Ready, "ready"},             {LocalPadIndex, "localPadIndex"}};
}

void NetplaySlotsModel::refresh() {
  m_slots = m_service.session().slotTable();
  const auto lobby = m_service.session().lobby();
  m_selfId = lobby.self.id;
  m_hostId = lobby.hostId;
  emit dataChanged(index(0), index(netplay::MAX_SLOTS - 1));
}

} // namespace firelight::gui
