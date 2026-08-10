#pragma once
#include <QSet>
#include <QSortFilterProxyModel>
#include <library/gui/entry_list_model.hpp>

namespace firelight::gui {

/**
 * A sorted, filtered view over the library entries.
 *
 * Filter values are staged rather than applied as they are set: a setter records what the user asked
 * for and applyFilters commits it, so changing several at once costs one pass. Mapping rows, telling
 * the view what moved, and forwarding a source change through to the delegate are all the proxy's job
 */
class LibraryEntrySortFilterModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(QAbstractListModel *sourceModel READ getSourceModel WRITE setSourceModel NOTIFY sourceModelChanged)
  Q_PROPERTY(int count READ getCount NOTIFY countChanged)
  Q_PROPERTY(QString filterText READ getFilterText WRITE setFilterText NOTIFY filterTextChanged)
  Q_PROPERTY(bool favoritesOnly READ isFavoritesOnly WRITE setFavoritesOnly NOTIFY favoritesOnlyChanged)
  Q_PROPERTY(QVariantList platformIds READ getPlatformIds WRITE setPlatformIds NOTIFY platformIdsChanged)
  Q_PROPERTY(SortRole sortRole READ getSortRole WRITE setSortRole NOTIFY sortRoleChanged)
  Q_PROPERTY(bool sortAscending READ isSortAscending WRITE setSortAscending NOTIFY sortAscendingChanged)
  Q_PROPERTY(QVariantMap countByPlatform READ getCountByPlatform NOTIFY countByPlatformChanged)
  Q_PROPERTY(bool anyFiltersActive READ anyFiltersActive NOTIFY filtersOrSortChanged)
  Q_PROPERTY(QVariantList sortOptions READ getSortOptions CONSTANT)
  Q_PROPERTY(bool collapseVariants READ isCollapseVariants WRITE setCollapseVariants NOTIFY collapseVariantsChanged)

public:
  enum SortRole {
    DisplayName = library::EntryListModel::DisplayName,
    LastPlayedAt = library::EntryListModel::LastPlayedAt,
    NumSecondsPlayed = library::EntryListModel::NumSecondsPlayed,
    AchievementsEarned = library::EntryListModel::AchievementsEarned,
    CreatedAt = library::EntryListModel::CreatedAt
  };
  Q_ENUM(SortRole)

  explicit LibraryEntrySortFilterModel(QObject *parent = nullptr);

  [[nodiscard]] int getCount() const;

  [[nodiscard]] QAbstractListModel *getSourceModel() const;
  void setSourceModel(QAbstractListModel *sourceModel);

  [[nodiscard]] QString getFilterText() const;
  void setFilterText(const QString &filterText);

  [[nodiscard]] bool isFavoritesOnly() const;
  void setFavoritesOnly(bool favoritesOnly);

  [[nodiscard]] QVariantList getPlatformIds() const;
  void setPlatformIds(const QVariantList &platformIds);

  [[nodiscard]] SortRole getSortRole() const;
  void setSortRole(SortRole sortRole);

  [[nodiscard]] bool isSortAscending() const;
  void setSortAscending(bool sortAscending);

  [[nodiscard]] bool isCollapseVariants() const;
  void setCollapseVariants(bool collapseVariants);

  [[nodiscard]] bool anyFiltersActive() const;

  [[nodiscard]] static QVariantList getSortOptions();

  /**
   * Commits every staged value and re-runs the pass
   */
  Q_INVOKABLE void applyFilters();

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

  Q_INVOKABLE bool matchesSmartFolder(int folderId, int entryId);

  Q_INVOKABLE void removeEntryFromFolder(int entryId, int folderId);

  Q_INVOKABLE void setEntryFavorite(int entryId, bool favorite);

  /**
   * @return The entry ids in a variant group, the one standing for it first
   */
  [[nodiscard]] Q_INVOKABLE QVariantList getVariantEntryIds(int groupId) const;

  Q_INVOKABLE void clearAllFilters() {
    setFilterText({});
    setFavoritesOnly(false);
    setPlatformIds({});
  }

signals:
  void sourceModelChanged();

  void countChanged();

  void filterTextChanged();

  void favoritesOnlyChanged();

  void platformIdsChanged();

  void sortRoleChanged();

  void sortAscendingChanged();

  void countByPlatformChanged();

  void collapseVariantsChanged();

  void filtersOrSortChanged();

protected:
  [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
  library::EntryListModel *m_sourceModel{};

  // TODO
  // What the user has asked for but not yet committed. A setter writes only here, so a
  // source change re-running the filter cannot apply a value nobody asked to apply
  QString m_pendingFilterText{};
  bool m_pendingFavoritesOnly{false};
  QVariantList m_pendingPlatformIds{};
  SortRole m_pendingSortRole{DisplayName};
  bool m_pendingSortAscending{true};

  // What the current pass is filtering by
  QString m_filterText{};
  bool m_favoritesOnly{false};
  QSet<int> m_acceptedPlatformIds{};
  bool m_collapseVariants{true};
};

} // namespace firelight::gui
