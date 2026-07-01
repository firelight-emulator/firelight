#pragma once

#include <string>

namespace firelight::library {

// Domain events the user-library repository publishes through the global
// EventDispatcher (replacing the old Qt signals). They are consumed by the
// ingest service (content/run-config) and by the app's scanner wiring (watched
// directories). Synchronous, same-thread delivery.

struct ContentFileAddedEvent {
  int id = -1;
  std::string filePath;
  int platformId = -1;
  std::string contentHash;
};

struct RunConfigurationCreatedEvent {
  int id = -1;
  std::string filePath;
  int platformId = -1;
  std::string contentHash;
};

struct RunConfigurationDeletedEvent {
  std::string contentHash;
};

struct WatchedDirectoryAddedEvent {
  int id = -1;
  std::string path;
};

struct WatchedDirectoryRemovedEvent {
  int id = -1;
  std::string path;
};

struct WatchedDirectoryUpdatedEvent {
  int id = -1;
  std::string oldPath;
  std::string newPath;
};

} // namespace firelight::library
