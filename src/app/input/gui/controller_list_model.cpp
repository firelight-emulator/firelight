#include "controller_list_model.hpp"

#include "../controller_icons.hpp"

#include <spdlog/spdlog.h>

namespace firelight::gui {
ControllerListModel::ControllerListModel(QObject *parent) : QAbstractListModel(parent) {
  m_inputService = getInputService();

  // These events are published on the SDL input thread; refreshControllerList
  // resets the model, so hop to the GUI thread first
  const auto refreshOnGuiThread = [this] {
    QMetaObject::invokeMethod(this, [this] { refreshControllerList(); }, Qt::QueuedConnection);
  };

  m_connectedHandler = EventDispatcher::instance().subscribe<input::GamepadConnectedEvent>(
      [refreshOnGuiThread](const input::GamepadConnectedEvent &) { refreshOnGuiThread(); });

  m_disconnectedHandler = EventDispatcher::instance().subscribe<input::GamepadDisconnectedEvent>(
      [refreshOnGuiThread](const input::GamepadDisconnectedEvent &) { refreshOnGuiThread(); });

  m_gamepadOrderChangedHandler = EventDispatcher::instance().subscribe<input::GamepadOrderChangedEvent>(
      [refreshOnGuiThread](const input::GamepadOrderChangedEvent &) { refreshOnGuiThread(); });

  refreshControllerList();
}

int ControllerListModel::rowCount(const QModelIndex &parent) const { return m_items.size(); }

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
  for (auto it = oldToNewIndex.constBegin(); it != oldToNewIndex.constEnd(); ++it) {
    map[it.value().toInt()] = it.key().toInt();
  }

  m_inputService->changeGamepadOrder(map);
}

void ControllerListModel::refreshControllerList() {
  emit beginResetModel();
  m_items.clear();

  for (int i = 0; i < 4; i++) {
    auto con = m_inputService->getPlayerGamepad(i);
    if (con) {
      spdlog::info(" Adding controller {}: {}", i, con->getName());
      m_items.push_back({i, true, con->getProfile()->getId(), QString::fromStdString(con->getName()), "None",
                         con->isWired(), ControllerIcons::sourceUrlFromType(con->getType())});
    } else {
      spdlog::info("Got no controller for {}", i);
      m_items.push_back({i, false, -1, "Default", "None", true});
    }
  }
  emit endResetModel();
}
} // namespace firelight::gui
