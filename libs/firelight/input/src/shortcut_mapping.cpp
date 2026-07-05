#include <firelight/input/shortcut_mapping.hpp>

#include <nlohmann/json.hpp>

namespace firelight::input {

ShortcutMapping::ShortcutMapping(
    std::function<void(ShortcutMapping &)> syncCallback)
    : m_syncCallback(std::move(syncCallback)) {}

const std::vector<InputSource> &
ShortcutMapping::getBindings(const ShortcutId &id) const {
  static const std::vector<InputSource> EMPTY;
  const auto it = m_bindings.find(id);
  return it == m_bindings.end() ? EMPTY : it->second;
}

void ShortcutMapping::setBindings(const ShortcutId &id,
                                  std::vector<InputSource> sources) {
  if (sources.empty()) {
    m_bindings.erase(id);
  } else {
    m_bindings[id] = std::move(sources);
  }
}

void ShortcutMapping::addBinding(const ShortcutId &id,
                                 const InputSource &source) {
  m_bindings[id].push_back(source);
}

void ShortcutMapping::removeBinding(const ShortcutId &id,
                                    const std::size_t index) {
  const auto it = m_bindings.find(id);
  if (it == m_bindings.end() || index >= it->second.size()) {
    return;
  }
  it->second.erase(it->second.begin() + static_cast<std::ptrdiff_t>(index));
  if (it->second.empty()) {
    m_bindings.erase(it);
  }
}

const std::map<ShortcutId, std::vector<InputSource>> &
ShortcutMapping::getAll() const {
  return m_bindings;
}

std::string ShortcutMapping::serialize() const {
  nlohmann::json shortcuts = nlohmann::json::object();
  for (const auto &[id, sources] : m_bindings) {
    shortcuts[id] = sources;
  }

  nlohmann::json j;
  j["version"] = 1;
  j["shortcuts"] = shortcuts;
  return j.dump();
}

void ShortcutMapping::deserialize(const std::string &data) {
  m_bindings.clear();
  if (data.empty()) {
    return;
  }

  const auto j = nlohmann::json::parse(data, nullptr, false);
  if (j.is_discarded() || !j.is_object() || !j.contains("shortcuts")) {
    return; // tolerate legacy/invalid blobs
  }

  for (const auto &[id, sources] : j.at("shortcuts").items()) {
    m_bindings[id] = sources.get<std::vector<InputSource>>();
  }
}

void ShortcutMapping::sync() {
  if (m_syncCallback) {
    m_syncCallback(*this);
  }
}

} // namespace firelight::input
