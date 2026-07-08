#pragma once

#include <cstdint>
#include <string>

namespace firelight::netplay {

// Opaque member identity assigned by the lobby provider. Never interpreted.
using PlayerId = uint64_t;

inline constexpr int PROTOCOL_VERSION = 1;
inline constexpr int MAX_SLOTS = 8;

// Reject codes carried in Reject{code}.
inline const std::string REJECT_PROTOCOL_MISMATCH = "protocol-mismatch";
inline const std::string REJECT_LOBBY_FULL = "lobby-full";

// Generates a shareable lobby join code (FL-XXXX-XXXX, Crockford base32).
[[nodiscard]] std::string generateJoinCode();

} // namespace firelight::netplay
