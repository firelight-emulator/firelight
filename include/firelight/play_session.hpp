#pragma once
#include <string>

namespace firelight::db {
struct PlaySession {
  int id = -1;
  std::string contentMd5;
  unsigned int slotNumber = 1;
  unsigned int startTime = 0;
  unsigned int endTime = 0;
  unsigned int unpausedDurationSeconds = 0;
};
} // namespace firelight::db