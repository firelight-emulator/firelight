//
// Created by alexs on 2/28/2024.
//

#ifndef LIBRARY_ITEM_MODEL_HPP
#define LIBRARY_ITEM_MODEL_HPP
#include <QAbstractListModel>

namespace Firelight {

class LibraryItemModel final : public QAbstractListModel {
  Q_OBJECT

public:
  LibraryItemModel();

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
  enum Roles { DisplayName = Qt::UserRole + 1, EntryId, PlatformName };
  std::vector<QString> m_items;
};

} // namespace Firelight

#endif // LIBRARY_ITEM_MODEL_HPP
