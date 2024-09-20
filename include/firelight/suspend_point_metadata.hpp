#pragma once

#include <string>

namespace firelight::db {
struct SuspendPointMetadata {
  int id = -1;
  std::string contentId;
  unsigned int saveSlotNumber = 0;
  unsigned int slotNumber = 0;
  unsigned int lastModifiedAt = 0;
  unsigned int createdAt = 0;
  bool locked = false;
};
} // namespace firelight::db