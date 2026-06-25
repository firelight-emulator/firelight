#pragma once
#include <cstdint>
#include <string>

namespace firelight::activity {
struct PlaySession {
  int id = -1;
  std::string contentId;
  std::string contentHash;
  unsigned int slotNumber = 1;
  uint64_t startTime = 0;
  uint64_t endTime = 0;
  uint64_t unpausedDurationMillis = 0;
};
} // namespace firelight::db