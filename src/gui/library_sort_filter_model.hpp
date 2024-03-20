#pragma once

#include <QSortFilterProxyModel>

namespace firelight::gui {

class LibrarySortFilterModel final : public QSortFilterProxyModel {
  Q_OBJECT

public slots:
  void filterOnPlaylistId(int playlistId);
  [[nodiscard]] int currentPlaylistId() const;
  void sortByDisplayName();
  void sortByCreatedAt();

protected:
  [[nodiscard]] bool
  filterAcceptsRow(int source_row,
                   const QModelIndex &sourceParent) const override;
  [[nodiscard]] bool lessThan(const QModelIndex &source_left,
                              const QModelIndex &source_right) const override;

private:
  enum class SortType { DisplayName, CreatedAt };

  SortType m_sortType = SortType::DisplayName;
  int m_playlistId = -1;
};

} // namespace firelight::gui
