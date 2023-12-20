//
// Created by alexs on 12/18/2023.
//

#include "rom_model.hpp"
QHash<int, QByteArray> RomModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[FilenameRole] = "filename";
  roles[SizeRole] = "size";
  return roles;
}
QVariant RomModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  //  if (role < Qt::UserRole) {
  //    return QAbstractListModel::data(index, role);
  //  }

  return QVariant(QStringLiteral("hey there"));
}
int RomModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }

  return 100;
}
RomModel::RomModel(QObject *parent) : QAbstractListModel(parent) {}
