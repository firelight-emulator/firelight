#include "library_sort_filter_model.hpp"
#include "library_item_model.hpp"

namespace firelight::gui {

void LibrarySortFilterModel::filterOnPlaylistId(int playlistId) {
  m_playlistId = playlistId;
  invalidateFilter();
}

bool LibrarySortFilterModel::filterAcceptsRow(
    int sourceRow, const QModelIndex &sourceParent) const {
  if (m_playlistId == -1) {
    return true;
  }

  QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
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

} // namespace firelight::gui