#include "library_sort_filter_model.hpp"
#include "library_item_model.hpp"

namespace firelight::gui {

void LibrarySortFilterModel::filterOnPlaylistId(const int playlistId) {
  m_sortType = SortType::DisplayName;
  m_playlistId = playlistId;
  invalidate();
  sort(0);
}

int LibrarySortFilterModel::currentPlaylistId() const { return m_playlistId; }

QString LibrarySortFilterModel::sortType() const {
  if (m_sortType == SortType::DisplayName) {
    return "display_name";
  }
  if (m_sortType == SortType::CreatedAt) {
    return "created_at";
  }
  // if (m_sortType == SortType::LastPlayedAt) {
  //   return "last_played_at";
  // }

  return "";
}

void LibrarySortFilterModel::setSortType(const QString &sortType) {
  SortType newSortType;

  if (sortType == "display_name") {
    newSortType = SortType::DisplayName;
  } else if (sortType == "created_at") {
    newSortType = SortType::CreatedAt;
    // } else if (sortType == "last_played_at") {
    //   newSortType = SortType::LastPlayedAt;
  } else {
    return;
  }

  if (newSortType != m_sortType) {
    m_sortType = newSortType;
    invalidate();
    sort(0);
    emit sortTypeChanged();
  }
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
  // if (m_sortType == SortType::LastPlayedAt) {
  //   const auto leftPlayedTime =
  //       sourceModel()
  //           ->data(source_left, LibraryItemModel::LastPlayedAt)
  //           .toLongLong();
  //   const auto rightPlayedTime =
  //       sourceModel()
  //           ->data(source_right, LibraryItemModel::LastPlayedAt)
  //           .toLongLong();
  //
  //   printf("left (%s): %ld, right (%s): %ld\n",
  //          sourceModel()
  //              ->data(source_left, LibraryItemModel::DisplayName)
  //              .toString()
  //              .toStdString()
  //              .c_str(),
  //          leftPlayedTime,
  //          sourceModel()
  //              ->data(source_right, LibraryItemModel::DisplayName)
  //              .toString()
  //              .toStdString()
  //              .c_str(),
  //          rightPlayedTime);
  //
  //   // TODO: This is stupid, it just reverses the list.
  //   return leftPlayedTime > rightPlayedTime;
  // }

  return false;
}
} // namespace firelight::gui