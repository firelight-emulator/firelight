#pragma once

#include <cstdint>
#include <string>

namespace firelight::library {

enum class FolderType {
  Manual = 0, // Hand-picked entries, stored in folder_entries
  Smart = 1,  // Entries are computed from filterJson criteria, no folder_entries rows
};

struct FolderInfo {
  int id = -1;
  std::string displayName;
  std::string description;
  std::string iconSourceUrl;

  int type = static_cast<int>(FolderType::Manual);
  std::string filterJson; // For smart folders: the serialized SmartFolderCriteria. Empty for manual folders

  std::string color; // Accent color for the folder, in #RRGGBB form. Empty = default

  std::string sortRole; // Empty = no override, use the view default
  bool sortAscending = true;

  int parentId = -1; // The parent folder, or -1 for a top-level folder
  int position = 0;  // Manual sort position within the parent scope (0-based, ascending)

  uint64_t createdAt;
};
} // namespace firelight::library
