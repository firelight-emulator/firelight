#pragma once

#include <string>
#include <stdint.h>

namespace firelight::db {
  struct SuspendPointMetadata {
    int id = -1;
    std::string contentId;
    unsigned int saveSlotNumber = 0;
    unsigned int slotNumber = 0;
    uint64_t lastModifiedAt = 0;
    uint64_t createdAt = 0;
    bool locked = false;
  };
} // namespace firelight::db
