#pragma once

#include <string>

namespace firelight::library {
struct EntryFolderInfo {
  int id = -1;
  std::string displayName;
  std::string description;
  uint64_t createdAt;
};
} // namespace firelight::library