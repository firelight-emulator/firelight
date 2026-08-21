#pragma once

#include <vector>

namespace firelight::library {

/**
 * Something standing between a game and being played.
 *
 * Reserved growth, deliberately absent: a sheet whose track is gone, and a core that is there but
 * refuses to load
 */
enum class EntryProblem {
  // No core is registered for this platform at all, so nothing here will ever run it
  PlatformNotSupported,
  // The core this platform resolves to is not on disk
  CoreNotInstalled,
  // The core is there but the system file it boots from is not
  BiosMissing,
  // The folder the content lives in is still in the library and cannot be read right now, which is
  // what an unplugged drive looks like. The files are still catalogued, so nothing is lost
  ContentUnavailable,
  // The files are not where they were, whether they were deleted or their folder was taken out of
  // the library. The rows outlive them, so this can still say where they used to be
  FilesMissing,
  // A disc inside an archive. The core is handed a path that only exists within the archive
  ContentInArchive,
  // Fewer discs than the set is known to have
  DiscsMissing,
};

/**
 * What is known about one entry, gathered by whoever has cheap access to it
 */
struct EntryStatusFacts {
  bool hasWayIn = true;
  bool isCoreRegistered = true;
  bool isCoreInstalled = true;
  bool hasRequiredBios = true;
  // Whether the folder this entry's content sits in can be read at the moment
  bool isContentReachable = true;
  bool isDiscInArchive = false;
  // Both 0 when the entry is not a disc set, and the expected count is 0 until something
  // authoritative supplies it
  int presentDiscCount = 0;
  int expectedDiscCount = 0;
};

/**
 * Everything wrong with an entry, worst first
 */
struct EntryStatus {
  std::vector<EntryProblem> problems;

  /**
   * Whether the game would start. A set missing a disc still plays
   */
  [[nodiscard]] bool isPlayable() const;
};

/**
 * Whether a problem stops the game starting at all, as opposed to being worth mentioning
 */
[[nodiscard]] bool blocksLaunch(EntryProblem problem);

/**
 * How far up the list a problem belongs.
 *
 * Ordered by what the person can do about it rather than by how broken it is: what Firelight
 * cannot run comes before what one action fixes, which comes before what the filesystem fixes.
 * Telling somebody a file is missing for a platform we will never run sends them to re-copy a
 * game for nothing
 */
[[nodiscard]] int severity(EntryProblem problem);

/**
 * Every problem that applies, not only the first. A library scanned before any core is
 * installed would otherwise report the same thing everywhere and hide everything else
 */
[[nodiscard]] EntryStatus evaluateEntryStatus(const EntryStatusFacts &facts);

} // namespace firelight::library
