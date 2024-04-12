#pragma once
#include <string>

namespace firelight::db {
struct Game {
  int id = -1;
  std::string name;
  std::string slug;
  std::string description;
  int platformId = -1;
};
} // namespace firelight::db