#include "emulation_settings_model.hpp"

#include <spdlog/spdlog.h>

namespace firelight::settings {
EmulationSettingsModel::EmulationSettingsModel(QObject *parent)
    : QAbstractListModel(parent) {

  m_platformSettingChangedConnection =
      EventDispatcher::instance().subscribe<PlatformSettingChangedEvent>(
          [this](const PlatformSettingChangedEvent &e) {
            if (m_level != Platform) {
              return;
            }

            if (e.platformId != m_platformId) {
              return;
            }

            for (int i = 0; i < m_items.size(); ++i) {
              if (m_items[i].key == QString::fromStdString(e.key)) {
                auto &item = m_items[i];
                setItemValue(i, item, e.value);
                break;
              }
            }
          });

  m_gameSettingChangedConnection =
      EventDispatcher::instance().subscribe<GameSettingChangedEvent>(
          [this](const GameSettingChangedEvent &e) {
            if (m_level != Game) {
              return;
            }

            if (m_contentHash.isEmpty() ||
                m_contentHash.toStdString() != e.contentHash) {
              return;
            }

            for (int i = 0; i < m_items.size(); ++i) {
              if (m_items[i].key == QString::fromStdString(e.key)) {
                auto &item = m_items[i];
                setItemValue(i, item, e.value);
                break;
              }
            }
          });

  m_items.emplace_back(Item{
      .label = "Picture mode",
      .key = "picture-mode",
      .section = "Video",
      .description = "Determines how the game image fills the screen.",
      .type = "combobox",
      .defaultValue = "aspect-ratio-fill",
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
      .defaultValue = "emulator-corrected",
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
                            .type = "toggle",
                            .defaultValue = "true"});
}

int EmulationSettingsModel::getPlatformId() const { return m_platformId; }

void EmulationSettingsModel::setPlatformId(const int platformId) {
  if (platformId == m_platformId) {
    return;
  }

  // TODO: do stuff
  m_platformId = platformId;
  emit platformIdChanged();

  refreshValues();
}

int EmulationSettingsModel::getLevel() const { return m_level; }

void EmulationSettingsModel::setLevel(int level) {
  if (m_contentHash.isEmpty()) {
    return;
  }

  if (level == static_cast<int>(m_level)) {
    return;
  }

  m_level = static_cast<SettingsLevel>(level);
  emit levelChanged();

  m_settingsService->setSettingsLevel(m_contentHash.toStdString(), m_level);

  refreshValues();
}

QString EmulationSettingsModel::getContentHash() const { return m_contentHash; }
void EmulationSettingsModel::setContentHash(const QString &contentHash) {
  if (contentHash == m_contentHash) {
    return;
  }

  m_contentHash = contentHash;
  emit contentHashChanged();

  refreshValues();
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
  if (role == ValueRole) {
    if (item.type == "combobox" && value.canConvert<QString>()) {
      item.stringValue = value.toString();

      m_settingsService->setValue(m_level, m_contentHash.toStdString(),
                                  m_platformId, item.key.toStdString(),
                                  item.stringValue.toStdString());
      emit dataChanged(index, index, {ValueRole});
      return true;
    }
    if (item.type == "toggle" && value.canConvert<bool>()) {
      spdlog::info("Setting toggle value to {}", value.toBool());
      item.boolValue = value.toBool();
      auto strValue = item.boolValue ? item.trueValue : item.falseValue;

      m_settingsService->setValue(m_level, m_contentHash.toStdString(),
                                  m_platformId, item.key.toStdString(),
                                  strValue.toStdString());

      emit dataChanged(index, index, {ValueRole});
      return true;
    }
  }

  return {};
}

void EmulationSettingsModel::refreshValues() {
  if (m_level == Unknown) {
    return;
  }

  if ((m_level == Game && m_contentHash.isEmpty()) ||
      (m_level == Platform && m_platformId == -1)) {
    return;
  }

  for (auto i = 0; i < m_items.size(); ++i) {
    auto &item = m_items[i];
    auto valueString = item.defaultValue;
    auto val =
        m_settingsService->getValue(m_level, m_contentHash.toStdString(),
                                    m_platformId, item.key.toStdString());
    if (val) {
      valueString = QString::fromStdString(*val);
    }

    setItemValue(i, item, valueString.toStdString());
  }
}
void EmulationSettingsModel::setItemValue(int itemIndex, Item &item,
                                          const std::string &value) {
  if (item.type == "toggle") {
    if (value == item.trueValue.toStdString()) {
      item.boolValue = true;
    } else if (value == item.falseValue.toStdString()) {
      item.boolValue = false;
    } else {
      spdlog::warn("Unknown toggle value '{}' for key '{}', defaulting to "
                   "false",
                   value, item.key.toStdString());
      item.boolValue = false;
    }
  } else if (item.type == "combobox" && !item.options.isEmpty()) {
    for (const auto &option : item.options) {
      if (option.value("value").toString().toStdString() == value) {
        item.stringValue = option.value("value").toString();
        break;
      }
    }
  }

  emit dataChanged(index(itemIndex), index(itemIndex), {ValueRole});
}
} // namespace firelight::settings
