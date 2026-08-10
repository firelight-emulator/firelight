#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace firelight::library {

/**
 * A set of entries that are the same game — regional releases, revisions, translations.
 *
 * The group carries no metadata of its own; each variant keeps its own. What it holds is a title and
 * which variant stands for the set
 */
struct VariantGroup {
  int id = -1;
  std::string title;
  bool titleUserSet = false;
  std::optional<int> primaryEntryId;
  bool primaryUserSet = false;
  // TODO
  // Whether launching the group goes straight to the primary rather than asking which
  // variant to run
  bool autoLaunchPrimary = false;
  uint64_t createdAt = 0;
};

/**
 * The ordering used to rank variants when picking the one that stands for a group
 */
struct VariantPreference {
  std::vector<std::string> regionOrder;
  std::vector<std::string> languageOrder;
};

/**
 * What ranking a variant needs to know about it
 */
struct VariantCandidate {
  int entryId = -1;
  std::vector<std::string> regions;
  std::vector<std::string> languages;
  std::string revision;
  std::vector<std::string> flags;
  bool hidden = false;
};

/**
 * The entry that should stand for a group, or nothing when no candidate can.
 *
 * Ranks on released-before-unreleased, then preferred region, then preferred language, then highest
 * revision, and finally the lowest id so the answer never depends on the order asked. A hidden
 * candidate cannot be launched, so it is never chosen
 */
[[nodiscard]] std::optional<int> choosePrimaryEntry(const std::vector<VariantCandidate> &candidates,
                                                    const VariantPreference &preference);

/**
 * Splits a comma separated preference setting into codes, dropping blanks
 */
[[nodiscard]] std::vector<std::string> parsePreferenceOrder(const std::string &value);

} // namespace firelight::library
