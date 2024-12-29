#include "playlist_item_model.hpp"

namespace firelight::gui {

bool PlaylistItemModel::setData(const QModelIndex &index, const QVariant &value,
                                int role) {
  if (index.row() < 0 || index.row() >= m_items.size())
    return false;

  Item &item = m_items[index.row()];

  switch (role) {
  case PlaylistId:
    return false;
  case DisplayName:
    item.displayName = value.toString();
    break;
  default:
    return false;
  }

  emit dataChanged(index, index, {role});

  return true;
}

Qt::ItemFlags PlaylistItemModel::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

PlaylistItemModel::PlaylistItemModel(db::ILibraryDatabase *libraryDatabase)
    : m_libraryDatabase(libraryDatabase) {
  const auto entries = m_libraryDatabase->getAllPlaylists();

  for (const auto &entry : entries) {
    m_items.emplace_back(
        Item({entry.id, QString::fromStdString(entry.displayName)}));
  }
}

int PlaylistItemModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}

QVariant PlaylistItemModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  auto item = m_items.at(index.row());

  switch (role) {
  case PlaylistId:
    return item.playlistId;
  case DisplayName:
    return item.displayName;
  default:
    return QVariant{};
  }
}

QHash<int, QByteArray> PlaylistItemModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[PlaylistId] = "id";
  roles[DisplayName] = "display_name";
  return roles;
}

void PlaylistItemModel::addPlaylist(const QString &displayName) {

  auto newPlaylist =
      db::Playlist{.id = -1, .displayName = displayName.toStdString()};
  m_libraryDatabase->createPlaylist(newPlaylist);

  beginInsertRows(QModelIndex(), rowCount(QModelIndex()),
                  rowCount(QModelIndex()));

  m_items.push_back({newPlaylist.id, displayName});

  endInsertRows();
}
void PlaylistItemModel::removePlaylist(const int playlistId) {
  m_libraryDatabase->deletePlaylist(playlistId);

  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items[i].playlistId == playlistId) {
      beginRemoveRows(QModelIndex(), i, i);
      m_items.erase(m_items.begin() + i);
      endRemoveRows();
      break;
    }
  }
}

void PlaylistItemModel::renamePlaylist(const int playlistId,
                                       const QString &newName) {
  m_libraryDatabase->renamePlaylist(playlistId, newName.toStdString());

  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items.at(i).playlistId == playlistId) {
      setData(createIndex(i, 0), newName, DisplayName);
      break;
    }
  }
}

void PlaylistItemModel::addEntryToPlaylist(const int playlistId,
                                           const int entryId) {
  m_libraryDatabase->addEntryToPlaylist(playlistId, entryId);
}

} // namespace firelight::gui