#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace firelight::library {
  struct Entry {
    int id = -1;
    std::string displayName;
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

    // Filesystem provenance, joined from the entry's content_files (by
    // content_hash). Populated when loading an Entry; drive the two smart-folder
    // source modes. contentDirectoryIds holds the distinct content directories
    // (>= 0) the entry's files live under; contentPaths holds their on-disk
    // paths (the archive path for archived content) for "path contains" matching.
    std::vector<int> contentDirectoryIds{};
    std::vector<std::string> contentPaths{};

    uint64_t createdAt = 0;
  };
} // namespace firelight::library
