#include "firelight/settings/settings_catalog.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
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

EmulationSetting parseSetting(const nlohmann::json &j) {
  EmulationSetting s;
  s.key = j.value("key", std::string{});
  s.label = j.value("label", std::string{});
  s.category = j.value("category", std::string{});
  s.description = j.value("description", std::string{});
  s.defaultValue = j.value("default", std::string{});
  s.requiresRestart = j.value("requiresRestart", false);
  s.trueStringValue = j.value("trueValue", std::string("true"));
  s.falseStringValue = j.value("falseValue", std::string("false"));

  // `type` names are author-friendly and imply a default widget; an explicit
  // `widget` overrides. Value semantics collapse to BOOLEAN/OPTIONS/INTEGER/
  // CUSTOM.
  const auto typeStr = j.value("type", std::string("options"));
  if (typeStr == "boolean" || typeStr == "toggle") {
    s.type = BOOLEAN;
    s.widget = "toggle";
  } else if (typeStr == "integer" || typeStr == "slider") {
    s.type = INTEGER;
    s.widget = "slider";
  } else if (typeStr == "number" || typeStr == "spinbox") {
    s.type = INTEGER;
    s.widget = "spinbox";
  } else if (typeStr == "custom") {
    s.type = CUSTOM;
  } else {
    s.type = OPTIONS;
    s.widget = "dropdown";
  }
  s.widget = j.value("widget", s.widget);
  s.minValue = j.value("min", 0.0);
  s.maxValue = j.value("max", 0.0);
  s.stepValue = j.value("step", 1.0);

  if (j.contains("options")) {
    for (const auto &o : j["options"]) {
      s.options.push_back({o.value("label", std::string{}),
                           o.value("value", std::string{})});
    }
  }

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

} // namespace

bool SettingsCatalog::loadFromJson(const std::string &json) {
  try {
    const auto root = nlohmann::json::parse(json);

    std::vector<EmulationSetting> common;
    std::map<std::string, std::vector<EmulationSetting>> perCore;
    std::map<std::string, std::map<std::string, std::string>> coreDefaults;

    if (root.contains("common")) {
      for (const auto &s : root["common"]) {
        common.push_back(parseSetting(s));
      }
    }
    if (root.contains("cores")) {
      for (auto it = root["cores"].begin(); it != root["cores"].end(); ++it) {
        const auto &coreName = it.key();
        const auto &core = it.value();
        if (core.contains("settings")) {
          for (const auto &s : core["settings"]) {
            perCore[coreName].push_back(parseSetting(s));
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

    m_common = std::move(common);
    m_perCore = std::move(perCore);
    m_coreDefaults = std::move(coreDefaults);
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

const std::vector<EmulationSetting> &
SettingsCatalog::coreSpecificSettings(const std::string &coreName) const {
  static const std::vector<EmulationSetting> empty;
  const auto it = m_perCore.find(coreName);
  return it != m_perCore.end() ? it->second : empty;
}

const std::map<std::string, std::string> &
SettingsCatalog::coreDefaults(const std::string &coreName) const {
  static const std::map<std::string, std::string> empty;
  const auto it = m_coreDefaults.find(coreName);
  return it != m_coreDefaults.end() ? it->second : empty;
}

std::vector<EmulationSetting>
SettingsCatalog::settingsForCore(const std::string &coreName) const {
  std::vector<EmulationSetting> result = m_common;
  const auto &specific = coreSpecificSettings(coreName);
  result.insert(result.end(), specific.begin(), specific.end());
  return result;
}

} // namespace firelight::settings
