#pragma once

#include <firelight/library/entry.hpp>

#include <string>
#include <vector>

namespace firelight::library {

/**
 * What is known about which game an entry is a copy of.
 *
 * Every field is three-valued: it says the same thing, a different thing, or nothing at all.
 * Nothing at all never counts as different — an id nobody has resolved and a region nobody has
 * scraped are absences, not distinctions
 */
struct GameIdentity {
  unsigned platformId = 0;

  // The content database's id for the game, 0 when nothing has resolved one. It spans regions
  // and discs by construction, which is why region has to survive as its own field
  int gameId = 0;

  // The folded title. The whole key before a content database exists, and the fallback for
  // anything it has nothing to say about afterwards
  std::string title;

  // Ordered most authoritative first, empty when nothing has said
  std::vector<std::string> regions;

  // Which disc of a set, 0 when not one. Not identity — two entries can be the same game and
  // the same release and still be different discs — but the axis the two groupers read in
  // opposite directions
  int discNumber = 0;

  /** Whether nothing has said what this is, in which case it matches nothing */
  [[nodiscard]] bool isEmpty() const { return gameId == 0 && title.empty(); }
};

/** The identity an entry carries */
[[nodiscard]] GameIdentity identityOf(const Entry &entry);

/**
 * Whether two entries are copies of the same game.
 *
 * Resolved ids win outright when both sides have one. When either side has none the folded
 * title decides, which is the whole rule before a content database exists
 */
[[nodiscard]] bool areSameGame(const GameIdentity &left, const GameIdentity &right);

/**
 * Whether two regions can be the same release. An empty side is a release nothing has told us
 * about, not a third one
 */
[[nodiscard]] bool areRegionsCompatible(const std::vector<std::string> &left, const std::vector<std::string> &right);

/**
 * Whether two entries are alternate copies of one thing: the same game, standing in the same
 * place within it. Disc 2 of one release is a variant of disc 2 of another and never of disc 1
 */
[[nodiscard]] bool areVariants(const GameIdentity &left, const GameIdentity &right);

/**
 * Whether two entries are discs of one release: the same game, with nothing saying they came
 * from different regions.
 *
 * Disc numbers are deliberately not compared. Two copies of disc 2 have to reach the grouper so
 * it can refuse to guess which release each came from, rather than being hidden here
 */
[[nodiscard]] bool areDiscsOfOneRelease(const GameIdentity &left, const GameIdentity &right);

} // namespace firelight::library
