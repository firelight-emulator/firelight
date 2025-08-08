
#include "input_mappings_model.hpp"

#include "platforms/models/platform.hpp"
#include "platforms/platform_service.hpp"

#include <platform_metadata.hpp>

namespace firelight::input {
InputMappingsModel::InputMappingsModel(QObject *parent)
    : QAbstractListModel(parent) {
  m_inputService = InputService::instance();
  refreshMappings();
}

int InputMappingsModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}

QVariant InputMappingsModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  const auto &item = m_items.at(index.row());

  switch (role) {
  case IsDefault:
    return item.isDefault;
  case OriginalInput:
    return item.originalInput;
  case OriginalInputName:
    return item.originalInputName;
  case MappedInput:
    return item.mappedInput;
  case MappedInputName:
    return item.mappedInputName;
  case HasConflict:
    return item.hasConflict;
  case ConflictingInputNames:
    return item.conflictingInputNames;
  case HasMapping:
    return item.mappedInput != libretro::IRetroPad::Input::Unknown;
  default:
    return QVariant{};
  }
}

QHash<int, QByteArray> InputMappingsModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IsDefault] = "isDefault";
  roles[OriginalInput] = "originalInput";
  roles[OriginalInputName] = "originalInputName";
  roles[MappedInput] = "mappedInput";
  roles[MappedInputName] = "mappedInputName";
  roles[HasConflict] = "hasConflict";
  roles[ConflictingInputNames] = "conflictingInputNames";
  roles[HasMapping] = "hasMapping";
  return roles;
}

int InputMappingsModel::getProfileId() const { return m_profileId; }
void InputMappingsModel::setProfileId(const int profileId) {
  if (profileId == m_profileId) {
    return;
  }

  m_profileId = profileId;
  m_currentProfile = m_inputService->getProfile(m_profileId);
  emit profileIdChanged();
  refreshMappings();
}

int InputMappingsModel::getPlatformId() const { return m_platformId; }
void InputMappingsModel::setPlatformId(const int platformId) {
  if (platformId == m_platformId) {
    return;
  }

  m_platformId = platformId;
  emit platformIdChanged();
  refreshMappings();
}

int InputMappingsModel::getControllerTypeId() const {
  return m_controllerTypeId;
}

void InputMappingsModel::setControllerTypeId(const int controllerTypeId) {
  if (controllerTypeId == m_controllerTypeId) {
    return;
  }

  m_controllerTypeId = controllerTypeId;
  emit controllerTypeIdChanged();
  refreshMappings();
}

bool InputMappingsModel::setData(const QModelIndex &index,
                                 const QVariant &value, const int role) {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return false;
  }
  auto &item = m_items[index.row()];
  switch (role) {
  case MappedInput:
    item.mappedInput = static_cast<GamepadInput>(value.toInt());
    break;
  default:
    return false;
  }

  m_inputMapping->addMapping(item.originalInput, item.mappedInput);
  m_inputMapping->sync();

  return true;
}

Qt::ItemFlags InputMappingsModel::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

void InputMappingsModel::setMapping(const int originalInput, int mappedInput) {
  for (auto i = 0; i < m_items.size(); ++i) {
    auto &item = m_items[i];
    if (item.originalInput != originalInput) {
      continue;
    }

    spdlog::info("Found item to update: {}",
                 item.originalInputName.toStdString());
    item.mappedInput = static_cast<GamepadInput>(mappedInput);
    item.mappedInputName = QString::fromStdString(
        PlatformMetadata::getInputName(item.mappedInput));

    spdlog::info("Updating item (index {}): {} -> {}", i,
                 item.originalInputName.toStdString(),
                 item.mappedInputName.toStdString());
    item.isDefault = item.originalInput == item.mappedInput;

    m_inputMapping->addMapping(item.originalInput, item.mappedInput);
    m_inputMapping->sync();

    checkForConflicts();
    dataChanged(index(i), index(i));
    return;
  }
}

