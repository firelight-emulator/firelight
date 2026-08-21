#pragma once

#include <firelight/cheats/cheat.hpp>

#include <string>
#include <vector>

namespace firelight::cheats {

// Per-game cheat storage. Kept behind an interface so the emulation layer +
// tests depend on the abstraction, not sqlite (mirrors the other repositories)
class ICheatRepository {
public:
  virtual ~ICheatRepository() = default;

  // All cheats for a game, ordered by `ordinal`
  [[nodiscard]] virtual std::vector<Cheat> getCheats(const std::string &contentHash) = 0;

  // Inserts `cheat` (appending after the game's existing cheats) and fills in
  // its assigned `id`/`ordinal`. Returns false on failure
  virtual bool addCheat(Cheat &cheat) = 0;

  virtual bool updateCheat(const Cheat &cheat) = 0;

  virtual bool setEnabled(int id, bool enabled) = 0;

  virtual bool removeCheat(int id) = 0;
};

} // namespace firelight::cheats
