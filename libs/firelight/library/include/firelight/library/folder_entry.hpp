#pragma once

namespace firelight::library {

/**
 * Represents an entry's membership in a manual folder
 */
struct FolderEntry {
  int folderId = -1;
  int entryId = -1;

  int position = 0; // Sort position for the entry in the collection
};
} // namespace firelight::library
