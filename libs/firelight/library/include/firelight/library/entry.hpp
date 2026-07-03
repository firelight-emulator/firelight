#pragma once

#include <QString>
#include <cstdint>
#include <string>
#include <vector>

namespace firelight::library {
  struct Entry {
    int id = -1;
    QString displayName;
    QString contentHash;
    unsigned platformId = 0;
    unsigned activeSaveSlot = 1;
    bool hidden = false;
    bool favorite = false;
    QString icon1x1SourceUrl;
    QString boxartFrontSourceUrl;
    QString boxartBackSourceUrl;
    QString description;
    unsigned releaseYear = 0;
    QString developer;
    QString publisher;
    QString genres;
    QString regionIds;
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
