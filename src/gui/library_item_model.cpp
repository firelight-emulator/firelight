//
// Created by alexs on 2/28/2024.
//

#include "library_item_model.hpp"

namespace Firelight {
LibraryItemModel::LibraryItemModel() {
  m_items.emplace_back("Item 1");
  m_items.emplace_back("Item 2");
  m_items.emplace_back("Item 3");
  m_items.emplace_back("Item 4");
  m_items.emplace_back("Item 5");
}
int LibraryItemModel::rowCount(const QModelIndex &parent) const { return 5; }

QVariant LibraryItemModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  auto item = m_items.at(index.row());

  switch (role) {
  case EntryId:
    return index.row();
  case DisplayName:
    return item;
  case PlatformName:
    return "testing";
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