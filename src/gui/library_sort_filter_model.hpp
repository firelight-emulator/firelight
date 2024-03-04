#pragma once

#include <QSortFilterProxyModel>

namespace firelight::gui {

class LibrarySortFilterModel final : public QSortFilterProxyModel {
  Q_OBJECT

public slots:
  void filterOnPlaylistId(int playlistId);

protected:
  [[nodiscard]] bool
  filterAcceptsRow(int source_row,
                   const QModelIndex &source_parent) const override;
  [[nodiscard]] bool lessThan(const QModelIndex &source_left,
                              const QModelIndex &source_right) const override;

private:
  int m_playlistId = -1;
};

} // namespace firelight::gui
