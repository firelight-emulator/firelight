#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace firelight::library {

/**
 * A game that came on more than one disc.
 *
 * The discs are content files, not entries, so a three-disc game is one row in the library.
 * Which disc to launch is a parameter rather than a separate way to launch
 */
struct DiscSet {
  int id = -1;
  std::string title;
  bool titleUserSet = false;
  // How many discs the game came on, 0 until something authoritative says. Nothing can infer
  // it from the discs somebody happens to own
  int discCount = 0;
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
 * How complete a set is, from the disc numbers actually present
 */
enum class DiscSetState {
  // Every disc from 1 upward is here
  Complete,
  // Disc 1 is missing, so there is nothing worth launching
  MissingFirstDisc,
  // Disc 1 is here but the run has a hole in it
  MissingMiddleDisc,
};

/**
 * Which disc stands for a set: the lowest number present. There is no ranking to do, unlike a
 * variant group — a set is ordered by construction
 *
 * @return The disc number, or 0 when there are none
 */
[[nodiscard]] int representativeDisc(const std::vector<int> &discNumbers);

/**
 * Reads the shape of a set from the discs it holds.
 *
 * Without an external database we cannot know a set's true size, so this only reports what is
 * knowable: whether the numbers run from 1 without a gap
 */
[[nodiscard]] DiscSetState discSetState(const std::vector<int> &discNumbers);

/**
 * The disc numbers missing from the run, which is empty for a complete set. A set with no
 * disc 1 reports 1 as missing
 *
 * @param knownDiscCount How many discs the game came on, when something authoritative says.
 * Without it the run can only be checked for holes, so a set short of its last disc looks
 * complete — owning 1 and 2 of three reports nothing until the count is known
 */
[[nodiscard]] std::vector<int> missingDiscs(const std::vector<int> &discNumbers, int knownDiscCount = 0);

/**
 * Whether a set can be launched at all. Playing disc 2 of an RPG without disc 1 is not a use
 * case worth supporting, and refusing it keeps the set's identity from ever moving
 */
[[nodiscard]] bool isDiscSetLaunchable(const std::vector<int> &discNumbers);

} // namespace firelight::library
