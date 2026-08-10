#pragma once

#include <cstdint>
#include <string>

namespace firelight::saves {
struct SavefileInfo {
  bool hasData = false;
  std::string filePath;
  std::string contentHash;
  int saveSlot = -1;
  std::string savefileMd5;
  std::string name;
  std::string description;
  // Epoch milliseconds, like every other timestamp in the app
  int64_t lastModifiedAt = 0;
};
} // namespace firelight::saves
