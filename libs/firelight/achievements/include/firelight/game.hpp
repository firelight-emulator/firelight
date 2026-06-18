#pragma once

#include "achievement_set.hpp"

#include <string>
#include <vector>

namespace firelight::achievements {
struct Game {
  unsigned id;
  std::string title;
  std::string imageIconUrl;
  unsigned richPresenceGameId;
  std::string richPresencePatch;
  unsigned consoleId;

  std::vector<AchievementSet> achievementSets;
};
} // namespace firelight::achievements
