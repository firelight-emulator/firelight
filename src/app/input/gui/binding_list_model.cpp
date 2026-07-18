#include "binding_list_model.hpp"

#include <firelight/input/controller_repository.hpp>
#include <firelight/input/gamepad_input.hpp>

namespace firelight::input {

BindingListModel::BindingListModel(QObject *parent) : QAbstractListModel(parent) {}

int BindingListModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_bindings.size());
}

QVariant BindingListModel::data(const QModelIndex &index, int role) const {
  if (index.row() < 0 || index.row() >= static_cast<int>(m_bindings.size())) {
    return {};
  }
  const auto &binding = m_bindings.at(index.row());
  switch (role) {
  case SourceLabel:
    return QString::fromStdString(firelight::input::displayName(static_cast<GamepadInput>(binding.source.code)));
  case SourceCode:
    return binding.source.code;
  case Toggle:
    return binding.toggle;
  case TurboEnabled:
    return binding.turbo.enabled;
  case TurboRate:
    return static_cast<double>(binding.turbo.rateHz);
  default:
    return {};
  }
}

QHash<int, QByteArray> BindingListModel::roleNames() const {
  return {
      {SourceLabel, "sourceLabel"},   {SourceCode, "sourceCode"}, {Toggle, "toggle"},
      {TurboEnabled, "turboEnabled"}, {TurboRate, "turboRate"},
  };
}

int BindingListModel::profileId() const { return m_profileId; }

void BindingListModel::setProfileId(const int id) {
  if (id == m_profileId) {
    return;
  }
  m_profileId = id;
  reload();
  emit contextChanged();
}

int BindingListModel::platformId() const { return m_platformId; }

void BindingListModel::setPlatformId(const int id) {
  if (id == m_platformId) {
    return;
  }
  m_platformId = id;
  reload();
  emit contextChanged();
}

int BindingListModel::controllerTypeId() const { return m_controllerTypeId; }

void BindingListModel::setControllerTypeId(const int id) {
  if (id == m_controllerTypeId) {
    return;
  }
  m_controllerTypeId = id;
  reload();
  emit contextChanged();
}

int BindingListModel::targetInput() const { return m_targetInput; }

void BindingListModel::setTargetInput(const int input) {
  if (input == m_targetInput) {
    return;
  }
  m_targetInput = input;
  reload();
  emit contextChanged();
}

std::shared_ptr<InputMapping> BindingListModel::currentMapping() const {
  if (!m_profile || m_platformId < 0 || m_controllerTypeId < 0) {
    return nullptr;
  }
  return m_profile->getMappingForPlatformAndController(m_platformId, m_controllerTypeId);
}

void BindingListModel::reload() {
  beginResetModel();
  m_bindings.clear();

  const auto service = getControllerProfileRepository();
  if (service && m_profileId >= 0) {
    m_profile = service->getProfile(m_profileId);
  } else {
    m_profile = nullptr;
  }

  if (const auto mapping = currentMapping(); mapping && m_targetInput >= 0) {
    m_bindings = mapping->getBindings(static_cast<GamepadInput>(m_targetInput));
  }
  endResetModel();
}

void BindingListModel::commit() {
  const auto mapping = currentMapping();
  if (!mapping || m_targetInput < 0) {
    return;
  }
  mapping->setBindings(static_cast<GamepadInput>(m_targetInput), m_bindings);
  mapping->sync();
}

void BindingListModel::addBinding(const int sourceCode) {
  Binding binding;
  binding.source.type = SourceType::Button;
  binding.source.code = sourceCode;

  beginInsertRows(QModelIndex(), static_cast<int>(m_bindings.size()), static_cast<int>(m_bindings.size()));
  m_bindings.push_back(binding);
  endInsertRows();
  commit();
}

void BindingListModel::removeBinding(const int index) {
  if (index < 0 || index >= static_cast<int>(m_bindings.size())) {
    return;
  }
  beginRemoveRows(QModelIndex(), index, index);
  m_bindings.erase(m_bindings.begin() + index);
  endRemoveRows();
  commit();
}

void BindingListModel::setToggle(const int index, const bool on) {
  if (index < 0 || index >= static_cast<int>(m_bindings.size())) {
    return;
  }
  m_bindings[index].toggle = on;
  commit();
  emit dataChanged(this->index(index), this->index(index), {Toggle});
}

void BindingListModel::setTurboEnabled(const int index, const bool on) {
  if (index < 0 || index >= static_cast<int>(m_bindings.size())) {
    return;
  }
  m_bindings[index].turbo.enabled = on;
  commit();
  emit dataChanged(this->index(index), this->index(index), {TurboEnabled});
}

void BindingListModel::setTurboRate(const int index, const double hz) {
  if (index < 0 || index >= static_cast<int>(m_bindings.size())) {
    return;
  }
  m_bindings[index].turbo.rateHz = static_cast<float>(hz);
  commit();
  emit dataChanged(this->index(index), this->index(index), {TurboRate});
}

} // namespace firelight::input
