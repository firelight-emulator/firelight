#pragma once
#include "achievement.hpp"

#include <string>
#include <vector>

namespace firelight::achievements {
struct AchievementSet {
  unsigned id;
  std::string name;
  std::string iconUrl;
  unsigned numAchievements;
  unsigned totalPoints;

  // Read-only, populated from database
  std::vector<Achievement> achievements;
};
} // namespace firelight::achievements