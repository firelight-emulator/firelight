#pragma once

#include <string>

namespace firelight::db {
struct SavefileMetadata {
  int id = -1;
  std::string contentId;
  unsigned int slotNumber = 1;
  std::string savefileMd5;
  unsigned int lastModifiedAt = 0;
  unsigned int createdAt = 0;
};
} // namespace firelight::db