#include "savefile_list_model.hpp"

#include <QDateTime>

namespace firelight::saves {

SavefileListModel::SavefileListModel() = default;

int SavefileListModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}

QVariant SavefileListModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  const auto item = m_items.at(index.row());

  switch (role) {
  case HasData:
    return item.hasData;
  case FilePath:
    return QString::fromStdString(item.filePath);
  case ContentHash:
    return QString::fromStdString(item.contentHash);
  case SlotNumber:
    return item.slotNumber;
  case SavefileMd5:
    return QString::fromStdString(item.savefileMd5);
  case Name:
    return QString::fromStdString(item.name);
  case Description:
    return QString::fromStdString(item.description);
  case LastModified:
    return QDateTime::fromSecsSinceEpoch(item.lastModifiedEpochSeconds)
        .toString("yyyy-MM-dd hh:mm:ss");
  default:
    return QVariant{};
  }

  return {};
}

Qt::ItemFlags SavefileListModel::flags(const QModelIndex &index) const {
  return Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

QHash<int, QByteArray> SavefileListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[HasData] = "hasData";
  roles[FilePath] = "filePath";
  roles[ContentHash] = "contentHash";
  roles[SlotNumber] = "slotNumber";
  roles[SavefileMd5] = "savefileMd5";
  roles[Name] = "name";
  roles[Description] = "description";
  roles[LastModified] = "lastModified";
  return roles;
}
void SavefileListModel::reset(std::vector<SavefileInfo> items) {
  emit beginResetModel();
  m_items = std::move(items);
  emit endResetModel();
}

} // namespace firelight::saves
  // firelight