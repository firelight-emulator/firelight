#include "core_configuration.hpp"

#include <algorithm>

void CoreConfiguration::registerOption(Option option) {
    m_options.emplace(option.key, option);
    setOptionValue(option.key, option.defaultValueKey);
}

bool CoreConfiguration::anyOptionValueHasChanged() {
    const auto val = m_changedSinceLastChecked;
    m_changedSinceLastChecked = false;

    return val;
}

void CoreConfiguration::setOptionValue(std::string key, std::string value) {
    const auto opt = m_options.find(key);
    if (opt == m_options.end()) {
        return;
    }

    for (int i = 0; i < opt->second.possibleValues.size(); i++) {
        if (opt->second.possibleValues[i].key == value) {
            m_optionValueIndices[key] = i;
            m_changedSinceLastChecked = true;
            break;
        }
    }
}

std::optional<firelight::libretro::IConfigurationProvider::OptionValue>
CoreConfiguration::getOptionValue(const std::string key) {
    printf("Getting value for key %s...", key.c_str());
    const auto opt = m_options.find(key);
    if (opt == m_options.end()) {
        return std::nullopt;
    }

    const auto index = m_optionValueIndices.find(key);
    if (index == m_optionValueIndices.end()) {
        return std::nullopt;
    }

    auto val = opt->second.possibleValues.at(index->second);

    printf("found value %s\n", val.key.c_str());

    return {val};
}

void CoreConfiguration::setOptionVisibility(std::string key, bool visible) {
}
