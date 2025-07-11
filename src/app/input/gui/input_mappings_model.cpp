#include "input_mappings_model.hpp"

#include <platform_metadata.hpp>

namespace firelight::input {
InputMappingsModel::InputMappingsModel(QObject *parent)
    : QAbstractListModel(parent) {
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
  return roles;
}

int InputMappingsModel::getProfileId() const { return m_profileId; }
void InputMappingsModel::setProfileId(const int profileId) {
  if (profileId == m_profileId) {
    return;
  }

  m_profileId = profileId;
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
void InputMappingsModel::refreshMappings() {
  if (m_profileId == -1 || m_platformId == -1) {
    return;
  }

  auto inputs = PlatformMetadata::getInputsForPlatform(m_platformId);

  const auto mapping =
      getControllerRepository()->getInputMapping(m_profileId, m_platformId);
  if (!mapping) {
    return;
  }

  beginResetModel();
  m_items.clear();
  for (const auto &input : inputs) {
    Item item;
    item.originalInput = input.input;
    item.originalInputName = QString::fromStdString(input.displayName);

    auto mapped = mapping->getMappedInput(input.input);
    if (mapped.has_value()) {
      item.isDefault = false;
      item.mappedInput = *mapped;
      item.mappedInputName =
          QString::fromStdString(PlatformMetadata::getInputName(*mapped));
    } else {
      item.isDefault = true;
      item.mappedInput = input.input;
      item.mappedInputName =
          QString::fromStdString(PlatformMetadata::getInputName(input.input));
    }
    m_items.append(item);
  }
  for (auto &item : m_items) {
    item.hasConflict = false;
    item.conflictingInputNames.clear();
    for (const auto &otherItem : m_items) {
      if (&item == &otherItem) {
        continue;
      }
      if (item.mappedInput == otherItem.mappedInput) {
        spdlog::info("Conflict found: {} <-> {}",
                     item.originalInputName.toStdString(),
                     otherItem.originalInputName.toStdString());
        item.hasConflict = true;
        item.conflictingInputNames.append(otherItem.originalInputName);
      }
    }
  }
  endResetModel();
}
} // namespace firelight::input