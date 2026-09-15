#pragma once

#include <firelight/settings/setting_definition.hpp>

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace firelight::settings {

// TODO
// Declarative catalog of every setting Firelight has: what it is, where it
// renders, and — for emulation settings — how it reaches the core
// Authored as JSON across any number of files
//
// Three arrays, and which one a setting is authored in decides how it's read:
//   `app`    — frontend settings (appearance, general). Single-valued: no core
//              mapping, read only at the global tier
//   `common` — emulation concepts (rewind, aspect ratio, ...) that apply to
//              every core. Tiered global/platform/game
//   `cores`  — keyed by CORE, not platform, because core option keys differ per
//              core; a platform run by a different core gets a different set
//              Each `cores.<coreName>` has `settings` (friendly) and `defaults`
//              (Firelight's opinionated raw core-option overrides). Tiered
//
// Layout is declared top-down: each of `pages` lists its group ids under
// `groups`, and each of `groups` lists its setting keys under `settings`, both
// in render order
class SettingsCatalog {
public:
  // TODO
  /** Creates an empty catalog */
  SettingsCatalog() = default;

  // TODO
  /** Not copyable */
  SettingsCatalog(const SettingsCatalog &) = delete;

  // TODO
  /** Not copyable */
  SettingsCatalog &operator=(const SettingsCatalog &) = delete;

  // The one catalog shared across the app (loaded once at startup, read by the
  // emulation path and the settings UI). Starts empty; tests that don't load it
  // just see an empty catalog (callers fall back to core-declared defaults)
  static SettingsCatalog &instance();

  // Replaces the catalog from a JSON document / file. Returns false (and logs)
  // on a parse error, leaving the previous catalog intact. Validation problems
  // are logged but do not fail the load — a bad entry shouldn't cost the user
  // every other setting
  bool loadFromJson(const std::string &json);
  bool loadFromFile(const std::string &path);

  // TODO
  // Replaces the catalog from every .json under a directory, recursively, read in sorted path order.
  // Each file is the same shape as a whole catalog carrying only the pages, groups and settings it
  // needs
  //
  // One unreadable or unparseable file fails the whole load and leaves the previous catalog intact:
  // a half-loaded catalog gives every missing key an empty default, which is indistinguishable from
  // a setting the user never touched
  bool loadFromDirectory(const std::string &path);

  // TODO
  // Authoring mistakes the parser can't otherwise catch: empty or duplicate page, group and setting
  // ids, a page listing an undeclared group, a group listed more than once, a group listing an
  // undeclared setting, a setting listed twice in one group, a CUSTOM setting with no widget, a link with
  // no route, and options with nothing to pick. Empty => the catalog is sound
  [[nodiscard]] std::vector<std::string> validate() const;

  [[nodiscard]] const std::vector<SettingsPage> &pages() const { return m_contents.pages; }

  [[nodiscard]] const std::vector<SettingsGroup> &groups() const { return m_contents.groups; }

  [[nodiscard]] const SettingsPage *findPage(const std::string &id) const;
  [[nodiscard]] const SettingsGroup *findGroup(const std::string &id) const;

  // TODO
  /** The first group that lists the setting, or nullptr */
  [[nodiscard]] const SettingsGroup *findGroupForSetting(const std::string &key) const;

  // TODO
  /** The page that lists the group, or nullptr */
  [[nodiscard]] const SettingsPage *findPageForGroup(const std::string &groupId) const;

  [[nodiscard]] const std::vector<SettingDefinition> &appSettings() const { return m_contents.app; }

  [[nodiscard]] const std::vector<SettingDefinition> &commonSettings() const { return m_contents.common; }

  [[nodiscard]] const std::vector<SettingDefinition> &coreSpecificSettings(const std::string &coreName) const;
  // Firelight's default overrides for a core's raw options (coreKey -> value)
  [[nodiscard]] const std::map<std::string, std::string> &coreDefaults(const std::string &coreName) const;

  [[nodiscard]] const std::map<std::string, std::vector<SettingDefinition>> &allCoreSettings() const {
    return m_contents.perCore;
  }

  // common settings followed by the core-specific ones
  [[nodiscard]] std::vector<SettingDefinition> settingsForCore(const std::string &coreName) const;

  // TODO
  // The settings the group lists, in list order, skipping keys nothing declares. App and common
  // settings always apply; core-specific ones only for `coreName`, so an empty coreName means "no core
  // in scope" (the global tier) rather than "every core's settings at once"
  [[nodiscard]] std::vector<SettingDefinition> settingsForGroup(const std::string &groupId,
                                                                const std::string &coreName = {}) const;

  // The declared setting for a key, or nullptr. Keys are unique across all
  // three arrays (validate() reports collisions)
  [[nodiscard]] const SettingDefinition *findByKey(const std::string &key) const;

  // Every declared setting — the source the search index is built from
  // Pointers are owned by the catalog and invalidated by the next load
  [[nodiscard]] std::vector<const SettingDefinition *> allSettings() const;

  // Whether a key is an app setting (single-valued, global-tier-only)
  [[nodiscard]] bool isAppSetting(const std::string &key) const;

  // The declared default for a common (frontend) setting, or "" if the catalog
  // doesn't define it. The single source of truth for these defaults — callers
  // resolve overrides first, then fall back to this
  [[nodiscard]] std::string defaultForCommonKey(const std::string &key) const;

private:
  // TODO
  /** Everything a load declares */
  struct Contents {
    std::vector<SettingsPage> pages;
    std::vector<SettingsGroup> groups;
    std::vector<SettingDefinition> app;
    std::vector<SettingDefinition> common;
    std::map<std::string, std::vector<SettingDefinition>> perCore;
    std::map<std::string, std::map<std::string, std::string>> coreDefaults;
  };

  // TODO
  // One document's worth of catalog, before anything is committed. Several files accumulate into
  // one of these so a load either lands whole or not at all
  struct Accumulator {
    Contents contents;
    std::vector<std::string> problems;
  };

  // TODO
  /** A declared setting and the array it was authored in. `coreName` is empty for app and common settings */
  struct IndexedSetting {
    const SettingDefinition *definition = nullptr;
    std::string coreName;
    bool isApp = false;
  };

  // TODO
  // Appends one document to the accumulator. Throws whatever nlohmann throws on malformed JSON, so
  // the caller decides whether one bad file costs the whole load
  static void parseInto(const std::string &json, Accumulator &into, const std::string &sourceName);

  // TODO
  // Takes ownership, builds the lookups and logs whatever the parse and validation turned up
  bool commit(Accumulator &&accumulated);

  // TODO
  /** Clears and refills every lookup from the contents. On a duplicate id or key, the first declaration wins */
  void buildLookups();

  Contents m_contents;
  std::unordered_map<std::string, IndexedSetting> m_settingsByKey;
  std::unordered_map<std::string, const SettingsPage *> m_pagesById;
  std::unordered_map<std::string, const SettingsGroup *> m_groupsById;
  std::unordered_map<std::string, const SettingsGroup *> m_groupsBySettingKey;
  std::unordered_map<std::string, const SettingsPage *> m_pagesByGroupId;
};

} // namespace firelight::settings
