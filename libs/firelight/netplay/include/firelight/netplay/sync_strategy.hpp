#pragma once

namespace firelight::netplay {

// How gameplay is kept in sync across the lobby. HostStream = the host runs
// the game and streams audio/video; guests send inputs. Lockstep = every
// machine runs the same content and exchanges per-frame inputs (later)
enum class SyncStrategyKind { HostStream, Lockstep };

} // namespace firelight::netplay
