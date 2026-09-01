// TODO: NEEDS REVIEW
#pragma once

#include <vector>

namespace firelight::library {

/**
 * The reason that an entry can't be launched
 */
enum class EntryProblem {
  PlatformNotSupported, // No core is registered for this platform at all, so nothing here will ever run it
  CoreNotInstalled,     // The core this platform resolves to is not on disk
  BiosMissing,          // The core is there but the system file it boots from is not
  ContentUnavailable,   // Used instead of FilesMissing when the content's root is unavailable (unplugged drive)
  FilesMissing,         // Previously known files are missing (deleted folder, moved out of library)
  ContentInArchive,     // A disc inside an archive. The core is handed a path that only exists within the archive
  // TODO
  NoRunConfiguration, // The files are here and readable, but nothing records a way to launch them
  DiscsMissing,       // Fewer discs than the set is known to have
};

/**
 * What is known about one entry, gathered by whoever has cheap access to it
 */
struct EntryStatusFacts {
  bool hasWayIn = true; // Whether the entry has content that can be launched at all (couldn't think of a better name)
  // TODO
  // Separate from hasWayIn: that one asks whether a file is there, this one whether anything
  // records how to launch it
  bool hasRunConfiguration = true;
  bool isCoreRegistered = true;
  bool isCoreInstalled = true;
  bool hasRequiredBios = true;
  bool isContentReachable = true;
  bool isDiscInArchive = false;
  int presentDiscCount = 0;  // 0 when the entry is not a disc set, or when no discs are present
  int expectedDiscCount = 0; // 0 when the entry is not a disc set, or when the set's disc count is unknown
};

/**
 * Everything wrong with an entry, worst first
 */
struct EntryStatus {
  std::vector<EntryProblem> problems;
  [[nodiscard]] bool isPlayable() const;
};

/**
 * Whether a problem stops the game from starting at all
 */
[[nodiscard]] bool blocksLaunch(EntryProblem problem);

/**
 * How far up the list a problem belongs.
 *
 * Ordered by what the person can do about it rather than by how broken it is.
 * Telling somebody a file is missing for a platform we will never run sends them to re-copy a game for nothing
 */
[[nodiscard]] int severity(EntryProblem problem);

/**
 * Every problem that applies, not only the first. A library scanned before any core is
 * installed would otherwise report the same thing everywhere and hide everything else
 */
[[nodiscard]] EntryStatus evaluateEntryStatus(const EntryStatusFacts &facts);

} // namespace firelight::library
