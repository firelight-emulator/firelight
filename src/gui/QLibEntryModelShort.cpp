//
// Created by alexs on 12/22/2023.
//

#include "QLibEntryModelShort.hpp"

int QLibEntryModelShort::rowCount(const QModelIndex &parent) const {
  return m_entries.size();
}

QVariant QLibEntryModelShort::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= m_entries.size())
    return {};

  const FL::Library::Entry &entry = m_entries.at(index.row());
  switch (role) {
  case DisplayName:
    return QString::fromStdString(entry.display_name);
  case EntryId:
    return {entry.id};
    // Handle other roles
  }

  return {};
}

QHash<int, QByteArray> QLibEntryModelShort::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[DisplayName] = "displayName";
  roles[EntryId] = "entryId";
  // Add other roles
  return roles;
}

void QLibEntryModelShort::setEntries(
    const std::vector<FL::Library::Entry> &entries) {
  beginResetModel();
  m_entries = entries;
  endResetModel();
}
