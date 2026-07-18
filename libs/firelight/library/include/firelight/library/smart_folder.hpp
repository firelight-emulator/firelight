#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace firelight::library {

// TODO
// The primitive attributes of a library entry needed to evaluate smart-folder
// criteria. Deliberately Qt-free and self-contained so matches() is a pure
// function and fully unit-testable. The model layer builds it from a loaded
// Entry (its attributes + file locations) joined with play stats from
// the activity log
struct EntryFields {
  int platformId = 0;
  bool favorite = false;
  // The entry's genres, as stored on the entry (a free-form string, typically
  // comma- or semicolon-separated, e.g. "Action, Adventure")
  std::string genres;
  std::string developer;
  std::string publisher;
  int releaseYear = 0; // 0 = unknown
  // File locations (see Entry): the content directories the entry's
  // files live under, and their on-disk paths
  std::vector<int> contentDirectoryIds;
  std::vector<std::string> contentPaths;
  // Play history, from the activity log. 0 = never played
  int64_t lastPlayedMillis = 0;
  int64_t secondsPlayed = 0;
};

// TODO
// Criteria defining a smart folder's membership. Two orthogonal parts:
//   SOURCE (which pool of games): contentDirectoryIds (specific content
//     directories) and/or pathContains (case-insensitive substring of a file
//     path — lets users organize with subfolders under one content directory)
//     An empty source means the whole library
//   FILTERS (attribute predicates): platform, favorite, metadata, play history
// Every specified criterion must hold (AND across criteria). A multi-valued
// criterion (platformIds, genres) matches if ANY of its values match (OR
// within the list). An unset/empty criterion is ignored
struct SmartFolderCriteria {
  // --- Source ---
  std::vector<int> contentDirectoryIds;
  std::string pathContains;

  // --- Filters ---
  std::vector<int> platformIds;
  std::optional<bool> favorite;
  std::vector<std::string> genres; // entry matches if it has any listed genre
  std::string developer;           // case-insensitive substring
  std::string publisher;           // case-insensitive substring
  std::optional<int> yearMin;
  std::optional<int> yearMax;
  std::optional<int64_t> playedAfterMillis;
  std::optional<int64_t> minSecondsPlayed;
  // TODO
  // Rolling window: the entry must have been played within the last N days,
  // resolved against "now" at evaluation time so it never goes stale (unlike
  // playedAfterMillis, which is a fixed timestamp)
  std::optional<int> playedWithinDays;
  // true = never played; false = has been played at least once
  std::optional<bool> unplayed;

  // Parses criteria from JSON. Malformed JSON yields empty (match-all) criteria
  // rather than throwing, so a corrupt folder degrades to "whole library"
  static SmartFolderCriteria parse(const std::string &json);
  [[nodiscard]] std::string toJson() const;

  // True when no source and no filter is set — the folder matches everything
  [[nodiscard]] bool isEmpty() const;

  // TODO
  // Pure predicate: does the entry satisfy every specified criterion? The
  // no-clock overload resolves the rolling playedWithinDays window against the
  // system clock; pass nowMillis explicitly for deterministic tests
  [[nodiscard]] bool matches(const EntryFields &entry) const;
  [[nodiscard]] bool matches(const EntryFields &entry, int64_t nowMillis) const;
};

} // namespace firelight::library
