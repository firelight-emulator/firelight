#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace firelight::library {
struct Entry {
  int id = -1;
  std::string displayName;
  bool nameUserSet = false; // So we don't overwrite a user-edited name with metadata
  std::string contentHash;
  unsigned platformId = 0;
  unsigned activeSaveSlot = 1;
  bool hidden = false;
  bool favorite = false;
  std::string icon1x1SourceUrl;
  std::string boxartFrontSourceUrl;
  std::string boxartBackSourceUrl;
  std::string description;
  unsigned releaseYear = 0;
  std::string developer;
  std::string publisher;
  std::string genres;
  std::string regionIds;
  unsigned retroachievementsSetId = 0;

  std::vector<int> folderIds{};

  // The content directories that contain this entry's content file(s). Used for smart filtering and for
  // determining whether an entry is in any content directories
  std::vector<int> contentDirectoryIds{};
  std::vector<std::string> contentPaths{};

  uint64_t createdAt = 0;
};
} // namespace firelight::library
