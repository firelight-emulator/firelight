#include "emulation_settings_model.hpp"

#include "libretro/core_registry.hpp"
#include "platform_metadata.hpp"

#include <algorithm>
#include <firelight/library/user_library_service.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <spdlog/spdlog.h>

namespace firelight::settings {

namespace {
// Maps a catalog setting's declared control to a QML delegate id. `widget` is
// authoritative when set; otherwise it is derived from the value type.
QString widgetFor(const EmulationSetting &setting) {
  if (!setting.widget.empty()) {
    return QString::fromStdString(setting.widget);
  }
  switch (setting.type) {
  case BOOLEAN:
    return "toggle";
  case INTEGER:
    return "slider";
  case STRING:
    return "text";
  case CUSTOM:
    return "custom";
  case OPTIONS:
  default:
    return "dropdown";
  }
}
} // namespace

EmulationSettingsModel::EmulationSettingsModel(QObject *parent)
    : QAbstractListModel(parent) {
  // A "core" change re-resolves which core (and thus which settings) apply, so
  // the item set must be rebuilt; any other key just refreshes values.
  const auto onMatch = [this](bool matches, const std::string &key) {
    if (!matches) {
      return;
    }
    if (key == CoreRegistry::CORE_SETTING_KEY) {
      rebuildItems();
    } else {
      refreshValues();
    }
  };

  m_globalSettingChangedConnection =
      EventDispatcher::instance().subscribe<GlobalSettingChangedEvent>(
          [this](const GlobalSettingChangedEvent &) { refreshValues(); });

  m_platformSettingChangedConnection =
      EventDispatcher::instance().subscribe<PlatformSettingChangedEvent>(
          [this, onMatch](const PlatformSettingChangedEvent &e) {
            onMatch(e.platformId == m_platformId, e.key);
          });

  m_gameSettingChangedConnection =
      EventDispatcher::instance().subscribe<GameSettingChangedEvent>(
          [this, onMatch](const GameSettingChangedEvent &e) {
            onMatch(e.contentHash == m_contentHash.toStdString(), e.key);
          });

  rebuildItems();
}

void EmulationSettingsModel::rebuildItems() {
  beginResetModel();
  m_items.clear();

  const auto &catalog = SettingsCatalog::instance();
  std::vector<EmulationSetting> settings = catalog.commonSettings();
  if (m_platformId != -1) {
    const auto coreName = CoreRegistry::instance().resolveCoreName(
        m_platformId, m_contentHash.toStdString(), m_settingsService);
    for (const auto &s : catalog.coreSpecificSettings(coreName)) {
      settings.push_back(s);
    }
  }

  for (const auto &setting : settings) {
    Item item;
    item.label = QString::fromStdString(setting.label);
    item.key = QString::fromStdString(setting.key);
    item.section = QString::fromStdString(setting.category);
    item.description = QString::fromStdString(setting.description);
    item.widget = widgetFor(setting);
    item.defaultValue = QString::fromStdString(setting.defaultValue);
    item.trueValue = QString::fromStdString(setting.trueStringValue);
    item.falseValue = QString::fromStdString(setting.falseStringValue);
    item.minimumValue = setting.minValue;
    item.maximumValue = setting.maxValue;
    item.stepValue = setting.stepValue;
    item.requiresRestart = setting.requiresRestart;
    item.advanced = setting.advanced;
    item.visibleWhen = setting.visibleWhen;
    item.enabledWhen = setting.enabledWhen;
    item.placeholder = QString::fromStdString(setting.placeholder);
    item.directoryMode = setting.directoryMode;
    for (const auto &ext : setting.fileExtensions) {
      item.fileExtensions.append(QString::fromStdString(ext));
    }
    if (setting.libraryGameSource) {
      item.options = buildGameOptions(setting);
    } else {
      for (const auto &option : setting.options) {
        item.options.emplace_back(
            QVariantHash{{"label", QString::fromStdString(option.label)},
                         {"value", QString::fromStdString(option.value)}});
      }
    }
    m_items.emplace_back(std::move(item));
  }

  endResetModel();
  refreshValues();
}

QVector<QVariantHash>
EmulationSettingsModel::buildGameOptions(const EmulationSetting &setting) const {
  QVector<QVariantHash> options;
  // A leading "None" (empty value) lets the user clear the choice.
  options.append(
      QVariantHash{{"label", QStringLiteral("None")}, {"value", QString()}});

  auto *library = getLibraryService();
  if (!library) {
    return options;
  }

  const auto &ids = setting.gamePickerPlatformIds;
  const auto isEligible = [&ids](int platformId) {
    return ids.empty() ||
           std::find(ids.begin(), ids.end(), platformId) != ids.end();
  };

  auto entries = library->getEntries();
  std::sort(entries.begin(), entries.end(),
            [](const library::Entry &a, const library::Entry &b) {
              return QString::fromStdString(a.displayName)
                         .localeAwareCompare(
                             QString::fromStdString(b.displayName)) < 0;
            });

  for (const auto &entry : entries) {
    if (!isEligible(static_cast<int>(entry.platformId))) {
      continue;
    }
    options.append(
        QVariantHash{{"label", QString::fromStdString(entry.displayName)},
                     {"value", QString::fromStdString(entry.contentHash)}});
  }
  return options;
}

std::optional<std::string>
EmulationSettingsModel::resolveValue(const std::string &key) {
  if (!m_settingsService) {
    return std::nullopt;
  }
  const auto hash = m_contentHash.toStdString();
  if (m_level <= Game) {
    if (auto v = m_settingsService->getValueAtLevel(Game, hash, m_platformId, key)) {
      return v;
    }
  }
  if (m_level <= Platform) {
    if (auto v =
            m_settingsService->getValueAtLevel(Platform, hash, m_platformId, key)) {
      return v;
    }
  }
  if (auto v = m_settingsService->getValueAtLevel(Global, hash, m_platformId, key)) {
    return v;
  }
  return std::nullopt;
}

std::string EmulationSettingsModel::currentValueOf(const std::string &key) const {
  for (const auto &item : m_items) {
    if (item.key.toStdString() == key) {
      if (item.widget == "toggle") {
        return (item.boolValue ? item.trueValue : item.falseValue).toStdString();
      }
      return item.stringValue.toStdString();
    }
  }
  return {};
}

void EmulationSettingsModel::recomputeConditions() {
  const SettingValueResolver resolver = [this](const std::string &key) {
    return currentValueOf(key);
  };
  for (auto &item : m_items) {
    item.visible = conditionsHold(item.visibleWhen, resolver) &&
                   (!item.advanced || m_showAdvanced);
    item.enabled = conditionsHold(item.enabledWhen, resolver);
  }
  if (!m_items.isEmpty()) {
    emit dataChanged(index(0), index(m_items.size() - 1),
                     {VisibleRole, EnabledRole});
  }
}

bool EmulationSettingsModel::getShowAdvanced() const { return m_showAdvanced; }

void EmulationSettingsModel::setShowAdvanced(const bool showAdvanced) {
  if (m_showAdvanced == showAdvanced) {
    return;
  }
  m_showAdvanced = showAdvanced;
  emit showAdvancedChanged();
  recomputeConditions();
}

int EmulationSettingsModel::getPlatformId() const { return m_platformId; }

void EmulationSettingsModel::setPlatformId(const int platformId) {
  if (platformId == m_platformId) {
    return;
  }
  m_platformId = platformId;
  emit platformIdChanged();
  rebuildItems();
}

int EmulationSettingsModel::getLevel() const { return m_level; }

void EmulationSettingsModel::setLevel(int level) {
  if (level == static_cast<int>(m_level)) {
    return;
  }
  // `level` is just the tier this surface edits (Game / Platform / Global); it
  // is not a stored per-game "read level" — the running game resolves settings
  // by inheritance.
  m_level = static_cast<SettingsLevel>(level);
  emit levelChanged();
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
  return parent.isValid() ? 0 : m_items.size();
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
  case WidgetRole:
    return item.widget;
  case ValueRole:
    if (item.widget == "toggle") {
      return item.boolValue;
    }
    return item.stringValue;
  case DefaultValueRole:
    return item.defaultValue;
  case OptionsRole:
    return QVariant::fromValue(item.options);
  case MinimumRole:
    return item.minimumValue;
  case MaximumRole:
    return item.maximumValue;
  case StepRole:
    return item.stepValue;
  case OverriddenRole:
    return item.overridden;
  case VisibleRole:
    return item.visible;
  case EnabledRole:
    return item.enabled;
  case RequiresRestartRole:
    return item.requiresRestart;
  case PlaceholderRole:
    return item.placeholder;
  case FileExtensionsRole:
    return item.fileExtensions;
  case DirectoryModeRole:
    return item.directoryMode;
  default:
    return {};
  }
}

