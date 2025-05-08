#pragma once
#include <cstdint>

namespace firelight::achievements {
struct UserAchievementStatus {
  int achievementId = 0;
  bool achieved = false;
  bool achievedHardcore = false;
  int64_t timestamp = 0;
  int64_t timestampHardcore = 0;
};
} // namespace firelight::achievements