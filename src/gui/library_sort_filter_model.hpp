//
// Created by alexs on 2/29/2024.
//

#ifndef LIBRARY_SORT_FILTER_MODEL_HPP
#define LIBRARY_SORT_FILTER_MODEL_HPP
#include <QSortFilterProxyModel>

namespace firelight {

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

} // namespace Firelight

#endif // LIBRARY_SORT_FILTER_MODEL_HPP