QHash<int, QByteArray> EmulationSettingsModel::roleNames() const {
  return {{LabelRole, "label"},
          {KeyRole, "key"},
          {DescriptionRole, "description"},
          {SectionRole, "section"},
          {WidgetRole, "widget"},
          {ValueRole, "value"},
          {DefaultValueRole, "defaultValue"},
          {OptionsRole, "options"},
          {MinimumRole, "minimumValue"},
          {MaximumRole, "maximumValue"},
          {StepRole, "stepValue"},
          {OverriddenRole, "overridden"},
          {VisibleRole, "visible"},
          {EnabledRole, "enabled"},
          {RequiresRestartRole, "requiresRestart"},
          {PlaceholderRole, "placeholder"},
          {FileExtensionsRole, "fileExtensions"},
          {DirectoryModeRole, "directoryMode"}};
}

Qt::ItemFlags EmulationSettingsModel::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

bool EmulationSettingsModel::setData(const QModelIndex &index,
                                     const QVariant &value, const int role) {
  if (!index.isValid() || index.row() >= m_items.size() || role != ValueRole ||
      m_level == Unknown || !m_settingsService) {
    return false;
  }

  auto &item = m_items[index.row()];
  QString stringValue;
  if (item.widget == "toggle") {
    item.boolValue = value.toBool();
    stringValue = item.boolValue ? item.trueValue : item.falseValue;
  } else {
    stringValue = value.toString();
    item.stringValue = stringValue;
  }

  m_settingsService->setValueAtLevel(m_level, m_contentHash.toStdString(),
                                     m_platformId, item.key.toStdString(),
                                     stringValue.toStdString());
  item.overridden = true;
  emit dataChanged(index, index, {ValueRole, OverriddenRole});
  // A change here can flip the visibility/enablement of dependent settings.
  recomputeConditions();
  return true;
}

