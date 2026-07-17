#pragma once

#include <firelight/settings/setting_definition.hpp>

#include <map>
#include <string>
#include <vector>

namespace firelight::settings {

// Declarative catalog of every setting Firelight has: what it is, where it
// lives (page + group), and — for emulation settings — how it reaches the core.
// Authored as JSON (see the schema in data/settings_catalog.json).
//
// Three arrays, and which one a setting is authored in decides how it's read:
//   `app`    — frontend settings (appearance, general). Single-valued: no core
//              mapping, read only at the global tier.
//   `common` — emulation concepts (rewind, aspect ratio, ...) that apply to
//              every core. Tiered global/platform/game.
//   `cores`  — keyed by CORE, not platform, because core option keys differ per
//              core; a platform run by a different core gets a different set.
//              Each `cores.<coreName>` has `settings` (friendly) and `defaults`
//              (Firelight's opinionated raw core-option overrides). Tiered.
//
// `pages` and `groups` say where rows appear and in what order; both kinds of
// setting reference them the same way.
class SettingsCatalog {
public:
  // The one catalog shared across the app (loaded once at startup, read by the
  // emulation path and the settings UI). Starts empty; tests that don't load it
  // just see an empty catalog (callers fall back to core-declared defaults).
  static SettingsCatalog &instance();

  // Replaces the catalog from a JSON document / file. Returns false (and logs)
  // on a parse error, leaving the previous catalog intact. Validation problems
  // are logged but do not fail the load — a bad entry shouldn't cost the user
  // every other setting.
  bool loadFromJson(const std::string &json);
  bool loadFromFile(const std::string &path);

  // Authoring mistakes the parser can't otherwise catch: duplicate keys, a
  // setting pointing at a group that doesn't exist, a group pointing at a
  // missing page, a CUSTOM setting with no widget. Empty => the catalog is
  // sound. Page and group ids are load-bearing, so a typo silently orphans a
  // setting without this.
  [[nodiscard]] std::vector<std::string> validate() const;

  [[nodiscard]] const std::vector<SettingsPage> &pages() const {
    return m_pages;
  }
  [[nodiscard]] const std::vector<SettingsGroup> &groups() const {
    return m_groups;
  }
  [[nodiscard]] const SettingsPage *findPage(const std::string &id) const;
  [[nodiscard]] const SettingsGroup *findGroup(const std::string &id) const;

  [[nodiscard]] const std::vector<SettingDefinition> &appSettings() const {
    return m_app;
  }
  [[nodiscard]] const std::vector<SettingDefinition> &commonSettings() const {
    return m_common;
  }
  [[nodiscard]] const std::vector<SettingDefinition> &
  coreSpecificSettings(const std::string &coreName) const;
  // Firelight's default overrides for a core's raw options (coreKey -> value).
  [[nodiscard]] const std::map<std::string, std::string> &
  coreDefaults(const std::string &coreName) const;
  [[nodiscard]] const std::map<std::string, std::vector<SettingDefinition>> &
  allCoreSettings() const {
    return m_perCore;
  }

  // common settings followed by the core-specific ones.
  [[nodiscard]] std::vector<SettingDefinition>
  settingsForCore(const std::string &coreName) const;

  // Every setting declared in the group, ordered by `order` then declaration
  // order. App and common settings always apply; core-specific ones only for
  // `coreName`, so an empty coreName means "no core in scope" (the global tier)
  // rather than "every core's settings at once".
  [[nodiscard]] std::vector<SettingDefinition>
  settingsForGroup(const std::string &groupId,
                   const std::string &coreName = {}) const;

  // The declared setting for a key, or nullptr. Keys are unique across all
  // three arrays (validate() reports collisions).
  [[nodiscard]] const SettingDefinition *
  findByKey(const std::string &key) const;

  // Every declared setting — the source the search index is built from.
  // Pointers are owned by the catalog and invalidated by the next load.
  [[nodiscard]] std::vector<const SettingDefinition *> allSettings() const;

  // Whether a key is an app setting (single-valued, global-tier-only).
  [[nodiscard]] bool isAppSetting(const std::string &key) const;

  // The declared default for a common (frontend) setting, or "" if the catalog
  // doesn't define it. The single source of truth for these defaults — callers
  // resolve overrides first, then fall back to this.
  [[nodiscard]] std::string defaultForCommonKey(const std::string &key) const;

private:
  std::vector<SettingsPage> m_pages;
  std::vector<SettingsGroup> m_groups;
  std::vector<SettingDefinition> m_app;
  std::vector<SettingDefinition> m_common;
  std::map<std::string, std::vector<SettingDefinition>> m_perCore;
  std::map<std::string, std::map<std::string, std::string>> m_coreDefaults;
};

} // namespace firelight::settings
