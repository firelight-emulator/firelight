#pragma once

#include <cstdint>
#include <string>

namespace firelight::library {
// How a folder gets its contents. Manual folders hold hand-picked members
// (folder_entries rows); Smart folders compute membership live from filterJson
// criteria and have no folder_entries rows
enum class FolderType {
  Manual = 0,
  Smart = 1,
};

struct FolderInfo {
  int id = -1;
  std::string displayName;
  std::string description;
  std::string iconSourceUrl;
  // Manual (0) or Smart (1). Smart folders carry their criteria in filterJson
  int type = static_cast<int>(FolderType::Manual);
  // For smart folders: the serialized SmartFolderCriteria (see smart_folder.hpp)
  // Empty for manual folders
  std::string filterJson;

  // --- Appearance (Phase 2) ---
  // Accent color for the folder (e.g. "#ff8800"); empty = use the default
  std::string color;
  // The folder's remembered game sort: the EntryListModel role name to sort by
  // (empty = no override, use the view default) and its direction
  std::string sortRole;
  bool sortAscending = true;

  // --- Ordering / nesting ---
  // The containing folder's id, or -1 for a root-level folder. (The nested-tree
  // UI is a later phase; this column enables per-container ordering now.)
  int parentId = -1;
  // Manual sort position within the parent scope (0-based, ascending)
  int position = 0;

  uint64_t createdAt;
};
} // namespace firelight::library
