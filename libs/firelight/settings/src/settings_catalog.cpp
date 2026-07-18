#include "firelight/settings/settings_catalog.hpp"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <set>
#include <spdlog/spdlog.h>
#include <sstream>

namespace firelight::settings {

SettingsCatalog &SettingsCatalog::instance() {
  static SettingsCatalog s;
  return s;
}

namespace {

void parseConditions(const nlohmann::json &j, const char *field,
                     std::vector<SettingCondition> &out) {
  if (!j.contains(field)) {
    return;
  }
  for (const auto &c : j[field]) {
    SettingCondition cond;
    cond.key = c.value("key", std::string{});
    if (c.contains("values")) {
      for (const auto &v : c["values"]) {
        cond.values.push_back(v.get<std::string>());
      }
    }
    out.push_back(std::move(cond));
  }
}

void parseStringArray(const nlohmann::json &j, const char *field,
                      std::vector<std::string> &out) {
  if (!j.contains(field)) {
    return;
  }
  for (const auto &v : j[field]) {
    if (v.is_string()) {
      out.push_back(v.get<std::string>());
    }
  }
}

// Author-friendly `type` aliases. Each implies a widget; an explicit `widget`
// overrides it. Value semantics collapse to the SettingType set
bool applyTypeAlias(const std::string &typeStr, SettingDefinition &s) {
  if (typeStr == "boolean" || typeStr == "toggle") {
    s.type = SettingType::BOOLEAN;
    s.widget = "toggle";
  } else if (typeStr == "integer" || typeStr == "slider") {
    s.type = SettingType::INTEGER;
    s.widget = "slider";
  } else if (typeStr == "number" || typeStr == "spinbox") {
    s.type = SettingType::INTEGER;
    s.widget = "spinbox";
  } else if (typeStr == "stepper") {
    s.type = SettingType::INTEGER;
    s.widget = "stepper";
  } else if (typeStr == "custom") {
    s.type = SettingType::CUSTOM;
  } else if (typeStr == "game-picker") {
    // A dropdown whose options are the user's eligible library games (filled in
    // by the app layer at runtime); value semantics are OPTIONS (the chosen
    // game's content hash)
    s.type = SettingType::OPTIONS;
    s.widget = "dropdown";
    s.libraryGameSource = true;
  } else if (typeStr == "audio-device") {
    // A dropdown of the machine's audio outputs, filled in by the app layer at
    // runtime (the settings lib knows nothing about hardware)
    s.type = SettingType::OPTIONS;
    s.widget = "dropdown";
    s.audioDeviceSource = true;
  } else if (typeStr == "text") {
    s.type = SettingType::STRING;
    s.widget = "text";
  } else if (typeStr == "color") {
    s.type = SettingType::STRING;
    s.widget = "color";
  } else if (typeStr == "file" || typeStr == "file-picker") {
    s.type = SettingType::STRING;
    s.widget = "file-picker";
  } else if (typeStr == "folder" || typeStr == "folder-picker" ||
             typeStr == "directory") {
    s.type = SettingType::STRING;
    s.widget = "folder-picker";
    s.directoryMode = true;
  } else if (typeStr == "multi-select" || typeStr == "multiselect") {
    // A checklist over `options`; the value is a JSON array of selected option
    // values (serialized/parsed in the UI delegate)
    s.type = SettingType::OPTIONS;
    s.widget = "multi-select";
  } else if (typeStr == "segmented") {
    s.type = SettingType::OPTIONS;
    s.widget = "segmented";
  } else if (typeStr == "radio") {
    s.type = SettingType::OPTIONS;
    s.widget = "radio";
  } else if (typeStr == "key-binding") {
    s.type = SettingType::STRING;
    s.widget = "key-binding";
  } else if (typeStr == "options" || typeStr == "dropdown") {
    s.type = SettingType::OPTIONS;
    s.widget = "dropdown";
  } else {
    s.type = SettingType::OPTIONS;
    s.widget = "dropdown";
    return false;
  }
  return true;
}

SettingDefinition parseSetting(const nlohmann::json &j,
                               std::vector<std::string> &problems) {
  SettingDefinition s;
  s.key = j.value("key", std::string{});
  s.label = j.value("label", std::string{});
  s.description = j.value("description", std::string{});
  s.defaultValue = j.value("default", std::string{});
  s.groupId = j.value("group", std::string{});
  s.order = j.value("order", 0);
  s.requiresRestart = j.value("requiresRestart", false);
  s.advanced = j.value("advanced", false);
  s.trueStringValue = j.value("trueValue", std::string("true"));
  s.falseStringValue = j.value("falseValue", std::string("false"));
  parseStringArray(j, "keywords", s.keywords);

  const auto typeStr = j.value("type", std::string("options"));
  if (!applyTypeAlias(typeStr, s)) {
    problems.push_back("setting '" + s.key + "': unknown type '" + typeStr +
                       "' (fell back to dropdown)");
  }
  s.widget = j.value("widget", s.widget);
  s.minValue = j.value("min", 0.0);
  s.maxValue = j.value("max", 0.0);
  s.stepValue = j.value("step", 1.0);

  if (j.contains("options")) {
    for (const auto &o : j["options"]) {
      s.options.push_back(
          {o.value("label", std::string{}), o.value("value", std::string{})});
    }
  }

  if (j.contains("eligiblePlatformIds")) {
    for (const auto &p : j["eligiblePlatformIds"]) {
      if (p.is_number_integer()) {
        s.gamePickerPlatformIds.push_back(p.get<int>());
      }
    }
  }

  s.placeholder = j.value("placeholder", std::string{});
  parseStringArray(j, "extensions", s.fileExtensions);
  // A file-picker can opt into directory mode explicitly, too
  s.directoryMode = j.value("directory", s.directoryMode);

  if (j.contains("mapping")) {
    for (const auto &m : j["mapping"]) {
      CoreOptionMapping mapping;
      mapping.coreKey = m.value("coreKey", std::string{});
      if (m.contains("values")) {
        for (auto it = m["values"].begin(); it != m["values"].end(); ++it) {
          mapping.valueMap[it.key()] = it.value().get<std::string>();
        }
      }
      s.mapping.push_back(std::move(mapping));
    }
  }

  parseConditions(j, "visibleWhen", s.visibleWhen);
  parseConditions(j, "enabledWhen", s.enabledWhen);
  return s;
}

// App settings are single-valued and never reach a core, so the core-facing
// fields are meaningless there. Strip them rather than let them look load-
// bearing
SettingDefinition parseAppSetting(const nlohmann::json &j,
                                  std::vector<std::string> &problems) {
  auto s = parseSetting(j, problems);
  for (const char *field : {"mapping", "trueValue", "falseValue"}) {
    if (j.contains(field)) {
      problems.push_back("app setting '" + s.key + "': '" + field +
                         "' is only meaningful for emulation settings (ignored)");
    }
  }
  s.mapping.clear();
  s.trueStringValue = "true";
  s.falseStringValue = "false";
  return s;
}

SettingsPage parsePage(const nlohmann::json &j) {
  SettingsPage p;
  p.id = j.value("id", std::string{});
  p.label = j.value("label", std::string{});
  p.icon = j.value("icon", std::string{});
  p.route = j.value("route", std::string{});
  p.order = j.value("order", 0);
  parseStringArray(j, "keywords", p.keywords);
  return p;
}

SettingsGroup parseGroup(const nlohmann::json &j) {
  SettingsGroup g;
  g.id = j.value("id", std::string{});
  g.pageId = j.value("page", std::string{});
  g.label = j.value("label", std::string{});
  g.order = j.value("order", 0);
  return g;
}

void sortByOrder(std::vector<SettingDefinition> &settings) {
  std::stable_sort(settings.begin(), settings.end(),
                   [](const SettingDefinition &a, const SettingDefinition &b) {
                     return a.order < b.order;
                   });
}

} // namespace

bool SettingsCatalog::loadFromJson(const std::string &json) {
  try {
    const auto root = nlohmann::json::parse(json);

    std::vector<SettingsPage> pages;
    std::vector<SettingsGroup> groups;
    std::vector<SettingDefinition> app;
    std::vector<SettingDefinition> common;
    std::map<std::string, std::vector<SettingDefinition>> perCore;
    std::map<std::string, std::map<std::string, std::string>> coreDefaults;
    std::vector<std::string> problems;

    if (root.contains("pages")) {
      for (const auto &p : root["pages"]) {
        pages.push_back(parsePage(p));
      }
    }
    if (root.contains("groups")) {
      for (const auto &g : root["groups"]) {
        groups.push_back(parseGroup(g));
      }
    }
    if (root.contains("app")) {
      for (const auto &s : root["app"]) {
        app.push_back(parseAppSetting(s, problems));
      }
    }
    if (root.contains("common")) {
      for (const auto &s : root["common"]) {
        common.push_back(parseSetting(s, problems));
      }
    }
    if (root.contains("cores")) {
      for (auto it = root["cores"].begin(); it != root["cores"].end(); ++it) {
        const auto &coreName = it.key();
        const auto &core = it.value();
        if (core.contains("settings")) {
          for (const auto &s : core["settings"]) {
            perCore[coreName].push_back(parseSetting(s, problems));
          }
        }
        if (core.contains("defaults")) {
          for (auto d = core["defaults"].begin(); d != core["defaults"].end();
               ++d) {
            coreDefaults[coreName][d.key()] = d.value().get<std::string>();
          }
        }
      }
    }

    std::stable_sort(pages.begin(), pages.end(),
                     [](const SettingsPage &a, const SettingsPage &b) {
                       return a.order < b.order;
                     });
    std::stable_sort(groups.begin(), groups.end(),
                     [](const SettingsGroup &a, const SettingsGroup &b) {
                       return a.order < b.order;
                     });

    m_pages = std::move(pages);
    m_groups = std::move(groups);
    m_app = std::move(app);
    m_common = std::move(common);
    m_perCore = std::move(perCore);
    m_coreDefaults = std::move(coreDefaults);

    for (const auto &problem : problems) {
      spdlog::warn("Settings catalog: {}", problem);
    }
    for (const auto &problem : validate()) {
      spdlog::warn("Settings catalog: {}", problem);
    }
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to parse settings catalog: {}", e.what());
    return false;
  }
}

bool SettingsCatalog::loadFromFile(const std::string &path) {
  std::ifstream stream(path);
  if (!stream.is_open()) {
    spdlog::error("Failed to open settings catalog file: {}", path);
    return false;
  }
  std::stringstream buffer;
  buffer << stream.rdbuf();
  return loadFromJson(buffer.str());
}

std::vector<std::string> SettingsCatalog::validate() const {
  std::vector<std::string> problems;

  std::set<std::string> pageIds;
  for (const auto &page : m_pages) {
    if (page.id.empty()) {
      problems.push_back("a page has no id");
    } else if (!pageIds.insert(page.id).second) {
      problems.push_back("duplicate page id '" + page.id + "'");
    }
  }

  std::set<std::string> groupIds;
  for (const auto &group : m_groups) {
    if (group.id.empty()) {
      problems.push_back("a group has no id");
      continue;
    }
    if (!groupIds.insert(group.id).second) {
      problems.push_back("duplicate group id '" + group.id + "'");
    }
    if (!group.pageId.empty() && !pageIds.count(group.pageId)) {
      problems.push_back("group '" + group.id + "' names undeclared page '" +
                         group.pageId + "'");
    }
  }

  std::set<std::string> keys;
  const auto checkSetting = [&](const SettingDefinition &s,
                                const std::string &where) {
    if (s.key.empty()) {
      problems.push_back("a setting in " + where + " has no key");
      return;
    }
    if (!keys.insert(s.key).second) {
      problems.push_back("duplicate setting key '" + s.key + "' (" + where +
                         ")");
    }
    if (!s.groupId.empty() && !groupIds.count(s.groupId)) {
      problems.push_back("setting '" + s.key + "' names undeclared group '" +
                         s.groupId + "'");
    }
    if (s.type == SettingType::CUSTOM && s.widget.empty()) {
      problems.push_back("setting '" + s.key +
                         "' is custom but names no widget");
    }
    // Runtime-sourced options are authored empty on purpose
    if (s.type == SettingType::OPTIONS && s.options.empty() &&
        !s.libraryGameSource && !s.audioDeviceSource) {
      problems.push_back("setting '" + s.key + "' has no options");
    }
  };

  for (const auto &s : m_app) {
    checkSetting(s, "app");
  }
  for (const auto &s : m_common) {
    checkSetting(s, "common");
  }
  for (const auto &[coreName, settings] : m_perCore) {
    for (const auto &s : settings) {
      checkSetting(s, "core " + coreName);
    }
  }

  return problems;
}

const SettingsPage *SettingsCatalog::findPage(const std::string &id) const {
  const auto it = std::find_if(
      m_pages.begin(), m_pages.end(),
      [&](const SettingsPage &p) { return p.id == id; });
  return it != m_pages.end() ? &*it : nullptr;
}

const SettingsGroup *SettingsCatalog::findGroup(const std::string &id) const {
  const auto it = std::find_if(
      m_groups.begin(), m_groups.end(),
      [&](const SettingsGroup &g) { return g.id == id; });
  return it != m_groups.end() ? &*it : nullptr;
}

const std::vector<SettingDefinition> &
SettingsCatalog::coreSpecificSettings(const std::string &coreName) const {
  static const std::vector<SettingDefinition> empty;
  const auto it = m_perCore.find(coreName);
  return it != m_perCore.end() ? it->second : empty;
}

const std::map<std::string, std::string> &
SettingsCatalog::coreDefaults(const std::string &coreName) const {
  static const std::map<std::string, std::string> empty;
  const auto it = m_coreDefaults.find(coreName);
  return it != m_coreDefaults.end() ? it->second : empty;
}

std::vector<SettingDefinition>
SettingsCatalog::settingsForCore(const std::string &coreName) const {
  std::vector<SettingDefinition> result = m_common;
  const auto &specific = coreSpecificSettings(coreName);
  result.insert(result.end(), specific.begin(), specific.end());
  return result;
}

std::vector<SettingDefinition>
SettingsCatalog::settingsForGroup(const std::string &groupId,
                                  const std::string &coreName) const {
  std::vector<SettingDefinition> result;
  if (groupId.empty()) {
    return result;
  }
  const auto collect = [&](const std::vector<SettingDefinition> &from) {
    for (const auto &s : from) {
      if (s.groupId == groupId) {
        result.push_back(s);
      }
    }
  };
  collect(m_app);
  collect(m_common);
  if (!coreName.empty()) {
    collect(coreSpecificSettings(coreName));
  }
  sortByOrder(result);
  return result;
}

const SettingDefinition *
SettingsCatalog::findByKey(const std::string &key) const {
  for (const auto *s : allSettings()) {
    if (s->key == key) {
      return s;
    }
  }
  return nullptr;
}

std::vector<const SettingDefinition *> SettingsCatalog::allSettings() const {
  std::vector<const SettingDefinition *> result;
  result.reserve(m_app.size() + m_common.size());
  for (const auto &s : m_app) {
    result.push_back(&s);
  }
  for (const auto &s : m_common) {
    result.push_back(&s);
  }
  for (const auto &[coreName, settings] : m_perCore) {
    for (const auto &s : settings) {
      result.push_back(&s);
    }
  }
  return result;
}

bool SettingsCatalog::isAppSetting(const std::string &key) const {
  return std::any_of(
      m_app.begin(), m_app.end(),
      [&](const SettingDefinition &s) { return s.key == key; });
}

std::string SettingsCatalog::defaultForCommonKey(const std::string &key) const {
  for (const auto &setting : m_common) {
    if (setting.key == key) {
      return setting.defaultValue;
    }
  }
  return {};
}

} // namespace firelight::settings
