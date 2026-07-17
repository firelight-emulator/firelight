#include "settings_model.hpp"

#include "libretro/core_registry.hpp"

#include <algorithm>
#include <firelight/library/user_library_service.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <spdlog/spdlog.h>

namespace firelight::settings {

namespace {
// Maps a catalog setting's declared control to a QML delegate id. `widget` is
// authoritative when set; otherwise it is derived from the value type.
QString widgetFor(const SettingDefinition &setting) {
  if (!setting.widget.empty()) {
    return QString::fromStdString(setting.widget);
  }
  switch (setting.type) {
  case SettingType::BOOLEAN:
    return "toggle";
  case SettingType::INTEGER:
    return "slider";
  case SettingType::STRING:
    return "text";
  case SettingType::CUSTOM:
    return "custom";
  case SettingType::OPTIONS:
    return "dropdown";
  }
  return "dropdown";
}
} // namespace

SettingsModel::SettingsModel(QObject *parent)
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

  // An audio-device row's options are the machine's outputs, so plugging in
  // headphones has to rebuild them.
  m_mediaDevices = new QMediaDevices(this);
  connect(m_mediaDevices, &QMediaDevices::audioOutputsChanged, this,
          [this] { rebuildItems(); });

  rebuildItems();
}

void SettingsModel::rebuildItems() {
  beginResetModel();
  m_items.clear();

  const auto &catalog = SettingsCatalog::instance();
  // No platform in scope means no core in scope: the global tier shows the
  // frontend settings only, never some arbitrary core's.
  const auto coreName =
      m_platformId == -1
          ? std::string{}
          : CoreRegistry::instance().resolveCoreName(
                m_platformId, m_contentHash.toStdString(), m_settingsService);

  const auto settings =
      catalog.settingsForGroup(m_group.toStdString(), coreName);

  for (const auto &setting : settings) {
    Item item;
    item.label = QString::fromStdString(setting.label);
    item.key = QString::fromStdString(setting.key);
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
    item.appScope = catalog.isAppSetting(setting.key);
    item.visibleWhen = setting.visibleWhen;
    item.enabledWhen = setting.enabledWhen;
    item.placeholder = QString::fromStdString(setting.placeholder);
    item.directoryMode = setting.directoryMode;
    for (const auto &ext : setting.fileExtensions) {
      item.fileExtensions.append(QString::fromStdString(ext));
    }
    if (setting.libraryGameSource) {
      item.options = buildGameOptions(setting);
    } else if (setting.audioDeviceSource) {
      item.options = buildAudioDeviceOptions();
    } else {
      for (const auto &option : setting.options) {
        item.options.emplace_back(
            QVariantHash{{"label", QString::fromStdString(option.label)},
                         {"value", QString::fromStdString(option.value)}});
      }
    }
    m_items.emplace_back(std::move(item));
  }

  markSubItems();

  endResetModel();
  refreshValues();
}

void SettingsModel::markSubItems() {
  // A row that depends on another row shown right here is a sub-setting of it,
  // and renders indented and welded beneath it. Derived from the dependency the
  // catalog already declares rather than a second thing to author — and scoped
  // to what's on screen, since depending on a setting from another page says
  // nothing about how this one should look.
  const auto shownHere = [this](const std::string &key) {
    return std::any_of(m_items.begin(), m_items.end(), [&](const Item &other) {
      return other.key.toStdString() == key;
    });
  };
  const auto dependsOnAShownRow =
      [&](const std::vector<SettingCondition> &conditions) {
        return std::any_of(
            conditions.begin(), conditions.end(),
            [&](const SettingCondition &c) { return shownHere(c.key); });
      };

  for (auto &item : m_items) {
    item.subItem = dependsOnAShownRow(item.visibleWhen) ||
                   dependsOnAShownRow(item.enabledWhen);
  }
}

