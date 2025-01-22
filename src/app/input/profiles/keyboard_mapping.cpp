#include "keyboard_mapping.hpp"

namespace firelight::input {

  void KeyboardMapping::addKeyboardMapping(libretro::IRetroPad::Input input,
                                           int mappedKeyCode) {
    m_keyboardMappings.erase(input);
    m_keyboardMappings.emplace(input, mappedKeyCode);

  }

  std::optional<int> KeyboardMapping::getMappedKeyboardInput(
      const libretro::IRetroPad::Input input) {
    if (!m_keyboardMappings.contains(input)) {
      return std::nullopt;
    }
    return {m_keyboardMappings[input]};
  }

  std::map<libretro::IRetroPad::Input, int> &KeyboardMapping::getKeyboardMappings() {
    return m_keyboardMappings;
  }

  void KeyboardMapping::removeMapping(const libretro::IRetroPad::Input input) {
    m_keyboardMappings.erase(input);
  }

  std::string KeyboardMapping::serialize() {
    // TODO
    return "";
  }

  void KeyboardMapping::deserialize(const std::string &data) {
    // TODO
  }
} // namespace firelight::input