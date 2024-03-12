#pragma once
#include <cstdint>
#include <string>

namespace firelight::db {
struct PlaySession {
  int64_t id;
  std::string contentMd5;
  uint16_t slotNumber;
  uint64_t startTime;
  uint64_t endTime;
  uint16_t unpausedDurationSeconds;
};
} // namespace firelight::db