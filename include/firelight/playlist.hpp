#pragma once

#include <string>

namespace firelight::db {
struct Playlist {
  int id = -1;
  std::string displayName;
};

} // namespace firelight::db