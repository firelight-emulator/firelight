#pragma once
#include "achievement.hpp"

#include <string>
#include <vector>

namespace firelight::achievements {
struct AchievementSet {
  unsigned id;
  std::string title;
  std::string type;
  unsigned gameId;
  std::string imageIconUrl;

  unsigned numAchievements;
  unsigned totalPoints;

  // Read-only, populated from database
  std::vector<Achievement> achievements;
};
} // namespace firelight::achievements