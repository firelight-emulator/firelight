#include "input_mapping.hpp"

#include "input/gamepad_input.hpp"

namespace firelight::input {
InputMapping::InputMapping(const int id, const int profileId, int platformId,
                           const int controllerType,
                           std::function<void(InputMapping &)> syncCallback)
    : m_syncCallback(std::move(syncCallback)), m_id(id),
      m_controllerProfileId(profileId), m_platformId(platformId),
      m_controllerType(controllerType) {}

void InputMapping::addMapping(GamepadInput input, GamepadInput mappedInput) {
  m_mappings.erase(input);
  m_mappings.emplace(input, mappedInput);
}

std::optional<GamepadInput>
InputMapping::getMappedInput(const GamepadInput input) {
  if (!m_mappings.contains(input)) {
    return std::nullopt;
  }
  return {m_mappings[input]};
}

std::map<GamepadInput, GamepadInput> &InputMapping::getMappings() {
  return m_mappings;
}

void InputMapping::removeMapping(const GamepadInput input) {
  m_mappings.erase(input);
}

std::string InputMapping::serialize() {
  auto result = std::string();

  // Serialization format is from:to,from:to,from:to etc
  for (const auto &[input, mappedInput] : m_mappings) {
    result += std::to_string(input) + ":" + std::to_string(mappedInput) + ",";
  }
  return result;
}

void InputMapping::deserialize(const std::string &data) {
  m_mappings.clear();

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
    m_mappings.emplace(static_cast<GamepadInput>(input),
                       static_cast<GamepadInput>(mappedInput));
    start = end + 1;
  }
}

void InputMapping::sync() {
  if (m_syncCallback) {
    m_syncCallback(*this);
  }
}

unsigned InputMapping::getId() const { return m_id; }

unsigned InputMapping::getControllerProfileId() const {
  return m_controllerProfileId;
}

unsigned InputMapping::getPlatformId() const { return m_platformId; }
unsigned InputMapping::getControllerType() const { return m_controllerType; }

void InputMapping::setId(const unsigned id) { m_id = id; }

void InputMapping::setControllerProfileId(const unsigned controllerProfileId) {
  m_controllerProfileId = controllerProfileId;
}

void InputMapping::setPlatformId(const unsigned platformId) {
  m_platformId = platformId;
}
void InputMapping::setControllerType(const int controllerType) {
  m_controllerType = controllerType;
}
} // namespace firelight::input
