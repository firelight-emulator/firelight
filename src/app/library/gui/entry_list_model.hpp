#pragma once
#include <firelight/library/entry.hpp>
#include <firelight/library/user_library_service.hpp>
#include <QAbstractListModel>
#include <firelight/event_dispatcher.hpp>
#include <manager_accessor.hpp>

namespace firelight::library {
  class EntryListModel : public QAbstractListModel, public ManagerAccessor {
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
      CreatedAt,
      LastPlayedAt,
      NumSecondsPlayed
    };

    struct Item {
      Entry entry;
      uint64_t numSecondsPlayed{};
      uint64_t lastPlayedEpochMillis{};
    };

    explicit EntryListModel(UserLibraryService &userLibrary, QObject *parent = nullptr);

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

    [[nodiscard]] QVariant data(const QModelIndex &index,
                                int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role) override;

    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

    Q_INVOKABLE void addEntryToFolder(int entryId, int folderId);

    Q_INVOKABLE void removeEntryFromFolder(int entryId, int folderId);

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
    QList<Item> m_items{};

    ScopedConnection m_gamePlayedConnection;
  };
} // namespace firelight::library
