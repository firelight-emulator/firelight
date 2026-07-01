#pragma once
#include <firelight/input/shortcut_action.hpp>

#include <map>

namespace firelight::input {
class IGamepad;

// Detects shortcut triggers from raw device input and publishes ShortcutEvents
// via the EventDispatcher. Fed by the input service (both gamepad and keyboard).
// It reads each triggering device's own profile shortcut bindings and the
// global ShortcutRegistry for each action's activation type and scope.
class ShortcutEngine {
public:
  // Current context (ScopeInGame / ScopeInMenu). Actions only fire in scope.
  void setContext(int scope);
  [[nodiscard]] int context() const;

  // Called for every raw input transition on a device. `code` is a GamepadInput
  // for a controller or a Qt::Key for the keyboard.
  void onInput(int playerIndex, IGamepad *device, int code, bool pressed);

  // Forget a device's held/latched state (e.g. on disconnect).
  void forgetDevice(IGamepad *device);

private:
  [[nodiscard]] bool isSourceSatisfied(IGamepad *device,
                                       const InputSource &source) const;

  int m_context = ScopeInMenu; // no game is active until the UI says so
  std::map<IGamepad *, std::map<int, bool>> m_held;
  std::map<IGamepad *, std::map<ShortcutId, bool>> m_satisfied;
  std::map<IGamepad *, std::map<ShortcutId, bool>> m_holdActive;
  std::map<IGamepad *, std::map<ShortcutId, bool>> m_toggleLatch;
};

} // namespace firelight::input
