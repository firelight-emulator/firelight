#pragma once

#include <cstdint>
#include <string>

namespace firelight::saves {
// Index row for a suspend point (save state). The state bytes live on disk;
// this row carries the lock flag and timestamps (int64 milliseconds)
struct SuspendPointMetadata {
  int id = -1;
  std::string contentHash;
  int saveSlot = 0;
  // Which suspend point within the save slot. Named apart from the save slot because the
  // two used to share a name, which is how the table ended up keyed on the wrong one
  unsigned int pointIndex = 0;
  int64_t lastModifiedAt = 0;
  int64_t createdAt = 0;
  bool locked = false;
};
} // namespace firelight::saves
