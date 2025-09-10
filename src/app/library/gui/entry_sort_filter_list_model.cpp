#include "entry_sort_filter_list_model.hpp"

#include "entry_list_model.hpp"

#include <spdlog/spdlog.h>

namespace firelight::library {

EntrySortFilterListModel::EntrySortFilterListModel(QObject *parent)
    : QSortFilterProxyModel(parent) {
  // setDynamicSortFilter(true);
}
int EntrySortFilterListModel::getFolderId() const { return m_folderId; }
void EntrySortFilterListModel::setFolderId(const int folderId) {
  if (m_folderId == folderId) {
    return;
  }

  emit beginFilterChange();
  m_folderId = folderId;
  emit folderIdChanged();
  emit invalidateFilter();
  sort(0);
}

QString EntrySortFilterListModel::getSortMethod() const { return m_sortMethod; }
void EntrySortFilterListModel::setSortMethod(const QString &method) {
  if (m_sortMethod == method) {
    return;
  }

  m_sortMethod = method;
  invalidate();
  sort(0);
  emit sortMethodChanged();
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

bool EntrySortFilterListModel::lessThan(const QModelIndex &source_left,
                                        const QModelIndex &source_right) const {
  if (m_sortMethod == "alphabetical") {
    auto leftData =
        sourceModel()->data(source_left, EntryListModel::Roles::DisplayName);
    auto rightData =
        sourceModel()->data(source_right, EntryListModel::Roles::DisplayName);

    return QString::localeAwareCompare(leftData.toString(),
                                       rightData.toString()) < 0;
  } else if (m_sortMethod == "newest") {
    auto leftData =
        sourceModel()->data(source_left, EntryListModel::Roles::CreatedAt);
    auto rightData =
        sourceModel()->data(source_right, EntryListModel::Roles::CreatedAt);

    return leftData.toULongLong() > rightData.toULongLong();
  } else if (m_sortMethod == "recent") {
    auto leftData =
        sourceModel()->data(source_left, EntryListModel::Roles::LastPlayedAt);
    auto rightData =
        sourceModel()->data(source_right, EntryListModel::Roles::LastPlayedAt);

    auto leftTime = leftData.toULongLong();
    auto rightTime = rightData.toULongLong();

    // Get game names for debugging
    auto leftName = sourceModel()
                        ->data(source_left, EntryListModel::Roles::DisplayName)
                        .toString();
    auto rightName =
        sourceModel()
            ->data(source_right, EntryListModel::Roles::DisplayName)
            .toString();

    // Handle unplayed games (0 timestamp) - put them at the bottom
    if (leftTime == 0 && rightTime == 0) {
      return false; // Both unplayed, maintain order
    }
    if (leftTime == 0) {
      return false; // Left is unplayed, right should come first
    }
    if (rightTime == 0) {
      return true; // Right is unplayed, left should come first
    }

    // Both have been played - more recent first
    bool result = leftTime > rightTime;
    return result;
  }

  auto leftData = sourceModel()->data(source_left, EntryListModel::Roles::Id);
  auto rightData = sourceModel()->data(source_right, EntryListModel::Roles::Id);

  return leftData.toInt() < rightData.toInt();
  // return QSortFilterProxyModel::lessThan(source_left, source_right);
}
} // namespace firelight::library
