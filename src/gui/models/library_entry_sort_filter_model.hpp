#pragma once
#include <QAbstractListModel>
#include <QSet>
#include <QTimer>
#include <library/gui/entry_list_model.hpp>
#include <spdlog/spdlog.h>

namespace firelight::gui {

/**
 * A sorted, filtered view over the library entries.
 *
 * It owns the visible order rather than mapping index by index, so filtering and sorting happen in
 * one pass and a row's sort key is read once per pass instead of once per comparison.
 */
class LibraryEntrySortFilterModel : public QAbstractListModel {
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

  [[nodiscard]] bool anyFiltersActive() const;

  [[nodiscard]] static QVariantList getSortOptions();

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

  Q_INVOKABLE void clearAllFilters() {
    setFilterText({});
    setFavoritesOnly(false);
    setPlatformIds({});
  }

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

  bool setData(const QModelIndex &index, const QVariant &value, int role) override;

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

signals:
  void sourceModelChanged();

  void countChanged();

  void filterTextChanged();

  void favoritesOnlyChanged();

  void platformIdsChanged();

  void sortRoleChanged();

  void sortAscendingChanged();

  void countByPlatformChanged();

  void filtersOrSortChanged();

private:
  /**
   * @return Whether the source row passes every active filter
   */
  [[nodiscard]] bool accepts(const QModelIndex &sourceIndex) const;

  /**
   * Queues a rebuild for the end of the event loop turn, so setting several properties together
   * costs one pass
   */
  void scheduleApply();

  /**
   * The source row a visible row maps to, or an invalid index when the row is out of range
   */
  [[nodiscard]] QModelIndex sourceIndexFor(int row) const;

  library::EntryListModel *m_sourceModel{};
  std::vector<int> m_sourceRows{};

  QString m_filterText{};
  bool m_favoritesOnly{false};
  QVariantList m_platformIds{};
  QSet<int> m_acceptedPlatformIds{};
  SortRole m_sortRole{DisplayName};
  bool m_sortAscending{true}; // Set a default for each role

  QTimer m_applyTimer;
};

} // namespace firelight::gui
