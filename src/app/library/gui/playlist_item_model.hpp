#pragma once

#include "../../manager_accessor.hpp"
#include "../library_database.hpp"

#include <QAbstractListModel>

namespace firelight::gui {

class LibraryFolderListModel : public QAbstractListModel,
                               public ManagerAccessor {
  Q_OBJECT

public:
  enum Roles {
    PlaylistId = Qt::UserRole + 1,
    DisplayName,
  };

  explicit LibraryFolderListModel();

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

  Q_INVOKABLE bool addFolder(const QString &displayName);

  Q_INVOKABLE void deleteFolder(int folderId);
  // Q_INVOKABLE void renamePlaylist(int playlistId, const QString &newName);

private:
  struct Item {
    int playlistId;
    QString displayName;
  };

  std::vector<library::FolderInfo> m_items;
};
} // namespace firelight::gui
