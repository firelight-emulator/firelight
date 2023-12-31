//
// Created by alexs on 12/22/2023.
//

#include "QLibraryViewModel.hpp"

int QLibraryViewModel::rowCount(const QModelIndex &parent) const {
  return items_.size();
}

bool QLibraryViewModel::insertRows(int row, int count,
                                   const QModelIndex &parent) {
  beginInsertRows(parent, row, row + count);
  // TODO: insert the rows
  endInsertRows();

  return true;
}
bool QLibraryViewModel::removeRows(int row, int count,
                                   const QModelIndex &parent) {
  beginRemoveRows(parent, row, row + count);
  // TODO: remove the rows
  endRemoveRows();

  return true;
}

void QLibraryViewModel::set_items(const std::vector<Item> &items) {
  beginResetModel();
  items_ = items;
  endResetModel();

  // emit dataChanged(index(0), index(items.size() - 1),
  //                  QList<int>{EntryId, DisplayName});
}

QVariant QLibraryViewModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= items_.size()) {
    return QVariant{};
  }

  auto item = items_.at(index.row());

  switch (role) {
  case EntryId:
    return item.id;
  case DisplayName:
    return QString::fromStdString(item.display_name);
  default:
    return QVariant{};
  }
}

QHash<int, QByteArray> QLibraryViewModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[EntryId] = "id";
  roles[DisplayName] = "display_name";
  return roles;
}