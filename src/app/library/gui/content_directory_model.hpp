#pragma once

#include "../../manager_accessor.hpp"
#include "../library_database.hpp"

#include <QAbstractListModel>

namespace firelight::gui {

class ContentDirectoryModel : public QAbstractListModel {
  Q_OBJECT

public:
  enum Roles {
    DirectoryId = Qt::UserRole + 1,
    Path,
  };

  explicit ContentDirectoryModel(library::IUserLibrary &library);

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

  Q_INVOKABLE void deleteItem(int id);
  Q_INVOKABLE void addItem(QString path);

private:
  library::IUserLibrary &m_library;
  std::vector<library::WatchedDirectory> m_items;
};
} // namespace firelight::gui
