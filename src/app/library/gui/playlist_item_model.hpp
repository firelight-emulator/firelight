#pragma once

#include "../../service_accessor.hpp"

#include <firelight/library/folder_info.hpp>
#include <QAbstractListModel>

namespace firelight::gui {
  class LibraryFolderListModel : public QAbstractListModel,
                                 public ServiceAccessor {
    Q_OBJECT

  public:
    enum Roles {
      FolderId = Qt::UserRole + 1,
      DisplayName,
      Description,
      Icon1x1SourceUrl
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

  signals:
    void folderDeleted(int folderId);

  private:
    struct Item {
      int playlistId;
      QString displayName;
    };

    std::vector<library::FolderInfo> m_items;
  };
} // namespace firelight::gui
