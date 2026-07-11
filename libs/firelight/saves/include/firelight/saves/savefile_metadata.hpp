#pragma once

#include <cstdint>
#include <string>

namespace firelight::saves {
// Index row for a battery/SRAM save (the bytes live on disk as savefile.srm).
// Timestamps are int64 milliseconds since the Unix epoch.
struct SavefileMetadata {
  int id = -1;
  std::string contentId;
  unsigned int slotNumber = 1;
  std::string savefileMd5;
  int64_t lastModifiedAt = 0;
  int64_t createdAt = 0;
};
} // namespace firelight::saves
