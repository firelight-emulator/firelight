#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace firelight::settings {

// Value semantics of a setting. The concrete UI control is chosen separately
// (see SettingDefinition::widget) so, e.g., an INTEGER can render as a slider or
// a spinbox and a CUSTOM setting can use a bespoke delegate. STRING is a
// free-form string value (text field, color, file/folder path)
enum class SettingType { BOOLEAN, OPTIONS, INTEGER, STRING, CUSTOM };

struct SettingOption {
  std::string label;
  std::string value;
};

// A dependency clause: holds when the setting identified by `key` currently has
// one of `values`. Multiple clauses on a setting are AND-ed
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
  return true; // no conditions => always holds
}

// Maps a friendly emulation setting onto one libretro core option. A friendly
// setting may carry several of these (composite: one friendly control drives
// multiple core options)
struct CoreOptionMapping {
  std::string coreKey;
  // friendlyValue -> coreValue. Empty means identity: the friendly value is
  // written to the core option unchanged
  std::map<std::string, std::string> valueMap;
};

// One declared setting: what it is and where it lives. App settings and
// emulation settings share this shape — an app setting is simply one with no
// core mapping, read only at the global tier. Which array a setting is authored
// in decides that, not a field here
struct SettingDefinition {
  std::string label;
  std::string key;
  std::string description;
  std::string defaultValue;
  SettingType type = SettingType::OPTIONS;
  // The SettingsGroup this row belongs to; the group in turn names the page
  // Empty => the setting still resolves and still indexes, but no group-filtered
  // view renders it
  std::string groupId;
  // Terms search should match beyond label/description: the words users actually
  // think in ("vsync" for Sync method)
  std::vector<std::string> keywords;
  // Sort position within the group; ties keep declaration order
  int order = 0;
  bool requiresRestart = false;
  std::string trueStringValue = "true";
  std::string falseStringValue = "false";
  std::vector<SettingOption> options;
  // INTEGER numeric range (slider / spinbox)
  double minValue = 0.0;
  double maxValue = 0.0;
  double stepValue = 1.0;
  // UI control. Empty => derived from `type` (toggle / dropdown / slider). Set
  // to a specific control ("spinbox") or, for CUSTOM, a bespoke QML delegate id
  // (e.g. "gbc-palette"). Keeps custom widgets open-ended without schema churn
  std::string widget;
  // Hidden by default; only shown when the user enables "Show advanced settings"
  // Purely a UI concern — does not affect value resolution
  bool advanced = false;
  // Empty => a frontend-only setting (consumed by EmulatorInstance) or a
  // setting whose `key` is itself the core option key (identity). Non-empty =>
  // an explicit friendly->core mapping applied when composing core values
  std::vector<CoreOptionMapping> mapping;
  // When true, this setting's `options` are not authored here but built at
  // runtime from the user's library (the app layer fills them, since the
  // settings lib has no library dependency). The stored value is the chosen
  // game's content hash. See `gamePickerPlatformIds`
  bool libraryGameSource = false;
  // Eligible platform ids for a library-game-source setting (empty => any
  // platform). e.g. {1, 2} for Game Boy / Game Boy Color
  std::vector<int> gamePickerPlatformIds;
  // Like libraryGameSource, but the options are the machine's audio output
  // devices, enumerated at runtime by the app layer. The stored value is the
  // device description, or "" for the system default
  bool audioDeviceSource = false;
  // Placeholder text for a `text` widget (shown when empty)
  std::string placeholder;
  // Accepted file extensions for a `file-picker` widget (no dot, e.g. "gba");
  // empty => any file
  std::vector<std::string> fileExtensions;
  // A `folder-picker` (true) picks a directory rather than a file
  bool directoryMode = false;
  // Relationships to other settings. Empty => always visible/enabled
  std::vector<SettingCondition> visibleWhen;
  std::vector<SettingCondition> enabledWhen;
};

// A settings page: one entry in the settings nav, one route
struct SettingsPage {
  std::string id;
  std::string label;
  std::string icon;
  std::string route;
  int order = 0;
  // Terms that should match the page itself, distinct from its settings'
  std::vector<std::string> keywords;
};

// A titled group of rows within a page — the section card the rows render into
struct SettingsGroup {
  std::string id;
  std::string pageId;
  std::string label;
  int order = 0;
};

// Whether a setting should be shown / editable given the current values of the
// settings it depends on (resolver returns another setting's effective value)
inline bool settingIsVisible(const SettingDefinition &setting, const SettingValueResolver &resolve) {
  return conditionsHold(setting.visibleWhen, resolve);
}

inline bool settingIsEnabled(const SettingDefinition &setting, const SettingValueResolver &resolve) {
  return conditionsHold(setting.enabledWhen, resolve);
}

// Resolves the concrete (coreKey, coreValue) pairs a friendly setting value
// implies via its mapping. Empty if the setting has no core mapping
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
