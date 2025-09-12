#include "play_session_list_model.hpp"

namespace firelight::activity {

QHash<int, QByteArray> PlaySessionListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[StartTime] = "startTime";
  roles[EndTime] = "endTime";
  roles[NumUnpausedSeconds] = "numUnpausedSeconds";
  roles[SaveSlotNumber] = "saveSlotNumber";
  return roles;
}

int PlaySessionListModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}

QVariant PlaySessionListModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  const auto item = m_items.at(index.row());

  switch (role) {
  case StartTime:
    return QVariant::fromValue(item.startTime);
  case EndTime:
    return QVariant::fromValue(item.endTime);
  case NumUnpausedSeconds:
    return QVariant::fromValue(item.unpausedDurationMillis / 1000);
  case SaveSlotNumber:
    return item.slotNumber;
  default:
    return QVariant{};
  }
}
void PlaySessionListModel::refreshItems(
    std::vector<PlaySession> &playSessions) {

  emit beginResetModel();
  m_items.clear();

  for (const auto &session : playSessions) {
    m_items.push_back(session);
  }
  emit endResetModel();
}

QVector<PlaySession> PlaySessionListModel::getItems() const { return m_items; }

} // namespace firelight::activity
