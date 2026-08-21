#include "firelight/input/keyboard_input_handler.hpp"

#include <firelight/input/shortcut_registry.hpp>

#include <QMetaEnum>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace firelight::input {

namespace {
// Qt registers Qt::Key with the meta-object system, so key names resolve without
// a table here to drift out of step with Qt
std::optional<int> keyFromName(const std::string &name) {
  static const auto meta = QMetaEnum::fromType<Qt::Key>();
  bool found = false;
  const int value = meta.keyToValue(name.c_str(), &found);
  return found ? std::optional(value) : std::nullopt;
}

std::optional<ActivationType> activationFromName(const std::string &name) {
  if (name == "press") {
    return ActivationType::Press;
  }
  if (name == "hold") {
    return ActivationType::Hold;
  }
  return std::nullopt;
}

std::optional<int> scopeFromName(const std::string &name) {
  if (name == "in-game") {
    return ScopeInGame;
  }
  if (name == "in-menu") {
    return ScopeInMenu;
  }
  if (name == "always") {
    return ScopeAlways;
  }
  return std::nullopt;
}

std::optional<DeviceType> deviceFromName(const std::string &name) {
  if (name == "gamepad") {
    return DeviceType::Gamepad;
  }
  if (name == "keyboard") {
    return DeviceType::Keyboard;
  }
  return std::nullopt;
}

const char *deviceName(const DeviceType device) { return device == DeviceType::Keyboard ? "keyboard" : "gamepad"; }
} // namespace

std::optional<InputSource> parseInputSource(const std::string_view text, const DeviceType device) {
  if (text.empty()) {
    return std::nullopt;
  }

  // "Select+R3": everything before the last '+' is held alongside the trigger
  std::vector<std::string> parts;
  size_t start = 0;
  while (start <= text.size()) {
    const auto plus = text.find('+', start);
    const auto end = plus == std::string_view::npos ? text.size() : plus;
    parts.emplace_back(text.substr(start, end - start));
    if (plus == std::string_view::npos) {
      break;
    }
    start = plus + 1;
  }

  const auto resolve = [device](const std::string &name) -> std::optional<int> {
    if (device == DeviceType::Keyboard) {
      return keyFromName(name);
    }
    const auto input = gamepadInputFromName(name);
    return input ? std::optional(static_cast<int>(*input)) : std::nullopt;
  };

  InputSource source;
  source.type = device == DeviceType::Keyboard ? SourceType::Key : SourceType::Button;

  for (size_t i = 0; i < parts.size(); ++i) {
    const auto code = resolve(parts[i]);
    if (!code) {
      return std::nullopt;
    }
    if (i + 1 == parts.size()) {
      source.code = *code;
    } else {
      source.modifiers.push_back(*code);
    }
  }
  return source;
}

bool ShortcutPreset::appliesTo(const DeviceType device) const { return bindings.contains(device); }

const std::vector<InputSource> &ShortcutPreset::sourcesFor(const DeviceType device, const ShortcutId &id) const {
  static const std::vector<InputSource> EMPTY;
  const auto deviceIt = bindings.find(device);
  if (deviceIt == bindings.end()) {
    return EMPTY;
  }
  const auto it = deviceIt->second.find(id);
  return it == deviceIt->second.end() ? EMPTY : it->second;
}

ShortcutRegistry &ShortcutRegistry::instance() {
  static ShortcutRegistry registry;
  return registry;
}

bool ShortcutRegistry::loadFromFile(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    spdlog::error("Couldn't open the shortcut catalog at {}", path);
    return false;
  }
  return loadFromString(std::string(std::istreambuf_iterator(file), {}));
}

