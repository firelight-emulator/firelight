#pragma once

#include <firelight/cheats/cheat.hpp>

#include <vector>

namespace libretro {
class ICore;
}

namespace firelight::cheats {

// Firelight's own equivalent of RetroArch's "retro" cheat handler: it replays a
// set of RAM writes into the core's live system memory every emulated frame, so
// values the game overwrites are continuously re-applied (GameShark / Action
// Replay / raw memory / retro-handler .cht cheats). Core-applied cheats (Game
// Genie) are handled separately via ICore::setCheat
class CheatEngine {
public:
  // Replaces the active write set (typically the enabled RAM cheats' pokes)
  void setActivePokes(std::vector<CheatPoke> pokes);
  void clear();
  [[nodiscard]] bool empty() const { return m_pokes.empty(); }

  // Writes every active poke into the core's system RAM, bounds-checked. A no-op
  // when there are no pokes or the core exposes no writable system RAM
  void apply(::libretro::ICore &core) const;

private:
  std::vector<CheatPoke> m_pokes;
};

} // namespace firelight::cheats
