#pragma once

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
  // TODO: Field for last played time
  // TODO: Field for "verified" - flag if this is a known ROM

  // TODO: How to set these:
  // TODO: Field for "game ID" - if this is a known ROM, the game it belongs to
  // TODO: Field for "rom ID" - if this is a known ROM, the ROM it belongs to
  // TODO: Field for "parent entry" - if this is a ROM hack, the ROM it's a hack
  // of
  // TODO: Field for "rom hack" - if this is a ROM hack, the ID of the ROM hack
  // TODO: Field for "patch" - if this is a patch, the ID of the patch
};

} // namespace firelight::db