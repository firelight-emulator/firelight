#pragma once
#include "firelight/userdata_database.hpp"
#include <QAbstractListModel>

namespace firelight::gui {

class SavefileListModel : public QAbstractListModel {
  Q_OBJECT
public:
  SavefileListModel(db::IUserdataDatabase &userdataDatabase);
  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;

private:
  /**
   * @struct Item
   * @brief A struct representing an item in the model.
   */
  struct Item {
    int slotNumber;
  };
  db::IUserdataDatabase &m_userdataDatabase;
  std::vector<Item> m_items;
};

} // namespace firelight::gui
