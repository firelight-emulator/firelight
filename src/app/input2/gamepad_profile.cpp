#include "gamepad_profile.hpp"

namespace firelight::input {

GamepadProfile::GamepadProfile(const int id) : m_id(id) {}

int GamepadProfile::getId() const { return m_id; }
std::string GamepadProfile::getName() const { return m_name; }
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

} // namespace firelight::input