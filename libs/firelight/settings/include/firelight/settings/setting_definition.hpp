#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace firelight::settings {

// Value semantics of a setting. The concrete UI control is chosen separately (see SettingDefinition::widget)
enum class SettingType { BOOLEAN, OPTIONS, INTEGER, STRING, CUSTOM };

struct SettingOption {
  std::string label;
  std::string value;
};

// A dependency clause, matches when key is one of the strings in values
struct SettingCondition {
  std::string key;
  std::vector<std::string> values;
};

// Resolves the current (effective-or-default) value of another setting by key
using SettingValueResolver = std::function<std::string(const std::string &key)>;

inline bool conditionsHold(const std::vector<SettingCondition> &conditions, const SettingValueResolver &resolve) {
  for (const auto &c : conditions) {
    const auto current = resolve(c.key);
    if (std::find(c.values.begin(), c.values.end(), current) == c.values.end()) {
      return false;
    }
  }
  return true; // no conditions -> always holds
}

// Maps a friendly emulation setting onto one libretro core option. A friendly
// setting may carry several of these (composite: one friendly control drives
// multiple core options)
struct CoreOptionMapping {
  std::string coreKey;
  std::map<std::string, std::string> valueMap; // when key is X, set coreKey to value
};

struct SettingDefinition {
  std::string label;
  std::string key;
  std::string description;
  std::string longDescription;
  std::string defaultValue;
  SettingType type = SettingType::OPTIONS;
  std::vector<std::string> keywords; // for search
  bool requiresRestart = false;
  std::string trueStringValue = "true";
  std::string falseStringValue = "false";
  std::vector<SettingOption> options;

  // Values for integers TODO: doubles?
  double minValue = 0.0;
  double maxValue = 0.0;
  double stepValue = 1.0;

  std::string widget; // widget to use in GUI, see SettingsGroup.qml

  bool advanced = false;

  std::vector<CoreOptionMapping> mapping; // Friendly setting -> core options, if applicable

  bool libraryGameSource = false; // Whether the options are the library's games instead of hardcoded values
  std::vector<int> gamePickerPlatformIds; // Platform ids to filter on when libraryGameSource is true

  bool audioDeviceSource = false; // Whether the options are the system's audio devices instead of hardcoded values

  std::string placeholder; // Placeholder text for a text widget

  std::vector<std::string> fileExtensions; // Accepted file extensions for a `file-picker` widget (NO DOTS)
  bool directoryMode = false; // Pick directory rather than file

  std::vector<SettingCondition> visibleWhen;
  std::vector<SettingCondition> enabledWhen;

  // Whether the row draws indented beneath the setting it depends on. If not present then derived from visibleWhen and enabledWhen
  std::optional<bool> subItem;

  std::string route; // link for the setting row to navigate to another page
};

// A settings page (page has multiple groups)
struct SettingsPage {
  std::string id;
  std::string label;
  std::string icon;
  std::string route;
  std::vector<std::string> keywords; // keywords for page itself
  std::vector<std::string> groupIds; // ordered
};

// A titled group of rows within a page
struct SettingsGroup {
  std::string id;
  std::string label;
  std::vector<std::string> settingKeys;
};

inline bool settingIsVisible(const SettingDefinition &setting, const SettingValueResolver &resolve) {
  return conditionsHold(setting.visibleWhen, resolve);
}

inline bool settingIsEnabled(const SettingDefinition &setting, const SettingValueResolver &resolve) {
  return conditionsHold(setting.enabledWhen, resolve);
}

// Resolves the concrete (coreKey, coreValue) pairs a friendly setting value maps to
inline std::vector<std::pair<std::string, std::string>> resolveCoreOptionValues(const SettingDefinition &setting,
                                                                                const std::string &friendlyValue) {
  std::vector<std::pair<std::string, std::string>> result;
  result.reserve(setting.mapping.size());
  for (const auto &m : setting.mapping) {
    const auto it = m.valueMap.find(friendlyValue);
    result.emplace_back(m.coreKey, it != m.valueMap.end() ? it->second : friendlyValue);
  }
  return result;
}

} // namespace firelight::settings
