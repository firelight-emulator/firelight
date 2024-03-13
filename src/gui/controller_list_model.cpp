//
// Created by alexs on 2/16/2024.
//

#include "controller_list_model.hpp"

namespace firelight::gui {
int ControllerListModel::rowCount(const QModelIndex &parent) const { return 0; }

QVariant ControllerListModel::data(const QModelIndex &index, int role) const {
  return QVariant{};
}
} // namespace firelight::gui