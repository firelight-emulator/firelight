#pragma once

#include <cstdint>
#include <string>

namespace firelight::saves {

struct SuspendPointMetadata {
  int id = -1;
  std::string contentHash;
  int saveSlot = 0;
  unsigned int pointIndex = 0; // The index of the suspend point within the save slot, starting at 0
  int64_t lastModifiedAt = 0;
  int64_t createdAt = 0;
  bool locked = false;
};
} // namespace firelight::saves
