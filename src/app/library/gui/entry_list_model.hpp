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
#include <optional>
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
    Position,
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
    Playable,
    Problems,
    StatusText,
    SearchText,
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
    QString groupKey;
    EntryStatus status;
    QString statusText;
    int variantGroupId = -1;
    int variantCount = 1;
    bool isVariantPrimary = true;
    bool variantAutoLaunch = false;
    QString variantTitle;
    uint64_t variantSecondsPlayed{};
    uint64_t variantLastPlayedMillis{};
    QString searchText;
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

  // TODO
  /** Adds each listed entry that is in the model and not yet in the folder */
  Q_INVOKABLE void addEntriesToFolder(int folderId, const QVariantList &entryIds);

  Q_INVOKABLE void removeEntryFromFolder(int entryId, int folderId);

  // Only exists for bulk updates
  Q_INVOKABLE void setEntryFavorite(int entryId, bool favorite);

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
  [[nodiscard]] QString computeGroupKey(const Item &item) const;

  // Reconciles a single entry with the model after a create/update event:
  // inserts a newly-visible entry, removes one that became hidden/deleted, or
  // updates one in place (the QML SortFilterProxyModel re-sorts/re-filters, so
  // the source order doesn't matter). Runs on the GUI thread only
  void syncEntry(int entryId);

  void applyPlayStats(Item &item) const;

  void applyAchievementCounts(Item &item) const;

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

  void invalidateSmartFolderCache();
  void invalidateCountByFolderId();

  mutable std::optional<QVariantMap> m_countByFolderId;

  [[nodiscard]] static EntryFields buildEntryFields(const Item &item);

  const SmartFolderCriteria &criteriaForFolder(int folderId) const;
  mutable std::unordered_map<int, SmartFolderCriteria> m_smartFolderCache;

  std::unordered_map<int, int> m_indexByEntryId;

  // What stands between each platform and running anything
  QHash<int, EntryProblem> m_problemByPlatformId;

  QHash<int, QString> m_platformNameById;
  QHash<int, QString> m_missingBiosByPlatformId;

  // Drive disconnected and stuff like that
  QStringList m_unreachableRoots;

  QSet<int> m_knownContentDirectoryIds;

  void refreshRowFields();

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

  // Debounce for sync entry
  QTimer m_countsChangedTimer;
};
} // namespace firelight::library

// Handed to the proxy through the FilterFields role, so the predicate reads the row's cached
// record rather than rebuilding one
Q_DECLARE_METATYPE(const firelight::library::EntryFields *)
