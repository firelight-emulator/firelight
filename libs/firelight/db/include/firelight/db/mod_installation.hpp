#pragma once

#include <cstdint>
#include <string>

namespace firelight::db {
// A mod installed on top of an owned base ROM. Each installation is surfaced as
// its own library entry (its own tile) while remaining linked back to the base
// game by `baseContentHash` so a base game's details page can aggregate every
// mod installed for it.
//
// `patchedContentHash` is the canonical content hash of the patched bytes for
// the currently-installed version. It is both the save-directory key for the
// modded content and the copy-forward source when a save-compatible update
// lands. Because saves are content-addressed, a separately-imported pre-patched
// ROM that hashes to the same value shares saves transparently, with no row
// here required.
struct ModInstallation {
  int id = -1;
  int entryId = -1;
  std::string baseContentHash;
  int modId = -1;
  std::string modSlug;
  int installedPatchId = -1;
  std::string installedVersion;
  int saveGeneration = 0;
  std::string patchedContentHash;
  std::string patchFilePath;
  bool pinned = false;
  // The highest version the user has explicitly skipped; updates at or below it
  // are not surfaced. Empty means nothing skipped.
  std::string ignoreUpdatesUpToVersion;
  // "shop" for a one-click install, "dropin" for a hand-dropped patch file.
  std::string source;
  uint64_t createdAt = 0;
};
} // namespace firelight::db
