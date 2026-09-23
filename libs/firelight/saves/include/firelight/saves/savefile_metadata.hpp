#pragma once

#include <cstdint>
#include <string>

namespace firelight::saves {

struct SavefileMetadata {
  int id = -1;
  std::string contentHash;
  unsigned int saveSlot = 1;
  std::string savefileMd5;
  int64_t lastModifiedAt = 0;
  int64_t createdAt = 0;
};
} // namespace firelight::saves
