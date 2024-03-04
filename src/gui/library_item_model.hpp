#pragma once

#include "../app/db/library_database.hpp"
#include <QAbstractListModel>

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

public:
  /**
   * @enum Roles
   * @brief The roles that can be used with this model.
   */
  enum Roles {
    DisplayName = Qt::UserRole + 1,
    EntryId,
    PlatformName,
    Playlists
  };

  /**
   * @brief Constructs a new LibraryItemModel.
   * @param libraryDatabase The database to use for the model.
   */
  explicit LibraryItemModel(db::ILibraryDatabase *libraryDatabase);

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

private:
  /**
   * @struct Item
   * @brief A struct representing an item in the model.
   */
  struct Item {
    int m_entryId;
    QString displayName;
    QVector<int> m_playlists;
  };

  db::ILibraryDatabase *m_libraryDatabase;
  std::vector<Item> m_items;
};

} // namespace firelight::gui
