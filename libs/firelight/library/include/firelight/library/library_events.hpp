#pragma once

#include <optional>
#include <string>

namespace firelight::library {

// Domain events the user-library repository publishes through the global
// EventDispatcher (replacing the old Qt signals). They are consumed by the
// ingest service (content/run-config) and by the app's scanner wiring (watched
// directories). Synchronous, same-thread delivery

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

// TODO
// A catalogued file that is no longer on disk. The row stays, so this says nothing about the
// entry on its own — only every copy of a hash being gone decides that. The set id is carried
// because an absorbed disc has no entry of its own to notice
struct ContentFileMissingEvent {
  int id = -1;
  std::string contentHash;
  std::optional<int> discSetId{};
};

// TODO
// A file that was marked missing and is back
struct ContentFileRestoredEvent {
  int id = -1;
  std::string contentHash;
  std::optional<int> discSetId{};
};

struct ContentDirectoryAddedEvent {
  int id = -1;
  std::string path;
};

struct ContentDirectoryRemovedEvent {
  int id = -1;
  std::string path;
};

struct ContentDirectoryUpdatedEvent {
  int id = -1;
  std::string oldPath;
  std::string newPath;
};

} // namespace firelight::library
