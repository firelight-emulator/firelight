#pragma once
#include <string>
#include <vector>

namespace firelight::db {
struct ModRelease {
  int id = -1;
  std::string version;
  int modId = -1;
  std::vector<int> patchIds;
};
} // namespace firelight::db
