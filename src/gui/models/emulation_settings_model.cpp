#include "emulation_settings_model.hpp"

#include <spdlog/spdlog.h>

namespace firelight::settings {
EmulationSettingsModel::EmulationSettingsModel(QObject *parent)
    : QAbstractListModel(parent) {
  m_items.emplace_back(Item{
      .label = "Picture mode",
      .key = "picture-mode",
      .section = "Video",
      .description = "Determines how the game image fills the screen.",
      .type = "combobox",
      .options = {
          QVariantHash{{"label", "Aspect Ratio Fill"},
                       {"value", "aspect-ratio-fill"}},
          QVariantHash{{"label", "Integer Scale"}, {"value", "integer-scale"}},
          QVariantHash{{"label", "Stretch"}, {"value", "stretch"}}}});

  m_items.emplace_back(Item{
      .label = "Aspect ratio",
      .key = "aspect-ratio",
      .section = "Video",
      .description =
          "Some platforms rendered at a different resolution than the one at "
          "which they were displayed on CRTs. This option lets you control "
          "which aspect ratio is used.\n\nPixel-perfect: Uses square pixels; "
          "ignores the emulator's preferred aspect ratio\nEmulator-corrected: "
          "Uses the aspect ratio preferred by the emulator",
      .type = "combobox",
      .options = {
          QVariantHash{{"label", "Pixel-perfect"}, {"value", "pixel-perfect"}},
          QVariantHash{{"label", "Emulator-corrected"},
                       {"value", "emulator-corrected"}}}});

  m_items.emplace_back(Item{.label = "Enable rewind",
                            .key = "rewind-enabled",
                            .section = "Rewind",
                            .description =
                                "Note: Rewind is always disabled when using "
                                "RetroAchievements in Hardcore mode.",
                            .type = "toggle"});
}

int EmulationSettingsModel::getPlatformId() const { return m_platformId; }

void EmulationSettingsModel::setPlatformId(const int platformId) {
  if (platformId == m_platformId) {
    return;
  }

  // TODO: do stuff
  m_platformId = platformId;
  emit platformIdChanged();
}

int EmulationSettingsModel::getLevel() const { return m_level; }

void EmulationSettingsModel::setLevel(int level) {
  if (level == static_cast<int>(m_level)) {
    return;
  }

  m_level = static_cast<SettingsLevel>(level);
  emit levelChanged();
}

int EmulationSettingsModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}
QVariant EmulationSettingsModel::data(const QModelIndex &index,
                                      int role) const {
  if (!index.isValid() || index.row() >= m_items.size()) {
    return {};
  }

  const auto &item = m_items.at(index.row());
  switch (role) {
  case LabelRole:
    return item.label;
  case KeyRole:
    return item.key;
  case DescriptionRole:
    return item.description;
  case SectionRole:
    return item.section;
  case TypeRole:
    return item.type;
  case ValueRole: {
    if (item.type == "combobox") {
      return item.stringValue;
    }
    if (item.type == "toggle") {
      return item.boolValue;
    }

    return {};
  }
  case OptionsRole:
    return QVariant::fromValue(item.options);
  case RequiresRestartRole:
    return item.requiresRestart;
  default:
    return {};
  }
}
QHash<int, QByteArray> EmulationSettingsModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[LabelRole] = "label";
  roles[KeyRole] = "key";
  roles[DescriptionRole] = "description";
  roles[SectionRole] = "section";
  roles[TypeRole] = "type";
  roles[ValueRole] = "value";
  roles[OptionsRole] = "options";
  roles[RequiresRestartRole] = "requiresRestart";

  return roles;
}
Qt::ItemFlags EmulationSettingsModel::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}
bool EmulationSettingsModel::setData(const QModelIndex &index,
                                     const QVariant &value, const int role) {
  if (!index.isValid() || index.row() >= m_items.size()) {
    return false;
  }

  auto &item = m_items[index.row()];
  switch (role) {
  case ValueRole: {
    if (item.type == "combobox" && value.canConvert<QString>()) {
      spdlog::info("Setting combobox value to {}",
                   value.toString().toStdString());
      item.stringValue = value.toString();
      emit dataChanged(index, index, {ValueRole});
      return true;
    }
    if (item.type == "toggle" && value.canConvert<bool>()) {
      spdlog::info("Setting toggle value to {}", value.toBool());
      item.boolValue = value.toBool();
      emit dataChanged(index, index, {ValueRole});
      return true;
    }
  }
  }
}
} // namespace firelight::settings