void EmulationSettingsModel::resetValue(int row) {
  if (row < 0 || row >= m_items.size() || m_level == Unknown ||
      !m_settingsService) {
    return;
  }

  auto &item = m_items[row];
  m_settingsService->resetValueAtLevel(m_level, m_contentHash.toStdString(),
                                       m_platformId, item.key.toStdString());
  const auto resolved = resolveValue(item.key.toStdString());
  setItemValue(row, item, resolved.value_or(item.defaultValue.toStdString()));
  item.overridden = false;
  emit dataChanged(index(row), index(row), {ValueRole, OverriddenRole});
  recomputeConditions();
}

void EmulationSettingsModel::refreshValues() {
  if (m_level == Unknown) {
    return;
  }
  if ((m_level == Game && m_contentHash.isEmpty()) ||
      (m_level == Platform && m_platformId == -1)) {
    return;
  }

  for (int i = 0; i < m_items.size(); ++i) {
    auto &item = m_items[i];
    const auto resolved = resolveValue(item.key.toStdString());
    setItemValue(i, item, resolved.value_or(item.defaultValue.toStdString()));
    item.overridden =
        m_settingsService && m_settingsService
                                 ->getValueAtLevel(m_level, m_contentHash.toStdString(),
                                                   m_platformId, item.key.toStdString())
                                 .has_value();
  }
  if (!m_items.isEmpty()) {
    emit dataChanged(index(0), index(m_items.size() - 1),
                     {ValueRole, OverriddenRole});
  }
  recomputeConditions();
}

void EmulationSettingsModel::setItemValue(int itemIndex, Item &item,
                                          const std::string &value) {
  if (item.widget == "toggle") {
    item.boolValue = value == item.trueValue.toStdString();
  } else {
    item.stringValue = QString::fromStdString(value);
  }
  emit dataChanged(index(itemIndex), index(itemIndex), {ValueRole});
}

} // namespace firelight::settings
