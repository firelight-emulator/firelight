// TODO: NEEDS REVIEW
#include "profile_list_model.hpp"

#include "../profile_events.hpp"

#include <firelight/input/controller_repository.hpp>
#include <firelight/input/input_service.hpp>

#include <QFile>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace firelight::input {

ProfileListModel::ProfileListModel(QObject *parent) : QAbstractListModel(parent) {
  auto &dispatcher = EventDispatcher::instance();

  const auto refreshSlotsOnGuiThread = [this] {
    QMetaObject::invokeMethod(this, [this] { refreshPlayerSlots(); }, Qt::QueuedConnection);
  };

  m_connectedHandler = dispatcher.subscribe<GamepadConnectedEvent>([this](const GamepadConnectedEvent &) {
    QMetaObject::invokeMethod(this, [this] { refresh(); }, Qt::QueuedConnection);
  });
  m_disconnectedHandler = dispatcher.subscribe<GamepadDisconnectedEvent>(
      [refreshSlotsOnGuiThread](const GamepadDisconnectedEvent &) { refreshSlotsOnGuiThread(); });
  m_orderChangedHandler = dispatcher.subscribe<GamepadOrderChangedEvent>(
      [refreshSlotsOnGuiThread](const GamepadOrderChangedEvent &) { refreshSlotsOnGuiThread(); });
  m_settingsChangedHandler =
      dispatcher.subscribe<ProfileSettingsChangedEvent>([this](const ProfileSettingsChangedEvent &event) {
        const auto profileId = event.profileId;
        QMetaObject::invokeMethod(this, [this, profileId] { reloadProfile(profileId); }, Qt::QueuedConnection);
      });

  refresh();
}

int ProfileListModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_items.size());
}

QVariant ProfileListModel::data(const QModelIndex &index, int role) const {
  if (index.row() < 0 || index.row() >= m_items.size()) {
    return {};
  }
  const auto &item = m_items.at(index.row());

  if (const auto field = getAnalogFieldForRole(role)) {
    return getAnalogField(item.analog, *field);
  }

  switch (role) {
  case ProfileId:
    return item.id;
  case Name:
    return item.name;
  case Icon:
    return item.icon;
  case IsKeyboard:
    return item.isKeyboard;
  case PlayerSlots:
    return QVariant::fromValue(item.playerSlots);
  default:
    return {};
  }
}

bool ProfileListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (index.row() < 0 || index.row() >= m_items.size()) {
    return false;
  }
  const auto repository = getControllerProfileRepository();
  if (!repository) {
    return false;
  }
  auto &item = m_items[index.row()];
  const auto profileId = item.id;

  if (const auto field = getAnalogFieldForRole(role)) {
    auto updated = item.analog;
    if (!setAnalogField(updated, *field, value)) {
      return false;
    }

    repository->setProfileAnalogSettings(profileId, updated);
    item.analog = updated;
    emit dataChanged(index, index, {role});
    EventDispatcher::instance().publish(ProfileSettingsChangedEvent{profileId});
    return true;
  }

  if (role != Name) {
    return false;
  }

  const auto name = value.toString();
  if (name.isEmpty() || name == item.name) {
    return false;
  }

  if (!repository->renameProfile(profileId, name.toStdString())) {
    return false;
  }

  item.name = name;
  emit dataChanged(index, index, {Name});
  EventDispatcher::instance().publish(ProfileSettingsChangedEvent{profileId});
  return true;
}

Qt::ItemFlags ProfileListModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) {
    return QAbstractListModel::flags(index);
  }

  return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

QHash<int, QByteArray> ProfileListModel::roleNames() const {
  QHash<int, QByteArray> roles = {
      {ProfileId, "profileId"},     {Name, "name"}, {Icon, "icon"}, {IsKeyboard, "isKeyboard"},
      {PlayerSlots, "playerSlots"},
  };
  for (std::size_t i = 0; i < ANALOG_FIELDS.size(); ++i) {
    roles[AnalogRoleBase + static_cast<int>(i)] = getAnalogFieldName(ANALOG_FIELDS[i]);
  }

  return roles;
}

void ProfileListModel::refresh() {
  const auto repository = getControllerProfileRepository();
  if (!repository) {
    return;
  }

  beginResetModel();
  m_items.clear();
  for (const auto &profile : repository->listProfiles()) {
    if (!profile) {
      continue;
    }
    m_items.append(readItem(*profile));
  }
  endResetModel();
}

int ProfileListModel::createProfile(const QString &name) {
  const auto repository = getControllerProfileRepository();
  if (!repository) {
    return -1;
  }
  const auto profile = repository->createProfile(name.toStdString());
  refresh();
  return profile ? profile->getId() : -1;
}

int ProfileListModel::cloneProfile(const int sourceId, const QString &name) {
  const auto repository = getControllerProfileRepository();
  if (!repository) {
    return -1;
  }
  const auto profile = repository->cloneProfile(sourceId, name.toStdString());
  refresh();
  return profile ? profile->getId() : -1;
}

