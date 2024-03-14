#pragma once

#include <cstdint>
#include <string>

namespace firelight::db {

struct LibraryEntry {
  enum class EntryType { ROM, PATCH, UNKNOWN };

  int id = -1;
  std::string displayName;
  std::string contentMd5;
  int platformId = -1;
  unsigned int activeSaveSlot = 1;
  EntryType type = EntryType::UNKNOWN;
  std::string sourceDirectory;
  std::string contentPath;
  unsigned int createdAt = 0;
};

} // namespace firelight::db