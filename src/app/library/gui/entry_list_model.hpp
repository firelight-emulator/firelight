#pragma once
#include <firelight/event_dispatcher.hpp>
#include <firelight/library/entry.hpp>
#include <firelight/library/smart_folder.hpp>
#include <firelight/library/user_library_service.hpp>

#include <QAbstractListModel>
#include <QTimer>
#include <unordered_map>

namespace firelight::activity {
class IActivityLog;
}

namespace firelight::platforms {
class IPlatformService;
}

namespace firelight::achievements {
class AchievementService;
}

namespace firelight::library {
class EntryListModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ getCount NOTIFY countChanged)
  Q_PROPERTY(int numFavorites READ numFavorites NOTIFY numFavoritesChanged)
  Q_PROPERTY(QVariantMap countByPlatform READ getCountByPlatform NOTIFY countChanged)
  Q_PROPERTY(QVariantMap countByFolderId READ getCountByFolderId NOTIFY countByFolderIdChanged)
  // Grouping key mode read by the GroupKey role: "none" | "platform" | "decade"
  // | "year" | "genre" | "title"
  Q_PROPERTY(QString groupMode READ getGroupMode WRITE setGroupMode NOTIFY groupModeChanged)

public:
  /**
   * @enum Roles
   * @brief The roles that can be used with this model
   */
  enum Roles {
    Id = Qt::UserRole + 1,
    DisplayName,
    ContentHash,
    PlatformId,
    PlatformIconName,
    ActiveSaveSlot,
    Hidden,
    Favorite,
    Icon1x1SourceUrl,
    BoxartFrontSourceUrl,
    BoxartBackSourceUrl,
    Description,
    ReleaseYear,
    Developer,
    Publisher,
    Genres,
    RegionIds,
    FolderIds,
    ContentDirectoryIds,
    ContentPaths,
    CreatedAt,
    LastPlayedAt,
    NumSecondsPlayed,
    AchievementsEarned,
    AchievementsTotal,
    AchievementSetCount,
    Rating,
    GroupKey
  };

  struct Item {
    Entry entry;
    uint64_t numSecondsPlayed{};
    uint64_t lastPlayedEpochMillis{};
    int achievementsEarned{};
    int achievementsTotal{};
    int achievementSetCount{};
    // Cached group-header label for the current group mode, so data(GroupKey)
    // (read O(n log n) times by the proxy sorter) is a plain string return rather
    // than a per-call platform lookup
    QString groupKey;
  };

  EntryListModel(UserLibraryService &userLibrary, activity::IActivityLog &activityLog,
                 platforms::IPlatformService &platformService, achievements::AchievementService &achievementService,
                 QObject *parent = nullptr);

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

  bool setData(const QModelIndex &index, const QVariant &value, int role) override;

  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

  Q_INVOKABLE void addEntryToFolder(int entryId, int folderId);

  Q_INVOKABLE void removeEntryFromFolder(int entryId, int folderId);

  // Sets an entry's favorite flag by entry id. Multi-select bulk actions and
  // the context menu act on ids, not a delegate, so they can't go through
  // setData like the per-row heart does
  Q_INVOKABLE void setEntryFavorite(int entryId, bool favorite);

  // True if the entry satisfies the given smart folder's criteria. Used by
  // the client-side folder filter for smart folders (manual folders use
  // folderIds membership). Parsed criteria are cached per folder; call
  // invalidateSmartFolderCache() after a smart folder's criteria change
  Q_INVOKABLE bool matchesSmartFolder(int folderId, int entryId);

  // Drops cached smart-folder criteria so the next matchesSmartFolder /
  // count reflects edited criteria
  Q_INVOKABLE void invalidateSmartFolderCache();

  int getCount() const;

  int numFavorites() const;

  QVariantMap getCountByPlatform() const;

  QVariantMap getCountByFolderId() const;

  QString getGroupMode() const;

  void setGroupMode(const QString &mode);

public slots:
  void reset();

  void removeFolderId(int folderId);

signals:
  void countChanged();

  void numFavoritesChanged();

  void countByFolderIdChanged();

  void groupModeChanged();

private:
  // The group-header label for an item under the current group mode (platform
  // name, decade, year, first letter, ...). Empty when grouping is off
  [[nodiscard]] QString computeGroupKey(const Item &item) const;

  // Reconciles a single entry with the model after a create/update event:
  // inserts a newly-visible entry, removes one that became hidden/deleted, or
  // updates one in place (the QML SortFilterProxyModel re-sorts/re-filters, so
  // the source order doesn't matter). Runs on the GUI thread only
  void syncEntry(int entryId);

  // Fills an item's play stats (total + last-played) from the activity log
  void applyPlayStats(Item &item) const;

  // Fills an item's earned/total achievement counts from the achievement
  // service (offline, by content hash). Cheap indexed lookups; run on reset
  // and after a play session ends (a session may have unlocked achievements)
  void applyAchievementCounts(Item &item) const;

  // Recomputes every row's achievement counts and notifies. Counts are
  // per-user, so this runs after login (which completes async, post-reset)
  // and after a session ends. Must run on the GUI thread
  void refreshAllAchievementCounts();

  // Rebuilds m_indexByEntryId to match m_items (after a structural change)
  void rebuildIndex();

  // Coalesces the count-property change notifications so a burst of syncEntry
  // calls (e.g. a scan importing several games) recomputes the counts once
  void scheduleCountsChanged();

  UserLibraryService &m_userLibrary;
  activity::IActivityLog &m_activityLog;
  platforms::IPlatformService &m_platformService;
  achievements::AchievementService &m_achievementService;
  QList<Item> m_items{};
  QString m_groupMode = "none";

  // Flattens an item (entry attributes + joined play stats) into the Qt-free
  // struct the smart-folder evaluator consumes
  [[nodiscard]] static EntryFields buildEntryFields(const Item &item);

  // Resolves (and memoizes) a smart folder's parsed criteria by id
  const SmartFolderCriteria &criteriaForFolder(int folderId) const;
  mutable std::unordered_map<int, SmartFolderCriteria> m_smartFolderCache;

  // entry id -> index into m_items, rebuilt on reset(); lets the per-entry
  // matchesSmartFolder lookup avoid an O(n) scan (so a filter pass is O(n),
  // not O(n^2))
  std::unordered_map<int, int> m_indexByEntryId;

  ScopedConnection m_gamePlayedConnection;
  ScopedConnection m_entryCreatedConnection;
  ScopedConnection m_entryUpdatedConnection;
  ScopedConnection m_achievementSessionEndedConnection;
  ScopedConnection m_userLoggedInConnection;

  // Fires once (single-shot, 0ms) after a burst of syncEntry calls to emit the
  // count-property change signals a single time
  QTimer m_countsChangedTimer;
};
} // namespace firelight::library
