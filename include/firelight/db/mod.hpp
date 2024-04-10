#pragma once

#include <string>

namespace firelight::db {
struct Mod {
  int id = -1;
  std::string name;
  std::string primaryAuthor;
  int gameReleaseId;
  std::string imageSource;
  std::string description;
};
} // namespace firelight::db
