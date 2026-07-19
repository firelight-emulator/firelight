#pragma once

#include <nlohmann/json.hpp>

namespace firelight::achievements {
struct AwardAchievementResponse {
  bool Success;
  unsigned Score;
  unsigned SoftcoreScore;
  unsigned AchievementID;
  unsigned AchievementsRemaining;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AwardAchievementResponse, Success, Score, SoftcoreScore, AchievementID,
                                   AchievementsRemaining)
} // namespace firelight::achievements
