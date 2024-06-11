#pragma once
#include <string>

namespace firelight::db {
struct Region {
  int id = -1;
  std::string name;
  std::string abbreviation;
  std::string slug;
};
} // namespace firelight::db
