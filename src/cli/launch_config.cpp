#include "cli/launch_config.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

namespace firelight::cli {

namespace {

std::string trim(const std::string &s) {
  const auto begin = s.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return {};
  }
  const auto end = s.find_last_not_of(" \t\r\n");
  return s.substr(begin, end - begin + 1);
}

std::string toLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

// Converts a JSON scalar to the string form the settings layer stores. Objects
// and arrays are rejected (a setting value is always a scalar)
std::string jsonValueToString(const nlohmann::json &value) {
  if (value.is_string()) {
    return value.get<std::string>();
  }
  if (value.is_boolean()) {
    return value.get<bool>() ? "true" : "false";
  }
  if (value.is_number() || value.is_null()) {
    return value.dump(); // e.g. "120", "1.5", "null"
  }
  throw std::runtime_error(
      "settings-file JSON values must be strings, numbers, or booleans");
}

std::vector<OverridePair> parseJson(const std::string &contents) {
  nlohmann::json json;
  try {
    json = nlohmann::json::parse(contents);
  } catch (const nlohmann::json::exception &e) {
    throw std::runtime_error(std::string("invalid JSON settings-file: ") +
                             e.what());
  }
  if (!json.is_object()) {
    throw std::runtime_error("settings-file JSON must be an object of key/value "
                             "pairs");
  }
  std::vector<OverridePair> result;
  for (const auto &[key, value] : json.items()) {
    result.emplace_back(key, jsonValueToString(value));
  }
  return result;
}

std::vector<OverridePair> parseProperties(const std::string &contents) {
  std::vector<OverridePair> result;
  std::istringstream stream(contents);
  std::string line;
  while (std::getline(stream, line)) {
    const auto trimmed = trim(line);
    if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') {
      continue;
    }
    const auto eq = trimmed.find('=');
    if (eq == std::string::npos) {
      throw std::runtime_error("malformed settings-file line (expected "
                               "key=value): " +
                               trimmed);
    }
    const auto key = trim(trimmed.substr(0, eq));
    const auto value = trim(trimmed.substr(eq + 1));
    if (key.empty()) {
      throw std::runtime_error("settings-file line has an empty key: " +
                               trimmed);
    }
    result.emplace_back(key, value);
  }
  return result;
}

} // namespace

std::vector<OverridePair> loadOverrideFile(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    throw std::runtime_error("could not open settings-file: " + path);
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  const auto contents = buffer.str();

  const auto trimmed = trim(contents);
  if (!trimmed.empty() && trimmed[0] == '{') {
    return parseJson(trimmed);
  }
  return parseProperties(contents);
}

std::optional<GamepadType> parseControllerType(const std::string &name) {
  static const std::map<std::string, GamepadType> ALIASES = {
      {"keyboard", KEYBOARD},
      {"xbox360", MICROSOFT_XBOX_360},
      {"xbox-360", MICROSOFT_XBOX_360},
      {"xboxone", MICROSOFT_XBOX_ONE},
      {"xbox-one", MICROSOFT_XBOX_ONE},
      {"xbox", MICROSOFT_XBOX_ONE},
      {"dualshock3", SONY_DUALSHOCK_3},
      {"ds3", SONY_DUALSHOCK_3},
      {"dualshock4", SONY_DUALSHOCK_4},
      {"ds4", SONY_DUALSHOCK_4},
      {"dualsense", SONY_DUALSENSE},
      {"ds5", SONY_DUALSENSE},
      {"switch-pro", NINTENDO_SWITCH_PRO},
      {"switchpro", NINTENDO_SWITCH_PRO},
      {"pro", NINTENDO_SWITCH_PRO},
      {"nso-n64", NINTENDO_NSO_N64},
      {"nso-snes", NINTENDO_NSO_SNES},
      {"nso-genesis", NINTENDO_NSO_GENESIS},
  };
  const auto it = ALIASES.find(toLower(name));
  if (it == ALIASES.end()) {
    return std::nullopt;
  }
  return it->second;
}

std::string controllerTypeNames() {
  return "keyboard, xbox360, xboxone, dualshock3, dualshock4, dualsense, "
         "switch-pro, nso-n64, nso-snes, nso-genesis";
}

} // namespace firelight::cli
