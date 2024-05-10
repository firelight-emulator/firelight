#pragma once

#include <string>

namespace firelight::achievements {

struct AchievementUnlock {
  int id = -1;
  int raAchievementId = -1;
  int raGameId = -1;
  std::string title;
  std::string description;
  unsigned int points;
  int dateUnlocked;
  unsigned int createdAt = 0;
};

}; // namespace firelight::achievements