#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace firelight::cheats {

// How a cheat's code is interpreted. This picks the decoder and, indirectly,
// how it's applied: most formats resolve to RAM writes Firelight owns, while
// Game Genie is ROM read-substitution that only the core can do (retro_cheat_set).
enum class CheatType {
  Memory = 0,    // raw "address:value[:size]"; Firelight RAM poke
  RetroArch = 1, // imported from a RetroArch .cht entry
  GameGenie = 2, // core-applied (ROM substitution)
  GameShark = 3, // GameShark / Action Replay; Firelight RAM poke
};

// One resolved write into emulated RAM: `size` bytes of `value` at `address`.
struct CheatPoke {
  uint32_t address = 0;
  uint32_t value = 0;
  uint8_t size = 1; // 1, 2, or 4 bytes
  bool bigEndian = false;
};

// A cheat as Firelight models it, uniformly across formats: user-facing metadata
// plus a resolved application. `pokes` are the RAM writes Firelight replays each
// frame; a cheat with no pokes but a `rawCode` is handed to the core (Game
// Genie). Keyed to a game by content hash, ordered by `ordinal`.
struct Cheat {
  int id = 0;
  std::string contentHash;
  std::string name;
  CheatType type = CheatType::Memory;
  std::string rawCode;          // original text (display / re-decode / core code)
  std::vector<CheatPoke> pokes; // resolved RAM writes (empty for core-applied)
  bool enabled = false;
  bool affectsHardcore = true;  // blocked while RA hardcore mode is active
  int ordinal = 0;

  // True when Firelight can't apply this itself and must hand the raw code to
  // the core (Game Genie / an emu-handler .cht entry): no pokes, but a code.
  [[nodiscard]] bool isCoreApplied() const {
    return pokes.empty() && !rawCode.empty();
  }
};

} // namespace firelight::cheats
