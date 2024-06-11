//
// Created by alexs on 4/7/2024.
//

#include "mod_manager.hpp"

#include <algorithm>

namespace firelight::mods {
void ModManager::installLatest(const int modId, const int romId) const {
  // Get the most recent release of the mod from the content database
  auto mod = m_contentDatabase.getMod(modId);
  if (!mod) {
    return;
  }

  // From the release, find a patch file that targets the given ROM
  auto patches = m_contentDatabase.getMatchingPatches({.modId = modId});
  if (patches.empty()) {
    return;
  }

  for (const auto &patch : patches) {
    if (patch.romId == romId) {
      auto rom = m_contentDatabase.getRom(romId);
      if (!rom) {
        return;
      }

      db::LibraryEntry entry{
          .displayName = mod->name,
          .contentId = patch.md5,
          .platformId = rom->platformId,
          .activeSaveSlot = 1,
          .type = db::LibraryEntry::EntryType::PATCH,
          .sourceDirectory = "",
          .contentPath = "" // TODO: Set this to the new location
      };

      m_libraryDatabase.createLibraryEntry(entry);
      // TODO: Move the actual file
      return;
    }
  }
}
} // namespace firelight::mods
