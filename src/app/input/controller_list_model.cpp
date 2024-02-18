//
// Created by alexs on 2/16/2024.
//

#include "controller_list_model.hpp"

namespace Firelight::Input {
bool ControllerListModel::insertRows(int row, int count,
                                     const QModelIndex &parent) {
  return QAbstractListModel::insertRows(row, count, parent);
}
bool ControllerListModel::insertColumns(int column, int count,
                                        const QModelIndex &parent) {
  return QAbstractListModel::insertColumns(column, count, parent);
}
bool ControllerListModel::removeRows(int row, int count,
                                     const QModelIndex &parent) {
  return QAbstractListModel::removeRows(row, count, parent);
}
bool ControllerListModel::removeColumns(int column, int count,
                                        const QModelIndex &parent) {
  return QAbstractListModel::removeColumns(column, count, parent);
}
int ControllerListModel::rowCount(const QModelIndex &parent) const { return 0; }

QVariant ControllerListModel::data(const QModelIndex &index, int role) const {
  return QVariant{};
}
} // namespace Firelight::Input