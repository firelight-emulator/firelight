#include "core_configuration.hpp"

#include <algorithm>
#include <spdlog/spdlog.h>

static bool firstAccess = true;

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

void CoreConfiguration::setDefaultValue(const std::string &key, const std::string &value) {
    const auto opt = m_options.find(key);
    if (opt == m_options.end()) {
        return;
    }

    for (auto &possibleValue: opt->second.possibleValues) {
        if (possibleValue.key == value || possibleValue.label == value) {
            m_defaultValues[key] = value;
            m_changedSinceLastChecked = true;
            break;
        }
    }
}

std::optional<firelight::libretro::IConfigurationProvider::OptionValue>
CoreConfiguration::getOptionValue(const std::string &key) {
    // if (firstAccess) {
    //     // print all keys and possible values
    //     for (const auto &option: m_options) {
    //         printf("Option %s (default: %s)\n", option.first.c_str(), option.second.defaultValueKey.c_str());
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

    spdlog::info("Getting value for key {}...", key);

    if (!m_options.contains(key)) {
        return std::nullopt;
    }

    // First, check for values set by core
    // Second, check for values set specifically for game
    // Third, check for values set specifically for platformconst
    //
    auto value = m_gameValues.find(key);
    if (value != m_gameValues.end()) {
        spdlog::info("...found game value {}", value->second.c_str());
        return {{value->second, value->second}};
    }

    value = m_platformValues.find(key);
    if (value != m_platformValues.end()) {
        spdlog::info("...found platform value {}", value->second.c_str());
        return {{value->second, value->second}};
    }

    value = m_defaultValues.find(key);
    if (value != m_defaultValues.end()) {
        spdlog::info("...found default value {}", value->second.c_str());
        return {{value->second, value->second}};
    }

    spdlog::info("...didn't find a value");
    return std::nullopt;
}

void CoreConfiguration::setOptionVisibility(const std::string &key, bool visible) {
}

void CoreConfiguration::setPlatformValue(const std::string &key, const std::string &value) {
    m_platformValues[key] = value;
    m_changedSinceLastChecked = true;
}

void CoreConfiguration::setGameValue(const std::string &key, const std::string &value) {
    m_gameValues[key] = value;
    m_changedSinceLastChecked = true;
}
