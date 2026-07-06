#pragma once
#include <firelight/library/entry.hpp>
#include <firelight/library/smart_folder.hpp>
#include <firelight/library/user_library_service.hpp>
#include <QAbstractListModel>
#include <firelight/event_dispatcher.hpp>
#include <unordered_map>

namespace firelight::activity {
  class IActivityLog;
}
namespace firelight::platforms {
  class IPlatformService;
}

namespace firelight::library {
  class EntryListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ getCount NOTIFY countChanged)
    Q_PROPERTY(int numFavorites READ numFavorites NOTIFY numFavoritesChanged)
    Q_PROPERTY(QVariantMap countByPlatform READ getCountByPlatform NOTIFY countChanged)
    Q_PROPERTY(QVariantMap countByFolderId READ getCountByFolderId NOTIFY countByFolderIdChanged)

  public:
    /**
     * @enum Roles
     * @brief The roles that can be used with this model.
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
      NumSecondsPlayed
    };

    struct Item {
      Entry entry;
      uint64_t numSecondsPlayed{};
      uint64_t lastPlayedEpochMillis{};
    };

    EntryListModel(UserLibraryService &userLibrary,
                   activity::IActivityLog &activityLog,
                   platforms::IPlatformService &platformService,
                   QObject *parent = nullptr);

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

    [[nodiscard]] QVariant data(const QModelIndex &index,
                                int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role) override;

    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

    Q_INVOKABLE void addEntryToFolder(int entryId, int folderId);

    Q_INVOKABLE void removeEntryFromFolder(int entryId, int folderId);

    // True if the entry satisfies the given smart folder's criteria. Used by
    // the client-side folder filter for smart folders (manual folders use
    // folderIds membership). Parsed criteria are cached per folder; call
    // invalidateSmartFolderCache() after a smart folder's criteria change.
    Q_INVOKABLE bool matchesSmartFolder(int folderId, int entryId);

    // Drops cached smart-folder criteria so the next matchesSmartFolder /
    // count reflects edited criteria.
    Q_INVOKABLE void invalidateSmartFolderCache();

    int getCount() const;

    int numFavorites() const;

    QVariantMap getCountByPlatform() const;

    QVariantMap getCountByFolderId() const;

  public slots:
    void reset();

    void removeFolderId(int folderId);

  signals:
    void countChanged();

    void numFavoritesChanged();

    void countByFolderIdChanged();

  private:
    UserLibraryService &m_userLibrary;
    activity::IActivityLog &m_activityLog;
    platforms::IPlatformService &m_platformService;
    QList<Item> m_items{};

    // Flattens an item (entry attributes + joined play stats) into the Qt-free
    // struct the smart-folder evaluator consumes.
    [[nodiscard]] static EntryFields buildEntryFields(const Item &item);

    // Resolves (and memoizes) a smart folder's parsed criteria by id.
    const SmartFolderCriteria &criteriaForFolder(int folderId) const;
    mutable std::unordered_map<int, SmartFolderCriteria> m_smartFolderCache;

    // entry id -> index into m_items, rebuilt on reset(); lets the per-entry
    // matchesSmartFolder lookup avoid an O(n) scan (so a filter pass is O(n),
    // not O(n^2)).
    std::unordered_map<int, int> m_indexByEntryId;

    ScopedConnection m_gamePlayedConnection;
  };
} // namespace firelight::library