QVector<QVariantHash>
SettingsModel::buildGameOptions(const SettingDefinition &setting) const {
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

bool SettingsModel::overridesInheritedValue(const Item &item) const {
  // Reset means "clear this tier's override and go back to what I inherit", so
  // it only means something at a tier with one beneath it. The global tier and
  // app settings are the base: there's nothing below to fall back to, so no
  // Reset — changing the value is simply the value.
  if (item.appScope || (m_level != Game && m_level != Platform)) {
    return false;
  }
  if (!m_settingsService) {
    return false;
  }
  const auto hash = m_contentHash.toStdString();
  const auto key = item.key.toStdString();
  const auto stored =
      m_settingsService->getValueAtLevel(m_level, hash, m_platformId, key);
  if (!stored) {
    return false;
  }
  // An override that matches what it would inherit anyway isn't overriding
  // anything the user can see, so offering to clear it is just noise.
  const auto inherited =
      m_level == Game
          ? resolveValueFrom(key, Platform)
          : resolveValueFrom(key, Global);
  return stored != inherited.value_or(item.defaultValue.toStdString());
}

std::optional<std::string>
SettingsModel::resolveValueFrom(const std::string &key,
                                const SettingsLevel level) const {
  const auto hash = m_contentHash.toStdString();
  if (level <= Platform) {
    if (auto v =
            m_settingsService->getValueAtLevel(Platform, hash, m_platformId, key)) {
      return v;
    }
  }
  return m_settingsService->getValueAtLevel(Global, hash, m_platformId, key);
}

SettingsLevel SettingsModel::levelFor(const Item &item) const {
  // An app setting is an emulation setting pinned to the global tier: it has no
  // core mapping and nothing overrides it per-platform or per-game.
  return item.appScope ? Global : m_level;
}

bool SettingsModel::canResolve(const Item &item) const {
  if (item.appScope) {
    return m_settingsService != nullptr;
  }
  if (!m_settingsService || m_level == Unknown) {
    return false;
  }
  if (m_level == Game && m_contentHash.isEmpty()) {
    return false;
  }
  return !(m_level == Platform && m_platformId == -1);
}

QVector<QVariantHash> SettingsModel::buildAudioDeviceOptions() const {
  QVector<QVariantHash> options;
  // "" means "whatever the OS default is", which follows the OS if it changes.
  options.append(QVariantHash{{"label", QStringLiteral("System default")},
                              {"value", QString()}});
  for (const auto &device : QMediaDevices::audioOutputs()) {
    options.append(QVariantHash{{"label", device.description()},
                                {"value", device.description()}});
  }
  return options;
}

std::optional<std::string>
SettingsModel::resolveValue(const std::string &key, const SettingsLevel level) {
  if (!m_settingsService) {
    return std::nullopt;
  }
  const auto hash = m_contentHash.toStdString();
  if (level <= Game) {
    if (auto v = m_settingsService->getValueAtLevel(Game, hash, m_platformId, key)) {
      return v;
    }
  }
  if (level <= Platform) {
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

std::string SettingsModel::currentValueOf(const std::string &key) const {
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

void SettingsModel::recomputeConditions() {
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

QString SettingsModel::getGroup() const { return m_group; }

QString SettingsModel::getGroupLabel() const {
  const auto *group =
      SettingsCatalog::instance().findGroup(m_group.toStdString());
  return group ? QString::fromStdString(group->label) : QString();
}

void SettingsModel::setGroup(const QString &group) {
  if (m_group == group) {
    return;
  }
  m_group = group;
  emit groupChanged();
  rebuildItems();
}

bool SettingsModel::getShowAdvanced() const { return m_showAdvanced; }

void SettingsModel::setShowAdvanced(const bool showAdvanced) {
  if (m_showAdvanced == showAdvanced) {
    return;
  }
  m_showAdvanced = showAdvanced;
  emit showAdvancedChanged();
  recomputeConditions();
}

int SettingsModel::getPlatformId() const { return m_platformId; }

void SettingsModel::setPlatformId(const int platformId) {
  if (platformId == m_platformId) {
    return;
  }
  m_platformId = platformId;
  emit platformIdChanged();
  rebuildItems();
}

int SettingsModel::getLevel() const { return m_level; }

void SettingsModel::setLevel(int level) {
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

QString SettingsModel::getContentHash() const { return m_contentHash; }

void SettingsModel::setContentHash(const QString &contentHash) {
  if (contentHash == m_contentHash) {
    return;
  }
  m_contentHash = contentHash;
  emit contentHashChanged();
  refreshValues();
}

int SettingsModel::rowCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : m_items.size();
}

QVariant SettingsModel::data(const QModelIndex &index,
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
  case ResettableRole:
    return item.resettable;
  case SubItemRole:
    return item.subItem;
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

QHash<int, QByteArray> SettingsModel::roleNames() const {
  return {{LabelRole, "label"},
          {KeyRole, "key"},
          {DescriptionRole, "description"},
          {WidgetRole, "widget"},
          {ValueRole, "value"},
          {DefaultValueRole, "defaultValue"},
          {OptionsRole, "options"},
          {MinimumRole, "minimumValue"},
          {MaximumRole, "maximumValue"},
          {StepRole, "stepValue"},
          {ResettableRole, "resettable"},
          {SubItemRole, "subItem"},
          {VisibleRole, "visible"},
          {EnabledRole, "enabled"},
          {RequiresRestartRole, "requiresRestart"},
          {PlaceholderRole, "placeholder"},
          {FileExtensionsRole, "fileExtensions"},
          {DirectoryModeRole, "directoryMode"}};
}

Qt::ItemFlags SettingsModel::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

bool SettingsModel::setData(const QModelIndex &index,
                                     const QVariant &value, const int role) {
  if (!index.isValid() || index.row() >= m_items.size() || role != ValueRole) {
    return false;
  }

  auto &item = m_items[index.row()];
  if (!canResolve(item)) {
    return false;
  }

  QString stringValue;
  if (item.widget == "toggle") {
    item.boolValue = value.toBool();
    stringValue = item.boolValue ? item.trueValue : item.falseValue;
  } else {
    stringValue = value.toString();
    item.stringValue = stringValue;
  }

  m_settingsService->setValueAtLevel(levelFor(item), m_contentHash.toStdString(),
                                     m_platformId, item.key.toStdString(),
                                     stringValue.toStdString());
  item.resettable = overridesInheritedValue(item);
  emit dataChanged(index, index, {ValueRole, ResettableRole});
  // A change here can flip the visibility/enablement of dependent settings.
  recomputeConditions();
  return true;
}

void SettingsModel::resetValue(int row) {
  if (row < 0 || row >= m_items.size()) {
    return;
  }

  auto &item = m_items[row];
  if (!canResolve(item)) {
    return;
  }

  const auto level = levelFor(item);
  m_settingsService->resetValueAtLevel(level, m_contentHash.toStdString(),
                                       m_platformId, item.key.toStdString());
  const auto resolved = resolveValue(item.key.toStdString(), level);
  setItemValue(row, item, resolved.value_or(item.defaultValue.toStdString()));
  item.resettable = false;
  emit dataChanged(index(row), index(row), {ValueRole, ResettableRole});
  recomputeConditions();
}

void SettingsModel::refreshValues() {
  for (int i = 0; i < m_items.size(); ++i) {
    auto &item = m_items[i];
    if (!canResolve(item)) {
      continue;
    }
    const auto level = levelFor(item);
    const auto resolved = resolveValue(item.key.toStdString(), level);
    setItemValue(i, item, resolved.value_or(item.defaultValue.toStdString()));
    item.resettable = overridesInheritedValue(item);
  }
  if (!m_items.isEmpty()) {
    emit dataChanged(index(0), index(m_items.size() - 1),
                     {ValueRole, ResettableRole});
  }
  recomputeConditions();
}

void SettingsModel::setItemValue(int itemIndex, Item &item,
                                          const std::string &value) {
  if (item.widget == "toggle") {
    item.boolValue = value == item.trueValue.toStdString();
  } else {
    item.stringValue = QString::fromStdString(value);
  }
  emit dataChanged(index(itemIndex), index(itemIndex), {ValueRole});
}

} // namespace firelight::settings
