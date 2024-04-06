#pragma once
#include <vector>

namespace firelight::db {
struct GameRelease {
  int id = -1;
  int gameId = -1;
  int platformId = -1;
  int region_id = -1;
  std::vector<int> romIds;
};
} // namespace firelight::db