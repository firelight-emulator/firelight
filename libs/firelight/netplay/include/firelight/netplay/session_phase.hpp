#pragma once

namespace firelight::netplay {

// Sub-state of the game running inside a lobby. The lobby itself has no
// phases — it exists from creation until the host closes it, and games start
// and end within it.
enum class GamePhase { Idle, Starting, InGame };

} // namespace firelight::netplay
