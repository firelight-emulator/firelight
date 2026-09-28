#include "controller_list_model.hpp"

#include "../controller_icons.hpp"

#include <QThread>
#include <algorithm>
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

  // A single move published on the GUI thread is applied as a row move; anything else reloads
  m_gamepadOrderChangedHandler = EventDispatcher::instance().subscribe<input::GamepadOrderChangedEvent>(
      [this, refreshOnGuiThread](const input::GamepadOrderChangedEvent &event) {
        if (event.from < 0 || event.to < 0 || QThread::currentThread() != thread()) {
          refreshOnGuiThread();
          return;
        }

        moveSlotRow(event.from, event.to);
      });

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

void ControllerListModel::moveSlotRow(const int from, const int to) {
  if (from == to) {
    return;
  }

  const auto count = static_cast<int>(m_items.size());
  if (from < 0 || to < 0 || from >= count || to >= count) {
    refreshControllerList();
    return;
  }

  if (!beginMoveRows(QModelIndex(), from, from, QModelIndex(), to > from ? to + 1 : to)) {
    refreshControllerList();
    return;
  }

  const auto moved = m_items[from];
  m_items.erase(m_items.begin() + from);
  m_items.insert(m_items.begin() + to, moved);

  const auto first = std::min(from, to);
  const auto last = std::max(from, to);
  for (auto row = first; row <= last; ++row) {
    m_items[row].playerIndex = row;
  }

  endMoveRows();
  emit dataChanged(index(first), index(last), {PlayerIndex});
}
} // namespace firelight::gui
