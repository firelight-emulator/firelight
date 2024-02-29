//
// Created by alexs on 2/29/2024.
//

#include "library_sort_filter_model.hpp"

namespace Firelight {

void LibrarySortFilterModel::filterOnPlaylistId(int playlistId) {
  m_playlistId = playlistId;
  invalidateFilter();
}

bool LibrarySortFilterModel::filterAcceptsRow(
    int sourceRow, const QModelIndex &sourceParent) const {
  if (m_playlistId == -1) {
    return true;
  }

  // QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

  return sourceRow == m_playlistId;
}

} // namespace Firelight