#pragma once

#include <cstdint>
#include <string>

namespace firelight::db {

struct LibraryEntry {
  enum class EntryType { ROM, PATCH };

  int id;
  std::string displayName;
  std::string contentMd5;
  int platformId;
  EntryType type;
  std::string sourceDirectory;
  std::string contentPath;
  uint64_t createdAt;
};

} // namespace firelight::db