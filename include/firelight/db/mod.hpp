#pragma once

#include <string>
#include <vector>

namespace firelight::db {
struct Mod {
  int id = -1;
  std::string name;
  std::string primaryAuthor;
  std::vector<int> gameReleaseIds;
  std::string imageSource;
  std::string description;
};
} // namespace firelight::db
