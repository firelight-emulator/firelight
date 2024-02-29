//
// Created by alexs on 2/28/2024.
//

#ifndef LIBRARY_ITEM_MODEL_HPP
#define LIBRARY_ITEM_MODEL_HPP
#include "src/app/db/library_database.hpp"

#include <QAbstractListModel>

namespace Firelight {

class LibraryItemModel final : public QAbstractListModel {
  Q_OBJECT

public:
  enum Roles {
    DisplayName = Qt::UserRole + 1,
    EntryId,
    PlatformName,
    Playlists
  };

  explicit LibraryItemModel(LibraryDatabase *libraryDatabase);

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
  struct Item {
    int m_entryId;
    QString displayName;
    QVector<int> m_playlists;
  };
  LibraryDatabase *m_libraryDatabase;
  std::vector<Item> m_items;
};

} // namespace Firelight

#endif // LIBRARY_ITEM_MODEL_HPP
