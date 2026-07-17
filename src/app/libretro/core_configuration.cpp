#include "core_configuration.hpp"

#include <algorithm>
#include <spdlog/spdlog.h>
#include <utility>

#include "platform_core_defaults.hpp"

CoreConfiguration::CoreConfiguration(
    std::string contentHash, const int platformId,
    std::vector<firelight::settings::SettingDefinition> friendlySettings,
    std::map<std::string, std::string> coreDefaults,
    firelight::settings::SettingsService &settingsService)
    : m_contentHash(std::move(contentHash)), m_platformId(platformId),
      m_friendlySettings(std::move(friendlySettings)),
      m_coreDefaults(std::move(coreDefaults)),
      m_settingsService(settingsService) {}

void CoreConfiguration::registerOption(Option option) {
  m_options.emplace(option.key, option);

  if (!m_defaultValues.contains(option.key)) {
    setDefaultValue(option.key, option.defaultValueKey);
  }
}

std::vector<CoreConfiguration::Option> CoreConfiguration::getOptions() const {
  std::vector<Option> options;
  options.reserve(m_options.size());
  for (const auto &[key, option] : m_options) {
    options.push_back(option);
  }
  return options;
}

bool CoreConfiguration::anyOptionValueHasChanged() {
  const auto val = m_changedSinceLastChecked;
  m_changedSinceLastChecked = false;

  return val;
}

void CoreConfiguration::setDefaultValue(const std::string &key,
                                        const std::string &value) {
  const auto opt = m_options.find(key);
  if (opt == m_options.end()) {
    return;
  }

  for (auto &possibleValue : opt->second.possibleValues) {
    if (possibleValue.key == value || possibleValue.label == value) {
      m_defaultValues[key] = value;
      m_changedSinceLastChecked = true;
      break;
    }
  }
}

std::optional<std::string>
CoreConfiguration::getOptionValue(const std::string &key) {
  // Resolution for a core option `key` (higher wins):
  //  1. cache (currently never populated; kept as a cheap short-circuit);
  //  2. a raw user override of this exact core option (the "advanced" editor),
  //     resolved game -> platform -> global;
  //  3. a friendly setting whose mapping targets this core option, translated
  //     from its effective (or default) friendly value;
  //  4. a Firelight friendly default whose key *is* this core option (identity);
  //  5. a hardcoded per-platform default;
  //  6. the core's own declared default.
  if (m_cache.contains(key)) {
    return {m_cache[key]};
  }

  // 2. Raw override of the exact core key ("advanced" beats "friendly").
  if (auto raw = m_settingsService.getEffectiveValue(m_contentHash,
                                                     m_platformId, key)) {
    return raw;
  }

  // 3. A friendly setting whose mapping drives this core option.
  for (const auto &setting : m_friendlySettings) {
    for (const auto &mapping : setting.mapping) {
      if (mapping.coreKey != key) {
        continue;
      }
      const auto friendlyValue =
          m_settingsService
              .getEffectiveValue(m_contentHash, m_platformId, setting.key)
              .value_or(setting.defaultValue);
      const auto it = mapping.valueMap.find(friendlyValue);
      return it != mapping.valueMap.end() ? it->second : friendlyValue;
    }
  }

  // 4. Identity friendly default (the setting's key is itself the core key).
  for (const auto &setting : m_friendlySettings) {
    if (setting.key == key) {
      return setting.defaultValue;
    }
  }

  // 5. Firelight per-core default override (from the settings catalog).
  if (const auto it = m_coreDefaults.find(key); it != m_coreDefaults.end()) {
    return it->second;
  }

  // 5b. Legacy hardcoded per-platform default (cores not yet in the catalog).
  if (auto def = firelight::defaultCoreOptionForPlatform(m_platformId, key)) {
    return def;
  }

  // 6. The core's own declared default.
  if (const auto opt = m_options.find(key); opt != m_options.end()) {
    return opt->second.defaultValueKey;
  }

  return std::nullopt;
}

void CoreConfiguration::setOptionVisibility(const std::string &key,
                                            bool visible) {}

void CoreConfiguration::setPlatformValue(const std::string &key,
                                         const std::string &value) {
  m_platformValues[key] = value;
  m_changedSinceLastChecked = true;
}

void CoreConfiguration::setGameValue(const std::string &key,
                                     const std::string &value) {
  m_gameValues[key] = value;
  m_changedSinceLastChecked = true;
}
