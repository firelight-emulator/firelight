#include "entry_sort_filter_list_model.hpp"

#include "entry_list_model.hpp"

namespace firelight::library {

EntrySortFilterListModel::EntrySortFilterListModel(QObject *parent)
    : QSortFilterProxyModel(parent) {}
int EntrySortFilterListModel::getFolderId() const { return m_folderId; }
void EntrySortFilterListModel::setFolderId(const int folderId) {
  if (m_folderId == folderId) {
    return;
  }

  emit beginFilterChange();
  m_folderId = folderId;
  emit folderIdChanged();
  emit invalidateFilter();
}

bool EntrySortFilterListModel::filterAcceptsRow(
    int source_row, const QModelIndex &source_parent) const {
  if (m_folderId == -1) {
    return true;
  }

  auto index = sourceModel()->index(source_row, 0, source_parent);

  return sourceModel()
      ->data(index, EntryListModel::FolderIds)
      .toList()
      .contains(m_folderId);
}
} // namespace firelight::library
