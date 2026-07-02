#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace firelight::settings {

// Value semantics of a setting. The concrete UI control is chosen separately
// (see EmulationSetting::widget) so, e.g., an INTEGER can render as a slider or
// a spinbox and a CUSTOM setting can use a bespoke delegate.
enum EmulationSettingType { BOOLEAN, OPTIONS, INTEGER, CUSTOM };

struct EmulationSettingOption {
  std::string label;
  std::string value;
};

// A dependency clause: holds when the setting identified by `key` currently has
// one of `values`. Multiple clauses on a setting are AND-ed.
struct SettingCondition {
  std::string key;
  std::vector<std::string> values;
};

// Resolves the current (effective-or-default) value of another setting by key.
using SettingValueResolver = std::function<std::string(const std::string &key)>;

inline bool conditionsHold(const std::vector<SettingCondition> &conditions,
                           const SettingValueResolver &resolve) {
  for (const auto &c : conditions) {
    const auto current = resolve(c.key);
    if (std::find(c.values.begin(), c.values.end(), current) == c.values.end()) {
      return false;
    }
  }
  return true; // no conditions => always holds
}

// Maps a friendly emulation setting onto one libretro core option. A friendly
// setting may carry several of these (composite: one friendly control drives
// multiple core options).
struct CoreOptionMapping {
  std::string coreKey;
  // friendlyValue -> coreValue. Empty means identity: the friendly value is
  // written to the core option unchanged.
  std::map<std::string, std::string> valueMap;
};

struct EmulationSetting {
  std::string label;
  std::string category;
  std::string key;
  std::string description;
  std::string defaultValue;
  EmulationSettingType type;
  bool requiresRestart = false;
  std::string trueStringValue = "true";
  std::string falseStringValue = "false";
  std::vector<EmulationSettingOption> options;
  // INTEGER numeric range (slider / spinbox).
  double minValue = 0.0;
  double maxValue = 0.0;
  double stepValue = 1.0;
  // UI control. Empty => derived from `type` (toggle / dropdown / slider). Set
  // to a specific control ("spinbox") or, for CUSTOM, a bespoke QML delegate id
  // (e.g. "gbc-palette"). Keeps custom widgets open-ended without schema churn.
  std::string widget;
  // Empty => a frontend-only setting (consumed by EmulatorInstance) or a
  // setting whose `key` is itself the core option key (identity). Non-empty =>
  // an explicit friendly->core mapping applied when composing core values.
  std::vector<CoreOptionMapping> mapping;
  // Relationships to other settings. Empty => always visible/enabled.
  std::vector<SettingCondition> visibleWhen;
  std::vector<SettingCondition> enabledWhen;
};

// Whether a setting should be shown / editable given the current values of the
// settings it depends on (resolver returns another setting's effective value).
inline bool settingIsVisible(const EmulationSetting &setting,
                             const SettingValueResolver &resolve) {
  return conditionsHold(setting.visibleWhen, resolve);
}
inline bool settingIsEnabled(const EmulationSetting &setting,
                             const SettingValueResolver &resolve) {
  return conditionsHold(setting.enabledWhen, resolve);
}

// Resolves the concrete (coreKey, coreValue) pairs a friendly setting value
// implies via its mapping. Empty if the setting has no core mapping.
inline std::vector<std::pair<std::string, std::string>>
resolveCoreOptionValues(const EmulationSetting &setting,
                        const std::string &friendlyValue) {
  std::vector<std::pair<std::string, std::string>> result;
  result.reserve(setting.mapping.size());
  for (const auto &m : setting.mapping) {
    const auto it = m.valueMap.find(friendlyValue);
    result.emplace_back(m.coreKey,
                        it != m.valueMap.end() ? it->second : friendlyValue);
  }
  return result;
}

} // namespace firelight::settings
