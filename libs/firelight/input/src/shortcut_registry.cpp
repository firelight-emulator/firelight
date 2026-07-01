#include <firelight/input/shortcut_registry.hpp>

namespace firelight::input {

ShortcutRegistry &ShortcutRegistry::instance() {
  static ShortcutRegistry registry;
  return registry;
}

void ShortcutRegistry::registerAction(const ShortcutAction &action) {
  m_actions[action.id] = action;
}

const ShortcutAction *
ShortcutRegistry::getAction(const ShortcutId &id) const {
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

void ShortcutRegistry::clear() { m_actions.clear(); }

} // namespace firelight::input
