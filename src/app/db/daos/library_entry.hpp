//
// Created by alexs on 3/2/2024.
//

#pragma once
#include <string>

namespace Firelight::Databases {

enum class EntryType { ROM, PATCH };

struct LibraryEntry {
  long id;
  std::string displayName;
  std::string contentMd5;
  long platformId;
  EntryType type;
  std::string sourceDirectory;
  std::string contentPath;
};
} // namespace Firelight::Databases