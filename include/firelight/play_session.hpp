#pragma once
#include <string>

namespace firelight::db {
struct PlaySession {
  int id = -1;
  std::string contentId;
  unsigned int slotNumber = 1;
  unsigned int startTime = 0;
  unsigned int endTime = 0;
  unsigned int unpausedDurationMillis = 0;
};
} // namespace firelight::db