#include "emulator_settings_model.hpp"

namespace firelight::settings {
EmulatorSettingsModel::EmulatorSettingsModel(QObject *parent)
    : QAbstractListModel(parent) {
  m_items.emplace_back(Item{.name = "Toggle option",
                            .description = "A toggle option",
                            .category = "one",
                            .type = "toggle",
                            .boolValue = true});

  m_items.emplace_back(Item{
      .name = "Text option",
      .description = "A text option",
      .category = "one",
      .stringValue = "testing testing",
      .type = "text",
  });

  m_items.emplace_back(Item{
      .name = "Drop down option",
      .description = "A drop down option",
      .category = "two",
      .type = "dropdown",
  });
}

int EmulatorSettingsModel::platformId() const { return m_platformId; }

void EmulatorSettingsModel::setPlatformId(int platformId) {
  if (m_platformId == platformId) {
    return;
  }

  m_platformId = platformId;
  emit platformIdChanged();

  // TODO: Get options for platform
}

int EmulatorSettingsModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}

QVariant EmulatorSettingsModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() < 0 ||
      index.row() >= static_cast<int>(m_items.size())) {
    return {};
  }

  const auto &item = m_items[index.row()];
  switch (role) {
  case Name:
    return item.name;
  case Description:
    return item.description;
  case Category:
    return item.category;
  case Type:
    return item.type;
  case Value: {
    if (item.type == "toggle") {
      return item.boolValue;
    }

    if (item.type == "text") {
      return item.stringValue;
    }

    if (item.type == "dropdown") {
      return item.numberValue; // Assuming dropdowns use number values
    }
  }
  default:
    return {};
  }
}

QHash<int, QByteArray> EmulatorSettingsModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[Name] = "name";
  roles[Description] = "description";
  roles[Category] = "category";
  roles[Type] = "type";
  roles[Value] = "value";
  return roles;
}
} // namespace firelight::settings