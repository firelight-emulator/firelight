#pragma once

#include <cstdint>
#include <string>

namespace firelight::db {
struct SavefileMetadata {
  uint64_t id;
  std::string contentMd5;
  uint8_t slotNumber;
  std::string savefileMd5;
  uint64_t lastModifiedAt;
  uint64_t createdAt;
};
} // namespace firelight::db