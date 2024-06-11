#pragma once

#include <vector>

struct Platform {
  int id = -1;
  std::string name;
  std::string abbreviation;
  std::string slug;
  std::vector<std::string> supportedExtensions;
  unsigned int retroAchievementsId = 0;
};
