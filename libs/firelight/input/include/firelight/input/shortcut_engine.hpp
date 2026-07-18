#pragma once
#include <firelight/input/shortcut_action.hpp>

#include <map>
#include <mutex>

namespace firelight::input {
class IGamepad;

// TODO
// Detects shortcut triggers from raw device input and publishes ShortcutEvents
// via the EventDispatcher. Fed by the input service (both gamepad and keyboard)
// It reads each triggering device's own profile shortcut bindings and the
// global ShortcutRegistry for each action's activation type and scope
class ShortcutEngine {
public:
  // Current context (ScopeInGame / ScopeInMenu). Actions only fire in scope
  void setContext(int scope);
  [[nodiscard]] int context() const;

  // Called for every raw input transition on a device. `code` is a GamepadInput
  // for a controller or a Qt::Key for the keyboard
  void onInput(int playerIndex, IGamepad *device, int code, bool pressed);

  // Forget a device's held state (e.g. on disconnect)
  void forgetDevice(IGamepad *device);

  // TODO
  // Whether a device's hotkeys fire at all. Off hands every input straight to
  // the game, for a core that wants the whole keyboard. Per device, so turning
  // them off on one pad leaves another's alone even on a shared profile
  [[nodiscard]] bool hotkeysEnabled(IGamepad *device) const;
  void setHotkeysEnabled(IGamepad *device, bool enabled);

private:
  [[nodiscard]] bool isSourceSatisfied(IGamepad *device,
                                       const InputSource &source) const;

  // TODO
  // Guards all state below. onInput runs on the SDL event thread (controller)
  // and the GUI thread (keyboard); setContext/forgetDevice come from other
  // threads too. ShortcutEvents are collected under the lock and published
  // after releasing it, so a subscriber can't re-enter and deadlock
  mutable std::mutex m_mutex;
  int m_context = ScopeInMenu; // no game is active until the UI says so
  std::map<IGamepad *, std::map<int, bool>> m_held;
  std::map<IGamepad *, std::map<ShortcutId, bool>> m_satisfied;
  std::map<IGamepad *, std::map<ShortcutId, bool>> m_holdActive;
  // TODO
  // Absent means on. Keyed by device rather than profile because it answers
  // "is this controller driving the menus right now", which two pads sharing a
  // profile can answer differently. (Unlike the toggle latch deleted earlier,
  // the state here really is per-device, so this is where a latch belongs.)
  std::map<IGamepad *, bool> m_hotkeysDisabled;
};

} // namespace firelight::input
