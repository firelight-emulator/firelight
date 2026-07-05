#pragma once

#include <string>

namespace firelight::saves {

// Domain events the SaveManager publishes through the global EventDispatcher
// (replacing the old Qt signals on ISaveManager). Consumed by the app's
// suspend-point UI. Synchronous, same-thread delivery.

struct SuspendPointUpdatedEvent {
  std::string contentHash;
  int saveSlotNumber = -1;
  int index = -1;
};

struct SuspendPointDeletedEvent {
  std::string contentHash;
  int saveSlotNumber = -1;
  int index = -1;
};

} // namespace firelight::saves
