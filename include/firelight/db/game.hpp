#pragma once
#include <string>

namespace firelight::db {
struct Game {
  int id = -1;
  std::string name;
  std::string description;
};
} // namespace firelight::db