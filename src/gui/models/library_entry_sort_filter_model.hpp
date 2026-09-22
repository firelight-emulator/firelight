// TODO: NEEDS REVIEW
#pragma once
#include "library_filter.hpp"

#include <QSortFilterProxyModel>
#include <library/gui/entry_list_model.hpp>
#include <library/gui/playlist_item_model.hpp>
#include <unordered_set>

namespace firelight::gui {

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
  Q_PROPERTY(int scopeFolderId READ getScopeFolderId WRITE setScopeFolderId NOTIFY filtersOrSortChanged)
  Q_PROPERTY(LibraryFolderListModel *folderModel READ getFolderModel WRITE setFolderModel NOTIFY folderModelChanged)
  Q_PROPERTY(int openFolderId READ getOpenFolderId WRITE setOpenFolderId NOTIFY openFolderChanged)
  Q_PROPERTY(bool openFolderIsSmart READ isOpenFolderSmart NOTIFY openFolderChanged)
  Q_PROPERTY(bool sortPinnedToOpenFolder READ isSortPinnedToOpenFolder WRITE setSortPinnedToOpenFolder NOTIFY
                 sortPinnedChanged)
  Q_PROPERTY(QVariantList pickedEntryIds READ getPickedEntryIds WRITE setPickedEntryIds NOTIFY pickedEntryIdsChanged)
  Q_PROPERTY(PickedMode pickedMode READ getPickedMode WRITE setPickedMode NOTIFY pickedModeChanged)

public:
  enum SortRole {
    DisplayName = library::EntryListModel::DisplayName,
    LastPlayedAt = library::EntryListModel::LastPlayedAt,
    NumSecondsPlayed = library::EntryListModel::NumSecondsPlayed,
    AchievementsEarned = library::EntryListModel::AchievementsEarned,
    CreatedAt = library::EntryListModel::CreatedAt,
    ReleaseYear = library::EntryListModel::ReleaseYear,
    Custom = library::EntryListModel::Position
  };
  Q_ENUM(SortRole)

  // TODO
  // Roles this view adds on top of the entry model's
  enum ProxyRole { RemovedFromScope = Qt::UserRole + 900 };
  Q_ENUM(ProxyRole)

  enum PickedMode { ShowAll, OnlyPicked, HidePicked };
  Q_ENUM(PickedMode)

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
   * @return The folder rows must belong to, or -1 for no scope
   */
  [[nodiscard]] int getScopeFolderId() const;

  void setScopeFolderId(int scopeFolderId);

  // TODO
  /**
   * @return The staged ids of the picked entries
   */
  [[nodiscard]] QVariantList getPickedEntryIds() const;

  // TODO
  /**
   * Stages the ids of the picked entries
   */
  void setPickedEntryIds(const QVariantList &pickedEntryIds);

  // TODO
  /**
   * @return The staged way picked entries narrow the rows
   */
  [[nodiscard]] PickedMode getPickedMode() const;

  // TODO
  /**
   * Stages the way picked entries narrow the rows
   */
  void setPickedMode(PickedMode pickedMode);

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

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

  /**
   * The model the open collection's row is read from
   */
  [[nodiscard]] LibraryFolderListModel *getFolderModel() const;

  void setFolderModel(LibraryFolderListModel *folderModel);

  /**
   * The collection on screen, or -1 for the whole library
   */
  [[nodiscard]] int getOpenFolderId() const;

  /**
   * Adopts a collection: membership for a manual one, saved criteria for a smart one, plus whatever
   * sort it has pinned. Commits in one pass
   */
  void setOpenFolderId(int folderId);

  /**
   * Puts the open collection's saved criteria back, discarding anything refined on top
   */
  Q_INVOKABLE void resetToSaved();

  /**
   * Whether the open collection computes its members from criteria
   */
  [[nodiscard]] bool isOpenFolderSmart() const;

  /**
   * Whether the open collection has a sort of its own
   */
  [[nodiscard]] bool isSortPinnedToOpenFolder() const;

  /**
   * Clearing the pin hands the collection back to the default sort. Setting it is a no-op; a pin is
   * made by sorting
   */
  void setSortPinnedToOpenFolder(bool pinned);

  /**
   * The sortOptions role name for a sort role, or empty when there is none
   */
  [[nodiscard]] static QString sortRoleName(SortRole sortRole);

  /**
   * The sort role a sortOptions role name names, or DisplayName when it names none
   */
  [[nodiscard]] static SortRole sortRoleForName(const QString &roleName);

signals:
  void sourceModelChanged();

  void folderModelChanged();

  void openFolderChanged();

  void sortPinnedChanged();

  void countChanged();

  void sortRoleChanged();

  void sortAscendingChanged();

  void countByPlatformChanged();

  void filtersOrSortChanged();

  void refinementChanged();

  void pickedEntryIdsChanged();

  void pickedModeChanged();

protected:
  [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
  library::EntryListModel *m_sourceModel{};

  // What the user has asked for and not yet applied
  LibraryFilter *m_filter;
  SortRole m_pendingSortRole{DisplayName};
  bool m_pendingSortAscending{true};
  int m_scopeFolderId{-1};
  QVariantList m_pendingPickedEntryIds;
  PickedMode m_pendingPickedMode{ShowAll};
  LibraryFolderListModel *m_folderModel{};
  int m_openFolderId{-1};
  bool m_adoptingFolder{false};

  library::SmartFolderCriteria m_appliedCriteria{};
  qint64 m_nowMillis{0};
  int m_appliedScopeFolderId{-1};

  // Which entries the scoped collection held when the pass ran. A row taken out of the collection
  // while it is on screen stays until the next pass rather than vanishing under the cursor
  std::unordered_set<int> m_appliedScopeMembers;

  std::unordered_set<int> m_appliedPickedEntryIds;
  PickedMode m_appliedPickedMode{ShowAll};

  [[nodiscard]] bool isInScope(const QModelIndex &sourceIndex) const;

  [[nodiscard]] const library::FolderInfo *openFolder() const;

  void pinSortToOpenFolder();
};

} // namespace firelight::gui
