#pragma once
#include <string>

namespace firelight::achievements {
struct AchievementProgress {
  std::string username;
  unsigned achievementId;
  unsigned numerator;
  unsigned denominator;
};
} // namespace firelight::achievements