#pragma once
#include <firelight/input/shortcut_action.hpp>

#include <map>
#include <vector>

namespace firelight::input {

// Global catalog of the shortcut actions the emulator supports. The app
// registers actions at startup; the ShortcutEngine and the configuration UI
// read from it. Trigger bindings are stored separately, per profile.
class ShortcutRegistry {
public:
  static ShortcutRegistry &instance();

  void registerAction(const ShortcutAction &action);
  [[nodiscard]] const ShortcutAction *getAction(const ShortcutId &id) const;
  [[nodiscard]] std::vector<ShortcutAction> listActions() const;
  void clear();

private:
  std::map<ShortcutId, ShortcutAction> m_actions;
};

} // namespace firelight::input
