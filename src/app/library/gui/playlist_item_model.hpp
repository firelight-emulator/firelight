// TODO: NEEDS REVIEW
#pragma once

#include "../../service_accessor.hpp"

#include <firelight/library/folder_info.hpp>

#include <QAbstractListModel>
#include <QVariant>

namespace firelight::gui {
class LibraryFolderListModel : public QAbstractListModel, public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(QVariantList sortOptions READ getSortOptions CONSTANT)
  Q_PROPERTY(int count READ getCount NOTIFY countChanged)
  Q_PROPERTY(QVariantList manualFolders READ getManualFolders NOTIFY countChanged)

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

  /**
   * Moves count rows so they sit before destinationChild, and writes the new order back. Both ends
   * must be in one parent scope, which is what a position is numbered within
   */
  bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent,
                int destinationChild) override;

  /**
   * @return Every sort a collection may pin, from the one table the library view offers
   */
  [[nodiscard]] static QVariantList getSortOptions();

  /**
   * @return How many collections there are
   */
  [[nodiscard]] int getCount() const;

  /**
   * @return Each manual collection as its id and name
   */
  [[nodiscard]] QVariantList getManualFolders() const;

  /**
   * Every folder in a parent scope, in the order they are shown in
   */
  [[nodiscard]] Q_INVOKABLE QVariantList foldersInParent(int parentId) const;

  /**
   * @return Every field of one collection, or an empty map when there is no such collection
   */
  [[nodiscard]] Q_INVOKABLE QVariantMap folderById(int folderId) const;

  // TODO
  /**
   * @return Whether a collection already has exactly this name, once trimmed
   */
  [[nodiscard]] Q_INVOKABLE bool hasFolderNamed(const QString &displayName) const;

  /**
   * The cached row for a folder, or nullptr when there is none
   */
  [[nodiscard]] const library::FolderInfo *findFolder(int folderId) const;

  Q_INVOKABLE bool addFolder(const QString &displayName);

  Q_INVOKABLE int createFolder(const QString &displayName, int parentId);

  Q_INVOKABLE int addSmartFolder(const QString &displayName, const QString &filterJson);

  // TODO
  /**
   * Creates a collection from fields keyed by role name and returns its id, or -1 when none was created
   */
  Q_INVOKABLE int createCollection(const QVariantMap &fields);

  Q_INVOKABLE bool updateSmartFolder(int folderId, const QString &filterJson);

  // TODO: setData
  Q_INVOKABLE bool setFolderColor(int folderId, const QString &color);

  Q_INVOKABLE bool setFolderName(int folderId, const QString &displayName);

  Q_INVOKABLE bool setFolderSort(int folderId, const QString &sortRole, bool ascending);

  Q_INVOKABLE bool reorderFolders(int parentId, const QVariantList &orderedIds);
  Q_INVOKABLE bool setFolderParent(int folderId, int newParentId);

  Q_INVOKABLE void deleteFolder(int folderId);

signals:
  void folderDeleted(int folderId);

  void countChanged();

private:
  struct Item {
    int playlistId;
    QString displayName;
  };

  std::vector<library::FolderInfo> m_items;
};
} // namespace firelight::gui
