#include <firelight/input/input_mapping.hpp>

#include <nlohmann/json.hpp>

namespace firelight::input {
InputMapping::InputMapping(const int id, const int profileId, int platformId,
                           const int controllerType,
                           std::function<void(InputMapping &)> syncCallback)
    : m_syncCallback(std::move(syncCallback)), m_id(id),
      m_controllerProfileId(profileId), m_platformId(platformId),
      m_controllerType(controllerType) {}

const std::vector<Binding> &
InputMapping::getBindings(const GamepadInput input) const {
  static const std::vector<Binding> kEmpty;
  const auto it = m_bindings.find(input);
  return it == m_bindings.end() ? kEmpty : it->second;
}

void InputMapping::setBindings(const GamepadInput input,
                               std::vector<Binding> bindings) {
  if (bindings.empty()) {
    m_bindings.erase(input);
  } else {
    m_bindings[input] = std::move(bindings);
  }
}

void InputMapping::addBinding(const GamepadInput input, const Binding &binding) {
  m_bindings[input].push_back(binding);
}

void InputMapping::removeBinding(const GamepadInput input,
                                 const std::size_t index) {
  const auto it = m_bindings.find(input);
  if (it == m_bindings.end() || index >= it->second.size()) {
    return;
  }
  it->second.erase(it->second.begin() + static_cast<std::ptrdiff_t>(index));
  if (it->second.empty()) {
    m_bindings.erase(it);
  }
}

const std::map<GamepadInput, std::vector<Binding>> &
InputMapping::getAllBindings() const {
  return m_bindings;
}

const std::optional<AnalogSettings> &InputMapping::getAnalogOverride() const {
  return m_analogOverride;
}

void InputMapping::setAnalogOverride(std::optional<AnalogSettings> settings) {
  m_analogOverride = std::move(settings);
}

void InputMapping::addMapping(const GamepadInput input, const int mappedInput) {
  Binding binding;
  binding.source.type = SourceType::Button;
  binding.source.code = mappedInput;
  m_bindings[input] = {binding};
}

std::optional<int> InputMapping::getMappedInput(const GamepadInput input) {
  const auto it = m_bindings.find(input);
  if (it == m_bindings.end() || it->second.empty()) {
    return std::nullopt;
  }
  return it->second.front().source.code;
}

void InputMapping::removeMapping(const GamepadInput input) {
  m_bindings.erase(input);
}

std::string InputMapping::serialize() {
  nlohmann::json bindings = nlohmann::json::object();
  for (const auto &[input, list] : m_bindings) {
    bindings[std::to_string(static_cast<int>(input))] = list;
  }

  nlohmann::json j;
  j["version"] = 3;
  j["bindings"] = bindings;
  if (m_analogOverride.has_value()) {
    j["analog"] = *m_analogOverride;
  }
  return j.dump();
}

void InputMapping::deserialize(const std::string &data) {
  m_bindings.clear();
  m_analogOverride.reset();
  if (data.empty()) {
    return;
  }

  const auto j = nlohmann::json::parse(data, nullptr, false);
  // Tolerate legacy/invalid blobs (clean break: unparseable data resets to
  // defaults rather than throwing).
  if (j.is_discarded() || !j.is_object() || !j.contains("bindings")) {
    return;
  }

  for (const auto &[key, value] : j.at("bindings").items()) {
    m_bindings[static_cast<GamepadInput>(std::stoi(key))] =
        value.get<std::vector<Binding>>();
  }
  if (j.contains("analog")) {
    m_analogOverride = j.at("analog").get<AnalogSettings>();
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
