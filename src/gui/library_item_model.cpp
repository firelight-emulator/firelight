//
// Created by alexs on 2/28/2024.
//

#include "library_item_model.hpp"

namespace Firelight {
LibraryItemModel::LibraryItemModel(LibraryDatabase *libraryDatabase)
    : m_libraryDatabase(libraryDatabase) {

  const auto entries = m_libraryDatabase->getAllEntries();
  auto i = 0;
  for (const auto &entry : entries) {
    m_items.emplace_back(
        Item({entry.id, QString::fromStdString(entry.display_name), {i}}));
    i++;
    if (i > 4) {
      i = 0;
    }
  }
}

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
    return item.displayName;
  case PlatformName:
    return "Nintendo 64";
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
} // namespace Firelight