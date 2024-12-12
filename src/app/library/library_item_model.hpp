#pragma once

#include "firelight/library_database.hpp"
#include <QAbstractListModel>
#include <firelight/content_database.hpp>
#include <firelight/userdata_database.hpp>

namespace firelight::gui {

/**
 * @class LibraryItemModel
 * @brief A model for library items.
 *
 * This class provides a model for library items that can be used with Qt's
 * model-view framework.
 *
 * @see QAbstractListModel
 */
class LibraryItemModel final : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ getCount NOTIFY countChanged)

public:
  /**
   * @enum Roles
   * @brief The roles that can be used with this model.
   */
  enum Roles {
    DisplayName = Qt::UserRole + 1,
    EntryId,
    PlatformName,
    CreatedAt,
    Playlists,
    ContentPath,
    ParentGameName,
    LastPlayedAt
  };

  /**
   * @brief Constructs a new LibraryItemModel.
   * @param libraryDatabase The database to use for the model.
   * @param userdataDatabase The userdata database to use for recently played,
   * etc.
   */
  explicit LibraryItemModel(db::ILibraryDatabase *libraryDatabase,
                            db::IContentDatabase *contentDatabase,
                            db::IUserdataDatabase *userdataDatabase);

  /**
   * @brief Returns the number of rows in the model. Invoked by the view.
   * @param parent The parent index.
   * @return The number of rows.
   */
  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

  /**
   * @brief Returns the data for a given role and index. Invoked by the view.
   * @param index The index.
   * @param role The role.
   * @return The data.
   */
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;

  /**
   * @brief Returns the role names for the model. Invoked by the view.
   * @return The role names.
   */
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void updatePlaylistsForEntry(int entryId);
  Q_INVOKABLE bool isRomInLibrary(int romId) const;
  Q_INVOKABLE bool isModInLibrary(int modId) const;
  Q_INVOKABLE void addModToLibrary(int modId); // TODO: TEMPORARY
  Q_INVOKABLE void addPatchToLibrary(int patchId) const;

  [[nodiscard]] int getCount() const;

public slots:
  void refresh();

signals:
  void countChanged();

private:
  /**
   * @struct Item
   * @brief A struct representing an item in the model.
   */
  struct Item {
    int m_entryId;
    QString displayName;
    int platformId;
    QVector<int> m_playlists;
    unsigned int createdAt;
    QString contentPath;
    QString parentGameName;
    unsigned int lastPlayedAt;
  };

  db::ILibraryDatabase *m_libraryDatabase;
  db::IContentDatabase *m_contentDatabase;
  db::IUserdataDatabase *m_userdataDatabase;
  std::vector<Item> m_items;
};

} // namespace firelight::gui
