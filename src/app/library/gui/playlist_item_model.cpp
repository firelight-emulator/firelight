#include "playlist_item_model.hpp"

namespace firelight::gui {

bool LibraryFolderListModel::setData(const QModelIndex &index,
                                     const QVariant &value, int role) {
  if (index.row() < 0 || index.row() >= m_items.size())
    return false;

  library::FolderInfo &item = m_items[index.row()];

  switch (role) {
  case PlaylistId:
    return false;
  case DisplayName:
    item.displayName = value.toString().toStdString();
    // TODO: Persist to db
    break;
  default:
    return false;
  }

  getUserLibrary()->update(item);

  emit dataChanged(index, index, {role});

  return true;
}

Qt::ItemFlags LibraryFolderListModel::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

LibraryFolderListModel::LibraryFolderListModel() {
  m_items = getUserLibrary()->listFolders({});
  // const auto entries = m_libraryDatabase->getAllPlaylists();

  // for (const auto &entry : entries) {
  //   m_items.emplace_back(
  //       Item({entry.id, QString::fromStdString(entry.displayName)}));
  // }
}

int LibraryFolderListModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}

QVariant LibraryFolderListModel::data(const QModelIndex &index,
                                      int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  auto item = m_items.at(index.row());

  switch (role) {
  case PlaylistId:
    return item.id;
  case DisplayName:
    return QString::fromStdString(item.displayName);
  default:
    return QVariant{};
  }
}

QHash<int, QByteArray> LibraryFolderListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[PlaylistId] = "playlist_id";
  roles[DisplayName] = "display_name";
  return roles;
}

bool LibraryFolderListModel::addFolder(const QString &displayName) {

  if (auto folder =
          library::FolderInfo{.displayName = displayName.toStdString()};
      getUserLibrary()->create(folder)) {

    beginInsertRows(QModelIndex(), rowCount(QModelIndex()),
                    rowCount(QModelIndex()));

    m_items.push_back(folder);
    endInsertRows();

    return true;
  }

  return false;
}

void LibraryFolderListModel::deleteFolder(const int folderId) {
  if (!getUserLibrary()->deleteFolder(folderId)) {
    spdlog::warn("Failed to delete folder with ID {}", folderId);
    return;
  }

  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items[i].id == folderId) {
      beginRemoveRows(QModelIndex(), i, i);
      m_items.erase(m_items.begin() + i);
      endRemoveRows();
      break;
    }
  }
}
//
// void PlaylistItemModel::renamePlaylist(const int playlistId,
//                                        const QString &newName) {
//   m_libraryDatabase->renamePlaylist(playlistId, newName.toStdString());
//
//   for (int i = 0; i < m_items.size(); ++i) {
//     if (m_items.at(i).playlistId == playlistId) {
//       setData(createIndex(i, 0), newName, DisplayName);
//       break;
//     }
//   }
// }
//

} // namespace firelight::gui