// TODO: NEEDS REVIEW
#pragma once
#include <firelight/event_dispatcher.hpp>
#include <firelight/library/entry.hpp>
#include <firelight/library/entry_status.hpp>
#include <firelight/library/library_events.hpp>
#include <firelight/library/smart_folder.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/settings/settings_service.hpp>

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
    GroupKey,
    VariantGroupId,
    VariantCount,
    IsVariantPrimary,
    VariantAutoLaunch,
    // Whether the game would start. False means the row is shown with a badge rather than
    // hidden, so nobody has to guess why a game will not play
    Playable,
    // Every EntryProblem that applies, worst first
    Problems,
    // What is wrong, in a sentence. Empty when nothing is
    StatusText,
    SearchText,
    // TODO
    // The row's cached filter fields, by pointer. Valid only for the duration of the call that
    // asked for it, because appending a row moves the ones already there
    FilterFields
  };

  /**
   * Re-reads what stands between each platform and running anything.
   *
   * A platform fact rather than an entry one, so it is looked up once instead of per row
   */
  void refreshPlatformProblems();

  /** Everything wrong with an entry, from the platform facts and the entry's own */
  [[nodiscard]] EntryStatus statusOf(const Entry &entry) const;

  /**
   * The same verdict as a sentence, naming what a bare problem cannot: which system, which BIOS
   * files, which discs
   */
  [[nodiscard]] QString describeStatus(const Entry &entry, const EntryStatus &status) const;

  /** The unreadable content root this entry sits under, empty when it is reachable */
  [[nodiscard]] QString unreachableRootFor(const Entry &entry) const;

  /** Re-reads the platform facts and every row's verdict from them */
  void refreshStatuses();

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
    // TODO
    // Cached for the same reason as groupKey: data() reads it per row per role, and
    // answering it for a disc set costs two queries
    EntryStatus status;
    // TODO
    // Composed alongside the status, because naming the system, the BIOS files and the missing
    // discs costs lookups that must not happen once per row per role
    QString statusText;
    // TODO
    // An entry in no variant group reads as a group of one, so nothing downstream has
    // to branch on whether grouping applies
    int variantGroupId = -1;
    int variantCount = 1;
    bool isVariantPrimary = true;
    bool variantAutoLaunch = false;
    QString variantTitle;
    // TODO
    // Playtime across the whole group, and the most recent session any of it saw
    uint64_t variantSecondsPlayed{};
    uint64_t variantLastPlayedMillis{};
    // TODO
    // Everything a text filter should match on one line: this entry's name, the group's
    // title, and the names of the variants it stands for
    QString searchText;

    // TODO
    // Everything the filter predicate reads, rebuilt whenever the row changes. Cached for the
    // same reason as groupKey: the proxy asks per row per pass, and rebuilding it there would
    // cost an allocation a row and a second copy of the mapping
    EntryFields fields;
  };

  /** Puts both the verdict and its sentence on a row, so neither is refreshed without the other */
  void applyStatus(Item &item) const;

  EntryListModel(UserLibraryService &userLibrary, activity::IActivityLog &activityLog,
                 platforms::IPlatformService &platformService, achievements::AchievementService &achievementService,
                 settings::SettingsService &settings, QObject *parent = nullptr);

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
  settings::SettingsService &m_settings;
  QList<Item> m_items{};
  QString m_groupMode = "none";

  // Flattens an item (entry attributes + joined play stats) into the Qt-free
  // struct the smart-folder evaluator consumes
  // TODO
  // Drops cached smart-folder criteria, so the next count re-reads them. Driven by
  // FolderChangedEvent rather than by a caller remembering to ask
  void invalidateSmartFolderCache();

  [[nodiscard]] static EntryFields buildEntryFields(const Item &item);

  // Resolves (and memoizes) a smart folder's parsed criteria by id
  const SmartFolderCriteria &criteriaForFolder(int folderId) const;
  mutable std::unordered_map<int, SmartFolderCriteria> m_smartFolderCache;

  // entry id -> index into m_items, rebuilt on reset(); lets the per-entry
  // matchesSmartFolder lookup avoid an O(n) scan (so a filter pass is O(n),
  // not O(n^2))
  std::unordered_map<int, int> m_indexByEntryId;

  // What stands between each platform and running anything, read once per reset rather than
  // per row. Absent means nothing does
  QHash<int, EntryProblem> m_problemByPlatformId;

  // What a sentence needs to name the platform's half of a problem, cached beside it because
  // both come from the same pass over the platforms
  QHash<int, QString> m_platformNameById;
  QHash<int, QString> m_missingBiosByPlatformId;

  // Content roots that cannot be read at the moment, resolved once per refresh for the same
  // reason the platform problems are. Empty is the ordinary case
  QStringList m_unreachableRoots;

  // TODO
  // The content directories still in the library, so a file whose directory is not among them
  // reads as one whose folder was taken out rather than one that was deleted
  QSet<int> m_knownContentDirectoryIds;

  // TODO
  // The entries of each group, so re-picking a primary after one member changes reads
  // what is already loaded rather than the database

  // TODO
  // Fills in the variant fields and the group's totals across every row, then hands back
  // which entry stands for each group
  void refreshRowFields();

  // TODO
  // Re-picks the primary for one group and tells the view about every row that changed,
  // including the one that stopped standing for it

  // TODO
  // Name, group title, and the variants' names, lowercased for the filter to match against
  [[nodiscard]] QString computeSearchText(const Item &item) const;

  ScopedConnection m_folderChangedConnection;
  ScopedConnection m_gamePlayedConnection;
  ScopedConnection m_entryCreatedConnection;
  ScopedConnection m_entryUpdatedConnection;
  ScopedConnection m_entryDeletedConnection;
  ScopedConnection m_achievementSessionEndedConnection;
  ScopedConnection m_userLoggedInConnection;
  ScopedConnection m_coreSettingChangedConnection;
  ScopedConnection m_coreSettingResetConnection;

  // Fires once (single-shot, 0ms) after a burst of syncEntry calls to emit the
  // count-property change signals a single time
  QTimer m_countsChangedTimer;
};
} // namespace firelight::library

// TODO
// Handed to the proxy through the FilterFields role, so the predicate reads the row's cached
// record rather than rebuilding one
Q_DECLARE_METATYPE(const firelight::library::EntryFields *)
