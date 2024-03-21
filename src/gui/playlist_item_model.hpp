#pragma once

#include "firelight/library_database.hpp"
#include <QAbstractListModel>

namespace firelight::gui {

class PlaylistItemModel final : public QAbstractListModel {
  Q_OBJECT

public:
  enum Roles {
    PlaylistId = Qt::UserRole + 1,
    DisplayName,
  };

  explicit PlaylistItemModel(db::ILibraryDatabase *libraryDatabase);

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

  Q_INVOKABLE void addPlaylist(const QString &displayName);
  Q_INVOKABLE void removePlaylist(int playlistId);
  Q_INVOKABLE void renamePlaylist(int playlistId, const QString &newName);
  Q_INVOKABLE void addEntryToPlaylist(int playlistId, int entryId);

private:
  struct Item {
    int playlistId;
    QString displayName;
  };

  db::ILibraryDatabase *m_libraryDatabase;
  std::vector<Item> m_items;
};
} // namespace firelight::gui
