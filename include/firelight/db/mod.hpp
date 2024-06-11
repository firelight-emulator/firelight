#pragma once

#include <string>

namespace firelight::db {
struct Mod {
  int id = -1;
  std::string name;
  std::string slug;
  int gameId = -1;
  std::string description;
  std::string imageSource;
  std::string primaryAuthor;
};
} // namespace firelight::db