bool ShortcutRegistry::loadFromString(const std::string &json) {
  nlohmann::json parsed = nlohmann::json::parse(json, nullptr, false);
  if (parsed.is_discarded() || !parsed.is_object()) {
    spdlog::error("The shortcut catalog isn't valid JSON");
    return false;
  }

  clear();

  for (const auto &entry : parsed.value("actions", nlohmann::json::array())) {
    ShortcutAction action;
    action.id = entry.value("id", "");
    action.displayName = entry.value("label", "");
    action.category = entry.value("category", "");

    const auto activation = activationFromName(entry.value("activation", "press"));
    if (!activation) {
      spdlog::error("Shortcut '{}' has an unknown activation '{}'", action.id, entry.value("activation", ""));
      return false;
    }
    action.activation = *activation;

    const auto scope = scopeFromName(entry.value("scope", "in-game"));
    if (!scope) {
      spdlog::error("Shortcut '{}' has an unknown scope '{}'", action.id, entry.value("scope", ""));
      return false;
    }
    action.scope = *scope;

    if (action.id.empty()) {
      spdlog::error("A shortcut action is missing its id");
      return false;
    }
    m_actions[action.id] = action;
  }

  for (const auto &entry : parsed.value("presets", nlohmann::json::array())) {
    ShortcutPreset preset;
    preset.id = entry.value("id", "");
    preset.label = entry.value("label", "");
    if (preset.id.empty()) {
      spdlog::error("A shortcut preset is missing its id");
      return false;
    }

    for (const auto &deviceKey : {"gamepad", "keyboard"}) {
      if (!entry.contains(deviceKey)) {
        continue;
      }
      const auto device = *deviceFromName(deviceKey);
      auto &target = preset.bindings[device];

      for (const auto &[id, sources] : entry.at(deviceKey).items()) {
        std::vector<InputSource> parsedSources;
        for (const auto &source : sources) {
          const auto text = source.get<std::string>();
          const auto input = parseInputSource(text, device);
          if (!input) {
            spdlog::error("Preset '{}' binds '{}' to '{}', which isn't an input "
                          "a {} has",
                          preset.id, id, text, deviceKey);
            return false;
          }
          parsedSources.push_back(*input);
        }
        target[id] = parsedSources;
      }
    }
    m_presets.push_back(preset);
  }

  m_defaultPresetId = parsed.value("defaultPreset", "");
  return true;
}

std::vector<std::string> ShortcutRegistry::validate() const {
  std::vector<std::string> problems;

  if (m_actions.empty()) {
    problems.emplace_back("the catalog declares no actions");
  }

  if (!m_defaultPresetId.empty() && !findPreset(m_defaultPresetId)) {
    problems.push_back("defaultPreset '" + m_defaultPresetId + "' isn't a declared preset");
  } else if (m_defaultPresetId.empty()) {
    problems.emplace_back("no defaultPreset, so new profiles get no bindings");
  }

  // The keys a console input falls back to during gameplay. A shipped keyboard
  // shortcut on one of these would fire the shortcut *and* press the button
  const auto &gameplayKeys = KeyboardInputHandler::defaultKeyMap();

  for (const auto &preset : m_presets) {
    for (const auto &[device, byId] : preset.bindings) {
      for (const auto &[id, sources] : byId) {
        // The engine skips ids with no action, so a typo here would just never
        // fire — with nothing logged
        if (!m_actions.contains(id)) {
          problems.push_back("preset '" + preset.id + "' binds '" + id + "', which isn't a declared action");
        }

        for (const auto &source : sources) {
          if (device != DeviceType::Keyboard) {
            continue;
          }
          const auto isGameplayKey = [&gameplayKeys](const int code) {
            return gameplayKeys.values().contains(static_cast<Qt::Key>(code));
          };
          if (isGameplayKey(source.code)) {
            problems.push_back("preset '" + preset.id + "' binds '" + id +
                               "' to a key the keyboard already uses for gameplay");
          }
        }
      }
    }
  }
  return problems;
}

void ShortcutRegistry::registerAction(const ShortcutAction &action) { m_actions[action.id] = action; }

const ShortcutAction *ShortcutRegistry::getAction(const ShortcutId &id) const {
  const auto it = m_actions.find(id);
  return it == m_actions.end() ? nullptr : &it->second;
}

std::vector<ShortcutAction> ShortcutRegistry::listActions() const {
  std::vector<ShortcutAction> result;
  result.reserve(m_actions.size());
  for (const auto &[id, action] : m_actions) {
    result.push_back(action);
  }
  return result;
}

const std::vector<ShortcutPreset> &ShortcutRegistry::presets() const { return m_presets; }

const ShortcutPreset *ShortcutRegistry::findPreset(const std::string &id) const {
  for (const auto &preset : m_presets) {
    if (preset.id == id) {
      return &preset;
    }
  }
  return nullptr;
}

const std::string &ShortcutRegistry::defaultPresetId() const { return m_defaultPresetId; }

void ShortcutRegistry::clear() {
  m_actions.clear();
  m_presets.clear();
  m_defaultPresetId.clear();
}

} // namespace firelight::input
