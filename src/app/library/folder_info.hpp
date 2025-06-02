#pragma once

#include <string>

namespace firelight::library {
struct FolderInfo {
  int id = -1;
  std::string displayName;
  std::string description;
  std::string iconSourceUrl;
  uint64_t createdAt;
};
} // namespace firelight::library