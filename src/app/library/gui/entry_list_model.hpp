#pragma once
#include "../entry.hpp"
#include "../user_library.hpp"
#include <QAbstractListModel>

namespace firelight::library {
class EntryListModel : public QAbstractListModel {
  Q_OBJECT

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
    CreatedAt
  };

  explicit EntryListModel(IUserLibrary &userLibrary, QObject *parent = nullptr);

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;

  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;

  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

  Q_INVOKABLE void addEntryToFolder(int entryId, int folderId);

  Q_INVOKABLE void removeEntryFromFolder(int entryId, int folderId);

public slots:
  void reset();
  void removeFolderId(int folderId);

private:
  IUserLibrary &m_userLibrary;
  QList<Entry> m_items{};
};
} // namespace firelight::library
