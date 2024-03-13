#pragma once

#include <cstdint>
#include <string>

namespace firelight::db {
struct SavefileMetadata {
  int64_t id = -1;
  std::string contentMd5;
  uint8_t slotNumber;
  std::string savefileMd5;
  uint64_t lastModifiedAt;
  uint64_t createdAt;
};
} // namespace firelight::db