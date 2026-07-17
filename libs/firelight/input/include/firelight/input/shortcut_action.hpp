#pragma once
#include <firelight/input/input_source.hpp>

#include <string>
#include <vector>

namespace firelight::input {

// Stable identifier for an emulator action (e.g. "fast_forward", "save_state").
// A string keeps the catalog extensible and the serialized form readable.
using ShortcutId = std::string;

// The one action the engine itself handles, because it is about the device
// rather than the emulator: it turns that device's other hotkeys off, and has
// to keep working while they are, or there'd be no way back.
inline constexpr auto TOGGLE_HOTKEYS_ID = "toggle_hotkeys";

// How a shortcut behaves when its trigger is pressed/released.
//
// There is deliberately no Toggle: an action like pause or mute toggles state
// that is global to the emulator, but the engine only ever sees one device, so
// a latch here would be per-device and two devices would drift apart. The
// engine reports the edge; whoever owns the state flips it.
enum class ActivationType {
  Press, // fires once on the rising edge
  Hold,  // active while held (Started on press, Ended on release)
};

// Where a shortcut is allowed to fire (bit flags).
enum ShortcutScope {
  ScopeInGame = 1,
  ScopeInMenu = 2,
  ScopeAlways = ScopeInGame | ScopeInMenu,
};

// A global catalog entry: what an action is and how it behaves. Defined by the
// app; the trigger bindings live per-profile.
//
// Deliberately carries no defaults: the shipped bindings live in presets, which
// seed a profile once at creation. An action having its own default list was
// how this started out, and nothing ever filled it in — every profile shipped
// unbound. Bindings belong to the profile, and only there.
struct ShortcutAction {
  ShortcutId id;
  std::string displayName;
  std::string category;
  ActivationType activation = ActivationType::Press;
  int scope = ScopeInGame;
};

enum class ShortcutPhase { Started, Ended };

// Emitted by the ShortcutEngine when a shortcut fires.
struct ShortcutEvent {
  int playerIndex = -1;
  ShortcutId id;
  ShortcutPhase phase = ShortcutPhase::Started;
};

// One device turned its own hotkeys off or back on. Separate from ShortcutEvent
// because it reports state rather than an edge, which only makes sense here: the
// kill switch genuinely is per-device, unlike pause or mute. Worth announcing —
// a hotkey mode nobody can see is how a "my controller is broken" report starts.
struct HotkeysToggledEvent {
  int playerIndex = -1;
  bool enabled = true;
};

} // namespace firelight::input
