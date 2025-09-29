#include "core_configuration.hpp"

#include <algorithm>
#include <platform_metadata.hpp>
#include <platforms/platform_service.hpp>
#include <spdlog/spdlog.h>
#include <utility>

CoreConfiguration::CoreConfiguration(
    std::string contentHash, firelight::platforms::Platform platform,
    firelight::settings::SettingsService &settingsService)
    : m_contentHash(std::move(contentHash)), m_platform(std::move(platform)),
      m_settingsService(settingsService) {
  m_settingsLevel = m_settingsService.getSettingsLevel(m_contentHash);
}

void CoreConfiguration::registerOption(Option option) {
  m_options.emplace(option.key, option);

  if (!m_defaultValues.contains(option.key)) {
    setDefaultValue(option.key, option.defaultValueKey);
  }
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
  // if (firstAccess) {
  //     // print all keys and possible values
  //     for (const auto &option: m_options) {
  //         printf("Option %s (default: %s)\n", option.first.c_str(),
  //         option.second.defaultValueKey.c_str());
  //         //for (const auto &possibleValue: option.second.possibleValues) {
  //         //    printf("  %s\n", possibleValue.key.c_str());
  //         //}
  //     }
  //
  //
  //     firstAccess = false;
  // }

  // genesis_plus_gx_overclock
  // genesis_plus_gx_overclock

  // 1. See if we have it in the cache. If so, return it.
  // 2. If not, check the settings service for the value given the current
  // level.
  // 3. If it's there, cache it and return it.
  // 4. If not, get the default value from the platform service, cache, return.

  spdlog::info("Getting value for key {}...", key);
  if (m_cache.contains(key)) {
    return {m_cache[key]};
  }

  auto value = m_settingsService.getValue(m_settingsLevel, m_contentHash,
                                          m_platform.id, key);
  if (value.has_value()) {
    spdlog::info("...using user value {}", value->c_str());
    return value;
  }

  for (auto option : m_platform.emulationSettings) {
    if (option.key == key) {
      spdlog::info("...using Firelight default value {}", option.defaultValue);
      return option.defaultValue;
    }
  }

  auto defaultValue =
      firelight::PlatformMetadata::getDefaultConfigValue(m_platform.id, key);

  if (defaultValue.has_value()) {
    spdlog::info("...using Firelight default value {}", defaultValue->c_str());
    return defaultValue;
  }

  for (auto option : m_options) {
    if (option.first == key) {
      spdlog::info("...using core default value {}", option.second.key);
      return option.second.defaultValueKey;
    }
  }

  return std::nullopt;

  // if (!m_options.contains(key)) {
  //   return std::nullopt;
  // }
  //
  // // First, check for values set by core
  // // Second, check for values set specifically for game
  // // Third, check for values set specifically for platformconst
  // //
  // auto value = m_gameValues.find(key);
  // if (value != m_gameValues.end()) {
  //   spdlog::info("...found game value {}", value->second.c_str());
  //   return {value->second};
  // }
  //
  // value = m_platformValues.find(key);
  // if (value != m_platformValues.end()) {
  //   spdlog::info("...found platform value {}", value->second.c_str());
  //   return {value->second};
  // }
  //
  // value = m_defaultValues.find(key);
  // if (value != m_defaultValues.end()) {
  //   spdlog::info("...found default value {}", value->second.c_str());
  //   return {value->second};
  // }

  spdlog::info("...didn't find a value");
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
