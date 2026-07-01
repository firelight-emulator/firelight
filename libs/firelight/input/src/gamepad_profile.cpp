#include <firelight/input/gamepad_profile.hpp>

namespace firelight::input {

GamepadProfile::GamepadProfile(const int id) : m_id(id) {}

int GamepadProfile::getId() const { return m_id; }
std::string GamepadProfile::getName() const { return m_name; }

bool GamepadProfile::isKeyboardProfile() const { return m_isKeyboardProfile; }

void GamepadProfile::setIsKeyboardProfile(const bool isKeyboardProfile) {
  m_isKeyboardProfile = isKeyboardProfile;
}

bool GamepadProfile::isBuiltin() const { return m_builtin; }
void GamepadProfile::setBuiltin(const bool builtin) { m_builtin = builtin; }

int GamepadProfile::getBasedOnType() const { return m_basedOnType; }
void GamepadProfile::setBasedOnType(const int type) { m_basedOnType = type; }

std::string GamepadProfile::getIcon() const { return m_icon; }
void GamepadProfile::setIcon(const std::string &icon) { m_icon = icon; }

void GamepadProfile::setName(const std::string &name) { m_name = name; }

std::shared_ptr<InputMapping>
GamepadProfile::getMappingForPlatformAndController(
    const int platformId, const int controllerType) const {

  for (const auto &mapping : m_mappings) {
    if (mapping->getPlatformId() == platformId &&
        mapping->getControllerType() == controllerType) {
      return mapping;
    }
  }
  return nullptr;
}
void GamepadProfile::addMapping(const std::shared_ptr<InputMapping> &mapping) {
  for (const auto &existingMapping : m_mappings) {
    if (existingMapping->getPlatformId() == mapping->getPlatformId() &&
        existingMapping->getControllerType() == mapping->getControllerType()) {
      return;
    }
  }

  m_mappings.emplace_back(mapping);
}

void GamepadProfile::setShortcutMapping(
    std::shared_ptr<ShortcutMapping> mapping) {
  m_shortcutMapping = std::move(mapping);
}
std::shared_ptr<ShortcutMapping> GamepadProfile::getShortcutMapping() const {
  return m_shortcutMapping;
}

const AnalogSettings &GamepadProfile::getDefaultAnalogSettings() const {
  return m_defaultAnalogSettings;
}

void GamepadProfile::setDefaultAnalogSettings(const AnalogSettings &settings) {
  m_defaultAnalogSettings = settings;
}

AnalogSettings GamepadProfile::getAnalogSettings(const int platformId,
                                                 const int controllerType) const {
  const auto mapping =
      getMappingForPlatformAndController(platformId, controllerType);
  if (mapping && mapping->getAnalogOverride().has_value()) {
    return mapping->getAnalogOverride().value();
  }
  return m_defaultAnalogSettings;
}

} // namespace firelight::input
