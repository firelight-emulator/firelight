#pragma once
#include <cstdint>
#include <string>

namespace firelight::activity {
struct PlaySession {
  int id = -1;
  std::string contentHash;
  unsigned int saveSlot = 1;
  uint64_t startedAt = 0; // Epoch milliseconds
  uint64_t endedAt = 0;
  uint64_t unpausedDurationMillis = 0;
};
} // namespace firelight::activity
