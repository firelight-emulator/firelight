#include "keyboard_mapping.hpp"

namespace firelight::input {

KeyboardMapping::KeyboardMapping(
    const std::function<void(InputMapping &)> &syncCallback)
    : InputMapping(0, 0, 0, 0, syncCallback) {
  m_keyboardMappings.clear();
}

void KeyboardMapping::addKeyboardMapping(GamepadInput input,
                                         int mappedKeyCode) {
  m_keyboardMappings.erase(input);
  m_keyboardMappings.emplace(input, mappedKeyCode);
}

std::optional<int>
KeyboardMapping::getMappedKeyboardInput(const GamepadInput input) {
  if (!m_keyboardMappings.contains(input)) {
    return std::nullopt;
  }
  return {m_keyboardMappings[input]};
}

std::map<GamepadInput, int> &KeyboardMapping::getKeyboardMappings() {
  return m_keyboardMappings;
}

void KeyboardMapping::removeMapping(const GamepadInput input) {
  m_keyboardMappings.erase(input);
}

std::string KeyboardMapping::serialize() {
  auto result = std::string();

  // Serialization format is from:to,from:to,from:to etc
  for (const auto &[input, mappedInput] : m_keyboardMappings) {
    result += std::to_string(input) + ":" + std::to_string(mappedInput) + ",";
  }
  return result;
}

void KeyboardMapping::deserialize(const std::string &data) {
  m_keyboardMappings.clear();

  std::string::size_type start = 0;
  std::string::size_type end = 0;
  while ((end = data.find(',', start)) != std::string::npos) {
    auto mapping = data.substr(start, end - start);
    const auto split = mapping.find(':');
    if (split == std::string::npos) {
      continue;
    }
    auto input = std::stoi(mapping.substr(0, split));
    auto mappedInput = std::stoi(mapping.substr(split + 1));
    m_keyboardMappings.emplace(static_cast<GamepadInput>(input), mappedInput);
    start = end + 1;
  }
}
} // namespace firelight::input