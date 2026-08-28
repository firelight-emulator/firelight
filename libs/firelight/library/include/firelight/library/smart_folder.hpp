#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace firelight::library {

// The primitive attributes of a library entry needed to evaluate smart-folder
// criteria
struct EntryFields {
  int platformId = 0;
  bool favorite = false;
  std::vector<std::string> genres;
  std::string developer;
  std::string publisher;
  int releaseYear = 0; // 0 = unknown
  std::vector<int> contentDirectoryIds;
  std::vector<std::string> contentPaths;
  int64_t lastPlayedMillis = 0;
  int64_t secondsPlayed = 0;
  std::string searchText;
  bool playable = true;
  std::vector<int> folderIds;
};

// Criteria defining a smart folder's membership
// Every specified criterion must match. An unset/empty criterion is ignored
struct SmartFolderCriteria {
  std::vector<int> contentDirectoryIds; // entry matches if it has any listed content directory
  std::string pathContains;             // case-insensitive substring of a content path

  std::vector<int> platformIds;
  std::optional<bool> favorite;
  std::vector<std::string> genres; // entry matches if it has any listed genre
  std::string developer;           // case-insensitive substring
  std::string publisher;           // case-insensitive substring
  std::string nameContains;        // case-insensitive substring of searchText
  std::optional<bool> playable;    // true = only what would start; false = only what would not
  std::optional<int> yearMin;
  std::optional<int> yearMax;
  std::optional<int64_t> playedAfterMillis;
  std::optional<int64_t> minSecondsPlayed;
  std::optional<int> playedWithinDays; // Number of days back from now, inclusive. 0=today, 1=yesterday or today, etc
  std::optional<bool> unplayed;        // true = never played; false = has been played at least once

  static SmartFolderCriteria parse(const std::string &json); // Malformed JSON yields empty (match-all) criteria
  [[nodiscard]] std::string toJson() const;

  // True when no source and no filter is set, so the folder matches everything
  [[nodiscard]] bool isEmpty() const;

  [[nodiscard]] bool matches(const EntryFields &entry) const; // Resolves playedWithinDays against the system clock
  [[nodiscard]] bool matches(const EntryFields &entry, int64_t nowMillis) const; // Only exists for deterministic tests

  bool operator==(const SmartFolderCriteria &) const = default;
};

} // namespace firelight::library
