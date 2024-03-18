#include "controller_list_model.hpp"

namespace firelight::gui {
ControllerListModel::ControllerListModel(
    Input::ControllerManager &controllerManager)
    : m_controllerManager(controllerManager) {
  refreshControllerList();

  connect(&m_controllerManager, &Input::ControllerManager::controllerConnected,
          this, &ControllerListModel::refreshControllerList);
  connect(&m_controllerManager,
          &Input::ControllerManager::controllerDisconnected, this,
          &ControllerListModel::refreshControllerList);
}

int ControllerListModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}

QVariant ControllerListModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  auto item = m_items.at(index.row());

  switch (role) {
  case PlayerIndex:
    return item.playerIndex;
  case ModelName:
    return item.modelName;
  default:
    return QVariant{};
  }
}

QHash<int, QByteArray> ControllerListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[PlayerIndex] = "player_index";
  roles[ModelName] = "model_name";
  return roles;
}

void ControllerListModel::refreshControllerList() {
  emit beginResetModel();
  m_items.clear();
  for (int i = 0; i < 4; i++) {
    auto con = m_controllerManager.getControllerForPlayer(i);
    if (con.has_value()) {
      beginInsertRows(QModelIndex{}, i, i);
      m_items.push_back(
          {i, QString::fromStdString(con.value()->getControllerName()),
           "None"});
      endInsertRows();
    }
  }
  emit endResetModel();
}
} // namespace firelight::gui