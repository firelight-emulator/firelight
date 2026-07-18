#pragma once

#include "../../service_accessor.hpp"

#include <firelight/library/folder_info.hpp>

#include <QAbstractListModel>
#include <QVariant>

namespace firelight::gui {
class LibraryFolderListModel : public QAbstractListModel, public ServiceAccessor {
  Q_OBJECT

public:
  enum Roles {
    FolderId = Qt::UserRole + 1,
    DisplayName,
    Description,
    Icon1x1SourceUrl,
    FolderType, // 0 = manual, 1 = smart (library::FolderType)
    FilterJson, // smart-folder criteria JSON (empty for manual folders)
    Color,      // accent color (empty = default)
    SortRole,   // remembered game-sort role name (empty = view default)
    SortAscending,
    ParentId, // containing folder id, -1 = root
    Position  // manual order within the parent scope
  };

  explicit LibraryFolderListModel();

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  bool setData(const QModelIndex &index, const QVariant &value, int role) override;

  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

  Q_INVOKABLE bool addFolder(const QString &displayName);

  // Creates a manual folder under `parentId` (-1 = root). Returns the new
  // folder's id, or -1 on failure
  Q_INVOKABLE int createFolder(const QString &displayName, int parentId);

  // Creates a smart folder whose membership is computed live from the given
  // SmartFolderCriteria JSON (see smart_folder.hpp). Returns the new folder's
  // id, or -1 on failure (so the caller can then set color/sort on it)
  Q_INVOKABLE int addSmartFolder(const QString &displayName, const QString &filterJson);

  // Replaces a smart folder's criteria JSON
  Q_INVOKABLE bool updateSmartFolder(int folderId, const QString &filterJson);

  // Sets a folder's accent color (empty string clears it)
  Q_INVOKABLE bool setFolderColor(int folderId, const QString &color);

  // Renames a folder
  Q_INVOKABLE bool setFolderName(int folderId, const QString &displayName);

  // Remembers a folder's game sort (role name + direction). An empty role
  // clears the override so the view falls back to its default
  Q_INVOKABLE bool setFolderSort(int folderId, const QString &sortRole, bool ascending);

  // Manual folder ordering. reorderFolders sets the order of the folders in
  // orderedIds within a parent scope (parentId -1 = root); setFolderParent
  // moves a folder under a new parent (appended at the end). The UI for these
  // is a later step; the model API exists now
  Q_INVOKABLE bool reorderFolders(int parentId, const QVariantList &orderedIds);
  Q_INVOKABLE bool setFolderParent(int folderId, int newParentId);

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
