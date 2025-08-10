#include "controller_list_model.hpp"

#include "../controller_icons.hpp"

#include <spdlog/spdlog.h>

namespace firelight::gui {
ControllerListModel::ControllerListModel(QObject *parent)
    : QAbstractListModel(parent) {
  m_inputService = input::InputService::instance();

  m_connectedHandler =
      EventDispatcher::instance().subscribe<input::GamepadConnectedEvent>(
          [this](const input::GamepadConnectedEvent &event) {
            this->onGamepadConnected(event);
          });

  m_disconnectedHandler =
      EventDispatcher::instance().subscribe<input::GamepadDisconnectedEvent>(
          [this](const input::GamepadDisconnectedEvent &event) {
            this->onGamepadDisconnected(event);
          });

  m_gamepadOrderChangedHandler =
      EventDispatcher::instance().subscribe<input::GamepadOrderChangedEvent>(
          [this](const input::GamepadOrderChangedEvent &) {
            this->refreshControllerList();
          });

  refreshControllerList();
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
  case Connected:
    return item.connected;
  case ProfileId:
    return item.profileId;
  case ModelName:
    return item.modelName;
  case Wired:
    return item.wired;
  case ImageUrl:
    return item.imageUrl;
  default:
    return QVariant{};
  }
}

QHash<int, QByteArray> ControllerListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[PlayerIndex] = "player_index";
  roles[Connected] = "connected";
  roles[ProfileId] = "profile_id";
  roles[ModelName] = "model_name";
  roles[Wired] = "wired";
  roles[ImageUrl] = "image_url";
  return roles;
}

void ControllerListModel::changeGamepadOrder(const QVariantMap &oldToNewIndex) {
  std::map<int, int> map;
  for (auto it = oldToNewIndex.constBegin(); it != oldToNewIndex.constEnd();
       ++it) {
    map[it.value().toInt()] = it.key().toInt();
  }

  m_inputService->changeGamepadOrder(map);
}

void ControllerListModel::onGamepadConnected(
    const input::GamepadConnectedEvent &event) {
  const auto gamepad = event.gamepad;
  const auto playerIndex = gamepad->getPlayerIndex();
  if (playerIndex > 3 || playerIndex < 0) {
    return;
  }
  m_items[playerIndex] = {
      playerIndex,
      true,
      gamepad->getProfile()->getId(),
      QString::fromStdString(gamepad->getName()),
      "None",
      gamepad->isWired(),
      ControllerIcons::sourceUrlFromType(gamepad->getType())};
  emit dataChanged(createIndex(playerIndex, 0), createIndex(playerIndex, 0),
                   {});
}

void ControllerListModel::onGamepadDisconnected(
    const input::GamepadDisconnectedEvent &event) {
  const auto gamepad = event.gamepad;
  const auto playerIndex = gamepad->getPlayerIndex();
  if (playerIndex > 3 || playerIndex < 0) {
    return;
  }

  m_items[playerIndex] = {playerIndex, false, -1, "Default", "None", true};
  emit dataChanged(createIndex(playerIndex, 0), createIndex(playerIndex, 0),
                   {});
}

void ControllerListModel::refreshControllerList() {
  emit beginResetModel();
  m_items.clear();
  for (int i = 0; i < 4; i++) {
    auto con = m_inputService->getPlayerGamepad(i);
    if (con) {
      m_items.push_back({i, true, con->getProfile()->getId(),
                         QString::fromStdString(con->getName()), "None",
                         con->isWired(),
                         ControllerIcons::sourceUrlFromType(con->getType())});
    } else {
      m_items.push_back({i, false, -1, "Default", "None", true});
    }
  }
  emit endResetModel();
}
} // namespace firelight::gui
