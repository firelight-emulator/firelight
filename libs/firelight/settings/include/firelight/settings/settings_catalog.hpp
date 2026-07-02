#pragma once

#include <firelight/settings/emulation_setting.hpp>

#include <map>
#include <string>
#include <vector>

namespace firelight::settings {

// Declarative catalog of friendly emulation settings — their labels/options,
// their core-option mappings, and their inter-setting dependencies (conditional
// visible/enabled). Authored as JSON (see the schema in loadFromJson).
//
// Settings are keyed by CORE, not platform: core option keys differ per core,
// so a platform run by a different core in the future gets a different set.
// `common` settings are frontend concepts (rewind, aspect ratio, ...) that
// apply to every core. Each `cores.<coreName>` entry has `settings` (friendly)
// and `defaults` (Firelight's opinionated raw core-option default overrides).
class SettingsCatalog {
public:
  // The one catalog shared across the app (loaded once at startup, read by the
  // emulation path and the settings UI). Starts empty; tests that don't load it
  // just see an empty catalog (callers fall back to core-declared defaults).
  static SettingsCatalog &instance();

  // Replaces the catalog from a JSON document / file. Returns false (and logs)
  // on a parse error, leaving the previous catalog intact.
  bool loadFromJson(const std::string &json);
  bool loadFromFile(const std::string &path);

  [[nodiscard]] const std::vector<EmulationSetting> &commonSettings() const {
    return m_common;
  }
  [[nodiscard]] const std::vector<EmulationSetting> &
  coreSpecificSettings(const std::string &coreName) const;
  // Firelight's default overrides for a core's raw options (coreKey -> value).
  [[nodiscard]] const std::map<std::string, std::string> &
  coreDefaults(const std::string &coreName) const;

  // common settings followed by the core-specific ones.
  [[nodiscard]] std::vector<EmulationSetting>
  settingsForCore(const std::string &coreName) const;

  // The declared default for a common (frontend) setting, or "" if the catalog
  // doesn't define it. The single source of truth for these defaults — callers
  // resolve overrides first, then fall back to this.
  [[nodiscard]] std::string defaultForCommonKey(const std::string &key) const;

private:
  std::vector<EmulationSetting> m_common;
  std::map<std::string, std::vector<EmulationSetting>> m_perCore;
  std::map<std::string, std::map<std::string, std::string>> m_coreDefaults;
};

} // namespace firelight::settings
