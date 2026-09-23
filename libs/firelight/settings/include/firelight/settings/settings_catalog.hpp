#pragma once

#include <firelight/settings/setting_definition.hpp>

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace firelight::settings {

// Declarative catalog of every setting Firelight has
class SettingsCatalog {
public:
  SettingsCatalog() = default;

  // Only allow one to exist, and only through instance()
  SettingsCatalog(const SettingsCatalog &) = delete;
  SettingsCatalog &operator=(const SettingsCatalog &) = delete;

  // The one catalog shared across the app
  static SettingsCatalog &instance();

  bool loadFromJson(const std::string &json);
  bool loadFromFile(const std::string &path);

  // Replaces the catalog from every .json under a directory, recursively, read in sorted path order
  bool loadFromDirectory(const std::string &path);

  // Authoring mistakes the parser can't otherwise catch: empty or duplicate page, group and setting
  // ids, a page listing an undeclared group, a group listed more than once, a group listing an
  // undeclared setting, a setting listed twice in one group, a CUSTOM setting with no widget, a link with
  // no route, and options with nothing to pick. Empty -> the catalog is sound
  [[nodiscard]] std::vector<std::string> validate() const;

  [[nodiscard]] const std::vector<SettingsPage> &pages() const { return m_contents.pages; }

  [[nodiscard]] const std::vector<SettingsGroup> &groups() const { return m_contents.groups; }

  [[nodiscard]] const SettingsPage *findPage(const std::string &id) const;
  [[nodiscard]] const SettingsGroup *findGroup(const std::string &id) const;

  [[nodiscard]] const SettingsGroup *findGroupForSetting(const std::string &key) const;

  [[nodiscard]] const SettingsPage *findPageForGroup(const std::string &groupId) const;

  [[nodiscard]] const std::vector<SettingDefinition> &appSettings() const { return m_contents.app; }

  [[nodiscard]] const std::vector<SettingDefinition> &commonSettings() const { return m_contents.common; }

  [[nodiscard]] const std::vector<SettingDefinition> &coreSpecificSettings(const std::string &coreName) const;

  [[nodiscard]] const std::map<std::string, std::string> &coreDefaults(const std::string &coreName) const;

  [[nodiscard]] const std::map<std::string, std::vector<SettingDefinition>> &allCoreSettings() const {
    return m_contents.perCore;
  }

  // All applicable settings for a core (including common)
  [[nodiscard]] std::vector<SettingDefinition> settingsForCore(const std::string &coreName) const;

  [[nodiscard]] std::vector<SettingDefinition> settingsForGroup(const std::string &groupId,
                                                                const std::string &coreName = {}) const;

  // The declared setting for a key, or nullptr
  [[nodiscard]] const SettingDefinition *findByKey(const std::string &key) const;

  [[nodiscard]] std::vector<const SettingDefinition *> allSettings() const;

  // Whether a key is an app setting (single-valued, global-tier-only)
  [[nodiscard]] bool isAppSetting(const std::string &key) const;

  // The declared default for a common (frontend) setting, or "" if the catalog doesn't define it
  [[nodiscard]] std::string defaultForCommonKey(const std::string &key) const;

private:
  struct Contents {
    std::vector<SettingsPage> pages;
    std::vector<SettingsGroup> groups;
    std::vector<SettingDefinition> app;
    std::vector<SettingDefinition> common;
    std::map<std::string, std::vector<SettingDefinition>> perCore;
    std::map<std::string, std::map<std::string, std::string>> coreDefaults;
  };

  // Populated as files are read, then passed to commit() to build the lookups and log any problems
  struct Accumulator {
    Contents contents;
    std::vector<std::string> problems;
  };

  struct IndexedSetting {
    const SettingDefinition *definition = nullptr;
    std::string coreName;
    bool isApp = false;
  };

  // Appends one document to the accumulator
  static void parseInto(const std::string &json, Accumulator &into, const std::string &sourceName);

  // Takes ownership, builds the lookups and logs whatever the parse and validation turned up
  bool commit(Accumulator &&accumulated);


  // Clears and rebuilds the lookup tables for pages, groups and settings by key
  void buildLookups();

  Contents m_contents;
  std::unordered_map<std::string, IndexedSetting> m_settingsByKey;
  std::unordered_map<std::string, const SettingsPage *> m_pagesById;
  std::unordered_map<std::string, const SettingsGroup *> m_groupsById;
  std::unordered_map<std::string, const SettingsGroup *> m_groupsBySettingKey;
  std::unordered_map<std::string, const SettingsPage *> m_pagesByGroupId;
};

} // namespace firelight::settings
