#pragma once

#include <optional>
#include <string>

namespace firelight::library {

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

struct ContentFileMissingEvent {
  int id = -1;
  std::string contentHash;
  std::optional<int> discSetId{};
};

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

struct EntryCreatedEvent {
  int entryId;
};

struct EntryUpdatedEvent {
  int entryId;
};

// A folder was created, edited or deleted
struct FolderChangedEvent {
  int folderId;
};

struct EntryDeletedEvent {
  int entryId;
};

// Published when a group's title, pin, or the entry standing for it changes, and when the
// group is dissolved. The id outlives the group, so a listener must tolerate it being gone
struct VariantGroupUpdatedEvent {
  int groupId;
};

// Published when one entry is folded into another because they turned out to be the same
// game
struct EntryAbsorbedEvent {
  int survivingEntryId;
  std::string absorbedContentHash;
  std::string survivingContentHash;
};

} // namespace firelight::library
