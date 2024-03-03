//
// Created by alexs on 3/2/2024.
//

#pragma once
#include <string>

namespace firelight::db {

struct LibraryEntry {

  enum class EntryType { ROM, PATCH };
  long id;
  std::string displayName;
  std::string contentMd5;
  long platformId;
  EntryType type;
  std::string sourceDirectory;
  std::string contentPath;
};
} // namespace firelight::db