// TODO: NEEDS REVIEW
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace firelight::library {

/**
 * A game that came on more than one disc
 *
 * The discs are content files, not entries, so a three-disc game is one row in the library.
 * Which disc to launch is a parameter rather than a separate way to launch
 */
struct DiscSet {
  int id = -1;

  // TODO
  // What a disc looking for its set matches on, alongside the folded title below
  unsigned platformId = 0;

  std::string title;
  bool titleUserSet = false;

  // TODO
  // The folded form of the title, which is what grouping compares. Kept beside the display title
  // so renaming a set does not move it
  std::string normalizedTitle;

  int discCount = 0; // How many discs the game came on, 0 until something authoritative says
  uint64_t createdAt = 0;
};

/**
 * Which disc a save slot was last playing, so resuming returns to it
 */
struct EntryDiscState {
  int entryId = -1;
  int saveSlot = 0;
  int discNumber = 0;
};

/**
 * The disc numbers missing from the run, which is empty for a complete set. A set with no
 * disc 1 reports 1 as missing
 *
 * @param discNumbers The disc numbers present, which may be empty
 * @param knownDiscCount How many discs the game came on
 */
[[nodiscard]] std::vector<int> missingDiscs(const std::vector<int> &discNumbers, int knownDiscCount = 0);

} // namespace firelight::library
