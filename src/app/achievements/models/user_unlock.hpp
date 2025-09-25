#pragma once
#include <cstdint>
#include <string>

namespace firelight::achievements {
struct UserUnlock {
  std::string username;
  unsigned achievementId;
  bool earned;
  bool earnedHardcore;
  uint64_t unlockTimestamp;
  uint64_t unlockTimestampHardcore;
  bool synced;
  bool syncedHardcore;
};
} // namespace firelight::achievements