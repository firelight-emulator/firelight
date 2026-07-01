#pragma once
#include <firelight/input/input_source.hpp>

#include <string>
#include <vector>

namespace firelight::input {

// Stable identifier for an emulator action (e.g. "fast_forward", "save_state").
// A string keeps the catalog extensible and the serialized form readable.
using ShortcutId = std::string;

// How a shortcut behaves when its trigger is pressed/released.
enum class ActivationType {
  Press,  // fires once on the rising edge
  Hold,   // active while held (Started on press, Ended on release)
  Toggle, // press flips a latch (Started with the new toggledState)
};

// Where a shortcut is allowed to fire (bit flags).
enum ShortcutScope {
  ScopeInGame = 1,
  ScopeInMenu = 2,
  ScopeAlways = ScopeInGame | ScopeInMenu,
};

// A global catalog entry: what an action is and how it behaves. Defined by the
// app; the trigger bindings live per-profile.
struct ShortcutAction {
  ShortcutId id;
  std::string displayName;
  std::string category;
  ActivationType activation = ActivationType::Press;
  int scope = ScopeInGame;
  std::vector<InputSource> defaults; // shipped default triggers
};

enum class ShortcutPhase { Started, Ended };

// Emitted by the ShortcutEngine when a shortcut fires.
struct ShortcutEvent {
  int playerIndex = -1;
  ShortcutId id;
  ShortcutPhase phase = ShortcutPhase::Started;
  bool toggledState = false; // meaningful for Toggle actions
};

} // namespace firelight::input