bool ProfileListModel::renameProfile(const int id, const QString &name) {
  const auto repository = getControllerProfileRepository();
  if (!repository) {
    return false;
  }
  const auto ok = repository->renameProfile(id, name.toStdString());
  refresh();
  if (ok) {
    EventDispatcher::instance().publish(ProfileSettingsChangedEvent{id});
  }
  return ok;
}

bool ProfileListModel::deleteProfile(const int id) {
  const auto repository = getControllerProfileRepository();
  if (!repository) {
    return false;
  }
  const auto ok = repository->deleteProfile(id);
  refresh();
  if (ok) {
    EventDispatcher::instance().publish(ProfileSettingsChangedEvent{id});
  }
  return ok;
}

bool ProfileListModel::exportProfile(const int id, const QUrl &fileUrl) {
  const auto repository = getControllerProfileRepository();
  if (!repository) {
    return false;
  }
  const auto json = repository->exportProfile(id);
  if (json.empty()) {
    return false;
  }

  const auto path = fileUrl.isLocalFile() ? fileUrl.toLocalFile() : fileUrl.toString();
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    spdlog::error("Failed to open {} for profile export", path.toStdString());
    return false;
  }
  file.write(QByteArray::fromStdString(json));
  return true;
}

int ProfileListModel::importProfile(const QUrl &fileUrl) {
  const auto repository = getControllerProfileRepository();
  if (!repository) {
    return -1;
  }

  const auto path = fileUrl.isLocalFile() ? fileUrl.toLocalFile() : fileUrl.toString();
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    spdlog::error("Failed to open {} for profile import", path.toStdString());
    return -1;
  }
  const auto json = file.readAll().toStdString();
  const auto profile = repository->importProfile(json);
  refresh();
  return profile ? profile->getId() : -1;
}

QVariantList ProfileListModel::getAllProfiles() {
  QVariantList list;
  for (const auto &item : m_items) {
    QVariantMap map;
    map["id"] = item.id;
    map["name"] = item.name;
    map["icon"] = item.icon;
    map["isKeyboard"] = item.isKeyboard;
    map["playerSlots"] = QVariant::fromValue(item.playerSlots);

    list.append(map);
  }

  return list;
}

std::optional<AnalogField> ProfileListModel::getAnalogFieldForRole(const int role) {
  const auto offset = role - AnalogRoleBase;
  if (offset < 0 || offset >= static_cast<int>(ANALOG_FIELDS.size())) {
    return std::nullopt;
  }

  return ANALOG_FIELDS[offset];
}

ProfileListModel::Item ProfileListModel::readItem(const GamepadProfile &profile) const {
  return Item{
      .id = profile.getId(),
      .name = QString::fromStdString(profile.getName()),
      .icon = QString::fromStdString(profile.getIcon()),
      .isKeyboard = profile.isKeyboardProfile(),
      .analog = profile.getDefaultAnalogSettings(),
      .playerSlots = getPlayerSlotsFor(profile.getId()),
  };
}

QList<int> ProfileListModel::getPlayerSlotsFor(const int profileId) const {
  const auto inputService = getInputService();
  if (!inputService) {
    return {};
  }

  QList<int> occupied;
  for (const auto &device : inputService->listGamepads()) {
    if (!device || device->getPlayerIndex() < 0) {
      continue;
    }

    const auto profile = device->getProfile();
    if (profile && profile->getId() == profileId) {
      occupied.append(device->getPlayerIndex());
    }
  }
  std::sort(occupied.begin(), occupied.end());

  return occupied;
}

void ProfileListModel::refreshPlayerSlots() {
  for (int row = 0; row < m_items.size(); ++row) {
    auto occupied = getPlayerSlotsFor(m_items[row].id);
    if (occupied == m_items[row].playerSlots) {
      continue;
    }

    m_items[row].playerSlots = std::move(occupied);
    emit dataChanged(index(row), index(row), {PlayerSlots});
  }
}

void ProfileListModel::reloadProfile(const int profileId) {
  const auto row = getRowOf(profileId);
  const auto repository = getControllerProfileRepository();
  if (row < 0 || !repository) {
    return;
  }

  const auto profile = repository->getProfile(profileId);
  if (!profile) {
    refresh();
    return;
  }

  const auto fresh = readItem(*profile);
  auto &item = m_items[row];
  QList<int> changedRoles;
  if (fresh.name != item.name) {
    changedRoles.append(Name);
  }
  if (fresh.icon != item.icon) {
    changedRoles.append(Icon);
  }
  if (!(fresh.analog == item.analog)) {
    for (std::size_t i = 0; i < ANALOG_FIELDS.size(); ++i) {
      changedRoles.append(AnalogRoleBase + static_cast<int>(i));
    }
  }
  if (changedRoles.isEmpty()) {
    return;
  }

  item.name = fresh.name;
  item.icon = fresh.icon;
  item.analog = fresh.analog;
  emit dataChanged(index(row), index(row), changedRoles);
}

int ProfileListModel::getRowOf(const int profileId) const {
  for (int row = 0; row < m_items.size(); ++row) {
    if (m_items[row].id == profileId) {
      return row;
    }
  }

  return -1;
}

} // namespace firelight::input
