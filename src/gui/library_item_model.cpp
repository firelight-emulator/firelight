#include "library_item_model.hpp"

namespace firelight::gui {

LibraryItemModel::LibraryItemModel(db::ILibraryDatabase *libraryDatabase)
    : m_libraryDatabase(libraryDatabase) {}

int LibraryItemModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}

QVariant LibraryItemModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  auto item = m_items.at(index.row());

  switch (role) {
  case EntryId:
    return item.m_entryId;
  case DisplayName:
  case Qt::DisplayRole:
    return item.displayName;
  case CreatedAt:
    return item.createdAt;
  case PlatformName: {
    if (item.platformId == 0) {
      return "Nintendo 64";
    }
    if (item.platformId == 1) {
      return "SNES";
    }
    if (item.platformId == 2) {
      return "Game Boy Color";
    }
    if (item.platformId == 3) {
      return "Game Boy";
    }
    if (item.platformId == 4) {
      return "Game Boy Advance";
    }
    if (item.platformId == 5) {
      return "Nintendo DS";
    }
    return "Unknown";
  }
  case Playlists:
    return QVariant::fromValue(item.m_playlists);
  default:
    return QVariant{};
  }
}

QHash<int, QByteArray> LibraryItemModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[EntryId] = "id";
  roles[DisplayName] = "display_name";
  roles[PlatformName] = "platform_name";
  return roles;
}

void LibraryItemModel::updatePlaylistsForEntry(const int entryId) {
  const auto playlists = m_libraryDatabase->getPlaylistsForEntry(entryId);
  QVector<int> playlistIds;
  for (const auto &playlist : playlists) {
    playlistIds.push_back(playlist.id);
  }

  for (auto &item : m_items) {
    if (item.m_entryId == entryId) {
      item.m_playlists = playlistIds;
      break;
    }
  }
}
void LibraryItemModel::refresh() {
  emit beginResetModel();
  m_items.clear();
  const auto entries = m_libraryDatabase->getAllLibraryEntries();

  for (const auto &entry : entries) {
    const auto playlists = m_libraryDatabase->getPlaylistsForEntry(entry.id);
    QVector<int> playlistIds;
    for (const auto &playlist : playlists) {
      playlistIds.push_back(playlist.id);
    }

    m_items.emplace_back(
        Item({entry.id, QString::fromStdString(entry.displayName),
              entry.platformId, playlistIds, entry.createdAt}));
  }
  emit endResetModel();
}

} // namespace firelight::gui