#pragma once

#include <cstdint>
#include <string>

namespace firelight::saves {
// Index row for a suspend point (save state). The state bytes live on disk;
// this row carries the lock flag and timestamps (int64 milliseconds)
struct SuspendPointMetadata {
  int id = -1;
  std::string contentId;
  int saveSlotNumber = 0;
  unsigned int slotNumber = 0;
  int64_t lastModifiedAt = 0;
  int64_t createdAt = 0;
  bool locked = false;
};
} // namespace firelight::saves
