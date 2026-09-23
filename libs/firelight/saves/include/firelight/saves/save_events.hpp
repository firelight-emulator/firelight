#pragma once

#include <string>

namespace firelight::saves {

struct SuspendPointUpdatedEvent {
  std::string contentHash;
  int saveSlot = -1;
  int index = -1;
};

struct SuspendPointDeletedEvent {
  std::string contentHash;
  int saveSlot = -1;
  int index = -1;
};

} // namespace firelight::saves