void InputMappingsModel::resetToDefault(int originalInput) {
  if (!m_currentProfile || m_platformId == -1 || m_controllerTypeId == -1) {
    return;
  }

  auto platform =
      platforms::PlatformService::getInstance().getPlatform(m_platformId);
  if (!platform.has_value()) {
    spdlog::warn("Platform with ID {} not found", m_platformId);
    return;
  }

  std::vector<platforms::PlatformInputDescriptor> inputs;
  for (const auto &type : platform->controllerTypes) {
    if (type.id == m_controllerTypeId) {
      inputs = type.inputs;
      break;
    }
  }

  if (inputs.empty()) {
    spdlog::warn("No inputs found for controller type ID {}",
                 m_controllerTypeId);
    return;
  }

  for (auto i = 0; i < m_items.size(); ++i) {
    auto &item = m_items[i];
    if (item.originalInput == originalInput) {
      for (const auto &input : inputs) {
        if (input.virtualInput == originalInput) {
          item.mappedInput = input.virtualInput;
          item.mappedInputName = QString::fromStdString(
              PlatformMetadata::getInputName(input.virtualInput));
          item.isDefault = true;

          m_inputMapping->removeMapping(
              static_cast<GamepadInput>(originalInput));
          m_inputMapping->sync();

          checkForConflicts();
          emit dataChanged(index(i), index(i));
          return;
        }
      }
    }
  }
}
void InputMappingsModel::resetAllToDefault() {
  if (!m_currentProfile || m_platformId == -1 || m_controllerTypeId == -1) {
    return;
  }

  auto platform =
      platforms::PlatformService::getInstance().getPlatform(m_platformId);
  if (!platform.has_value()) {
    spdlog::warn("Platform with ID {} not found", m_platformId);
    return;
  }

  std::vector<platforms::PlatformInputDescriptor> inputs;
  for (const auto &type : platform->controllerTypes) {
    if (type.id == m_controllerTypeId) {
      inputs = type.inputs;
      break;
    }
  }

  if (inputs.empty()) {
    spdlog::warn("No inputs found for controller type ID {}",
                 m_controllerTypeId);
    return;
  }

  for (const auto &input : inputs) {
    resetToDefault(input.virtualInput);
  }
}

void InputMappingsModel::clearMapping(int originalInput) {
  for (auto i = 0; i < m_items.size(); ++i) {
    auto &item = m_items[i];
    if (item.originalInput != originalInput) {
      continue;
    }

    item.mappedInput = None;
    item.mappedInputName = "Not mapped";
    item.isDefault = false;

    m_inputMapping->addMapping(item.originalInput, item.mappedInput);
    m_inputMapping->sync();

    checkForConflicts();
    dataChanged(index(i), index(i));
    return;
  }
}

void InputMappingsModel::checkForConflicts() {
  for (auto i = 0; i < m_items.size(); ++i) {
    auto &item = m_items[i];

    item.hasConflict = false;
    item.conflictingInputNames.clear();
    dataChanged(index(i), index(i), {HasConflict, ConflictingInputNames});
    for (const auto &otherItem : m_items) {
      if (&item == &otherItem) {
        continue;
      }
      if (item.mappedInput == otherItem.mappedInput) {
        if (item.mappedInput == libretro::IRetroPad::Input::Unknown) {
          continue;
        }
        spdlog::info("Conflict found: {} <-> {}",
                     item.originalInputName.toStdString(),
                     otherItem.originalInputName.toStdString());
        item.hasConflict = true;
        item.conflictingInputNames.append(otherItem.originalInputName);
        dataChanged(index(i), index(i), {HasConflict, ConflictingInputNames});
      }
    }
  }
}

void InputMappingsModel::refreshMappings() {
  if (!m_currentProfile || m_platformId == -1 || m_controllerTypeId == -1) {
    return;
  }

  auto platform =
      platforms::PlatformService::getInstance().getPlatform(m_platformId);
  if (!platform.has_value()) {
    spdlog::warn("Platform with ID {} not found", m_platformId);
    return;
  }

  std::vector<platforms::PlatformInputDescriptor> inputs;
  for (const auto &type : platform->controllerTypes) {
    if (type.id == m_controllerTypeId) {
      inputs = type.inputs;
      break;
    }
  }

  if (inputs.empty()) {
    spdlog::warn("No inputs found for controller type ID {}",
                 m_controllerTypeId);
    return;
  }

  m_inputMapping = m_currentProfile->getMappingForPlatformAndController(
      m_platformId, m_controllerTypeId);
  if (!m_inputMapping) {
    return;
  }

  beginResetModel();
  m_items.clear();
  for (const auto &input : inputs) {
    Item item;
    item.originalInput = input.virtualInput;
    item.originalInputName = QString::fromStdString(input.name);

    auto mapped = m_inputMapping->getMappedInput(input.virtualInput);
    if (mapped.has_value()) {
      item.mappedInput = *mapped;
      item.mappedInputName =
          QString::fromStdString(PlatformMetadata::getInputName(*mapped));
      item.isDefault = item.originalInput == item.mappedInput;
    } else {
      item.isDefault = true;
      item.mappedInput = input.virtualInput;
      item.mappedInputName = QString::fromStdString(
          PlatformMetadata::getInputName(input.virtualInput));
    }
    m_items.append(item);
  }
  checkForConflicts();
  endResetModel();
}
} // namespace firelight::input