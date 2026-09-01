#pragma once
#include "library_filter.hpp"

#include <QSortFilterProxyModel>
#include <library/gui/entry_list_model.hpp>

namespace firelight::gui {

/**
 * A sorted, filtered view over the library entries.
 *
 * Filter values are staged rather than applied as they are set: a setter records what the user asked
 * for and applyFilters commits it, so changing several at once costs one pass. Mapping rows, telling
 * the view what moved, and forwarding a source change through to the delegate are all the proxy's job.
 *
 * The rule, without exception: every property reads the pending edit, and filterAcceptsRow is the
 * only thing that reads the applied snapshot
 */
class LibraryEntrySortFilterModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(QAbstractListModel *sourceModel READ getSourceModel WRITE setSourceModel NOTIFY sourceModelChanged)
  Q_PROPERTY(int count READ getCount NOTIFY countChanged)
  Q_PROPERTY(SortRole sortRole READ getSortRole WRITE setSortRole NOTIFY sortRoleChanged)
  Q_PROPERTY(QString sortDisplayName READ getSortDisplayName NOTIFY sortRoleChanged)
  Q_PROPERTY(bool sortAscending READ isSortAscending WRITE setSortAscending NOTIFY sortAscendingChanged)
  Q_PROPERTY(QVariantMap countByPlatform READ getCountByPlatform NOTIFY countByPlatformChanged)
  Q_PROPERTY(bool anyFiltersActive READ anyFiltersActive NOTIFY filtersOrSortChanged)
  Q_PROPERTY(QVariantList sortOptions READ getSortOptions CONSTANT)
  Q_PROPERTY(firelight::gui::LibraryFilter *filter READ getFilter CONSTANT)
  Q_PROPERTY(bool pending READ isPending NOTIFY filtersOrSortChanged)

public:
  enum SortRole {
    DisplayName = library::EntryListModel::DisplayName,
    LastPlayedAt = library::EntryListModel::LastPlayedAt,
    NumSecondsPlayed = library::EntryListModel::NumSecondsPlayed,
    AchievementsEarned = library::EntryListModel::AchievementsEarned,
    CreatedAt = library::EntryListModel::CreatedAt,
    ReleaseYear = library::EntryListModel::ReleaseYear
  };
  Q_ENUM(SortRole)

  explicit LibraryEntrySortFilterModel(QObject *parent = nullptr);

  [[nodiscard]] int getCount() const;

  [[nodiscard]] QAbstractListModel *getSourceModel() const;
  void setSourceModel(QAbstractListModel *sourceModel);

  [[nodiscard]] SortRole getSortRole() const;
  void setSortRole(SortRole sortRole);

  [[nodiscard]] QString getSortDisplayName() const;

  [[nodiscard]] bool isSortAscending() const;
  void setSortAscending(bool sortAscending);

  [[nodiscard]] bool anyFiltersActive() const;

  /**
   * @return The staged criteria, which QML edits directly
   */
  [[nodiscard]] LibraryFilter *getFilter() const;

  /**
   * @return Whether a staged value differs from what the current pass is showing
   */
  [[nodiscard]] bool isPending() const;

  /**
   * @return Every sort a view can offer: the label, the enum value, and the role name a folder
   *         stores. One table, so a folder's remembered sort and the menu cannot name different
   *         sets
   */
  [[nodiscard]] static QVariantList getSortOptions();

  /**
   * Commits every staged value and re-runs the pass
   */
  Q_INVOKABLE void applyFilters();

  /**
   * The same, against a fixed clock. A rolling window like playedWithinDays is resolved once per
   * pass rather than per row, so it cannot move partway through one
   */
  void applyFilters(qint64 nowMillis);

  /**
   * @return The entry id at a visible row, or -1 when the row is out of range
   */
  [[nodiscard]] Q_INVOKABLE int getEntryIdAt(int row) const;

  // TODO
  // Forwarded because this model stands where the entry model used to, under the
  // name QML already reaches for
  /**
   * @return How many entries each platform holds, before filtering
   */
  [[nodiscard]] QVariantMap getCountByPlatform() const;

  Q_INVOKABLE void removeEntryFromFolder(int entryId, int folderId);

  Q_INVOKABLE void setEntryFavorite(int entryId, bool favorite);

  /**
   * Forgets every criterion and re-runs the pass
   */
  Q_INVOKABLE void clearAllFilters();

signals:
  void sourceModelChanged();

  void countChanged();

  void sortRoleChanged();

  void sortAscendingChanged();

  void countByPlatformChanged();

  void filtersOrSortChanged();

protected:
  [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
  library::EntryListModel *m_sourceModel{};

  // What the user has asked for and not yet applied
  LibraryFilter *m_filter;
  SortRole m_pendingSortRole{DisplayName};
  bool m_pendingSortAscending{true};

  // What the current pass is filtering by. Read only by filterAcceptsRow, so a getter cannot
  // hand one of these back by mistake
  library::SmartFolderCriteria m_appliedCriteria{};
  qint64 m_nowMillis{0};
};

} // namespace firelight::gui
