#pragma once

#include <cstdint>
#include <string>

namespace firelight::saves {
struct SavefileInfo {
  bool hasData = false;
  std::string filePath;
  std::string contentHash;
  int slotNumber = -1;
  std::string savefileMd5;
  std::string name;
  std::string description;
  int64_t lastModifiedEpochSeconds = 0;
};
} // namespace firelight::saves