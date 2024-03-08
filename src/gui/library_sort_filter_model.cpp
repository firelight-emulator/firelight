#include "library_sort_filter_model.hpp"
#include "library_item_model.hpp"

namespace firelight::gui {

void LibrarySortFilterModel::filterOnPlaylistId(const int playlistId) {
  m_playlistId = playlistId;
  invalidateFilter();
}
void LibrarySortFilterModel::sortByDisplayName() {
  m_sortType = SortType::DisplayName;
  sort(0);
}
void LibrarySortFilterModel::sortByCreatedAt() {
  m_sortType = SortType::CreatedAt;
  sort(0);
}

bool LibrarySortFilterModel::filterAcceptsRow(
    int sourceRow, const QModelIndex &sourceParent) const {
  if (m_playlistId == -1) {
    return true;
  }

  const QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
  auto data = sourceModel()->data(index0, LibraryItemModel::Playlists).toList();
  for (const auto &playlistId : data) {
    if (playlistId.toInt() == m_playlistId) {
      return true;
    }
  }

  return false;
}

bool LibrarySortFilterModel::lessThan(const QModelIndex &source_left,
                                      const QModelIndex &source_right) const {
  if (m_sortType == SortType::DisplayName) {
    const QString leftDisplayName =
        sourceModel()
            ->data(source_left, LibraryItemModel::DisplayName)
            .toString();
    const QString rightDisplayName =
        sourceModel()
            ->data(source_right, LibraryItemModel::DisplayName)
            .toString();
    // Compare the display names
    return QString::compare(leftDisplayName, rightDisplayName,
                            Qt::CaseInsensitive) < 0;
  }
  if (m_sortType == SortType::CreatedAt) {
    const auto leftCreatedTime =
        sourceModel()
            ->data(source_left, LibraryItemModel::CreatedAt)
            .toLongLong();
    const auto rightCreatedTime =
        sourceModel()
            ->data(source_right, LibraryItemModel::CreatedAt)
            .toLongLong();

    // TODO: This is stupid, it just reverses the list.
    return leftCreatedTime > rightCreatedTime;
  }

  return false;
}
} // namespace firelight::gui