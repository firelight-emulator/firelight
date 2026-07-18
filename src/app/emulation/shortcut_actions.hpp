#pragma once

#include "emulator_controller.hpp"

#include <firelight/input/shortcut_action.hpp>

#include <functional>
#include <string>

namespace firelight::settings {
class SettingsService;
}

namespace firelight::emulation {

// What every emulator hotkey actually does
//
// This is where the shortcut behaviour lives, rather than in QML: it is the only
// thing that knows a save slot cursor exists, that releasing fast-forward should
// restore the speed you were at rather than 1x, and that rewind is off in
// hardcore mode. Plain class, injected with everything it touches, so it can be
// tested headless with a fake controller — no QML item, no core, no window
//
// Everything it can't do itself is a call back out, so it depends on nothing but
// the emulator interface and the settings. The three that genuinely need the UI
// (opening the quick menu or the rewind menu, going fullscreen) become Qt
// signals in the caller; nothing here knows the GUI exists
class ShortcutActions {
public:
  struct Intents {
    std::function<void()> openQuickMenu;
    std::function<void()> openRewindMenu;
    std::function<void()> toggleFullscreen;
    std::function<void()> resetGame;
    std::function<void()> exitGame;
    // Says what just happened. A hotkey gives no other feedback — a silent save
    // state is indistinguishable from a binding that doesn't work
    std::function<void(const std::string &)> notify;
  };

  ShortcutActions(settings::SettingsService &settingsService, std::function<bool()> hardcoreModeActive,
                  Intents intents);

  // The emulator to act on, or nullptr when no game is running. Set by
  // EmulatorItem as it comes and goes
  void setController(IEmulatorController *controller);

  // Set after construction because the intents are wired to the dispatcher's
  // signals, and the dispatcher needs this to exist first
  void setIntents(Intents intents) { m_intents = std::move(intents); }

  // Runs one shortcut. Unknown ids are ignored, so an action can be declared in
  // the catalog before it does anything
  void handle(const input::ShortcutId &id, input::ShortcutPhase phase);

  // The slot save_state/load_state currently point at
  [[nodiscard]] int saveSlot() const { return m_saveSlot; }

private:
  void beginHold(const input::ShortcutId &id, float multiplier);
  void endHold(const input::ShortcutId &id);
  void stepSpeed(bool faster);
  // Nudges the volume setting and reports the result
  void stepVolume(int delta);
  void setSaveSlot(int slot);
  void notify(const std::string &message) const;
  // "2x" / "0.5x" for a playback multiplier
  [[nodiscard]] static std::string speedLabel(float multiplier);

  // Rewind and slow motion rewrite history or trivialise a game, so hardcore
  // mode blocks them. One gate: EmulatorItem's own speed helpers used to check
  // separately, and QML gated the rewind menu on its own
  [[nodiscard]] bool blockedByHardcore(const input::ShortcutId &id) const;

  settings::SettingsService &m_settingsService;
  std::function<bool()> m_hardcoreModeActive;
  Intents m_intents;

  IEmulatorController *m_controller = nullptr;

  int m_saveSlot = 1;
  // The speed to go back to when a hold ends. Four actions write the multiplier
  // (fast_forward, toggle_fast_forward, speed_up, slow_down) and only a hold has
  // to be undone, so the value to restore is remembered when the hold starts
  float m_speedBeforeHold = 1.0f;
  input::ShortcutId m_activeSpeedHold;
};

} // namespace firelight::emulation
