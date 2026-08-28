#include <firelight/library/smart_folder.hpp>
#include <firelight/util/strings.hpp>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <nlohmann/json.hpp>

namespace firelight::library {

SmartFolderCriteria SmartFolderCriteria::parse(const std::string &json) {
  SmartFolderCriteria c;
  if (json.empty()) {
    return c;
  }

  const auto parsed = nlohmann::json::parse(json, nullptr, false);
  if (parsed.is_discarded() || !parsed.is_object()) {
    return c; // malformed -> match-all
  }

  const auto getStr = [&](const char *key, std::string &out) {
    if (parsed.contains(key) && parsed[key].is_string()) {
      out = parsed[key].get<std::string>();
    }
  };
  const auto getIntVec = [&](const char *key, std::vector<int> &out) {
    if (parsed.contains(key) && parsed[key].is_array()) {
      for (const auto &v : parsed[key]) {
        if (v.is_number_integer()) {
          out.push_back(v.get<int>());
        }
      }
    }
  };

  getIntVec("contentDirectoryIds", c.contentDirectoryIds);
  getStr("pathContains", c.pathContains);
  getIntVec("platformIds", c.platformIds);
  if (parsed.contains("favorite") && parsed["favorite"].is_boolean()) {
    c.favorite = parsed["favorite"].get<bool>();
  }
  if (parsed.contains("genres") && parsed["genres"].is_array()) {
    for (const auto &v : parsed["genres"]) {
      if (v.is_string()) {
        c.genres.push_back(v.get<std::string>());
      }
    }
  }
  getStr("developer", c.developer);
  getStr("publisher", c.publisher);
  getStr("nameContains", c.nameContains);
  if (parsed.contains("playable") && parsed["playable"].is_boolean()) {
    c.playable = parsed["playable"].get<bool>();
  }
  if (parsed.contains("yearMin") && parsed["yearMin"].is_number_integer()) {
    c.yearMin = parsed["yearMin"].get<int>();
  }
  if (parsed.contains("yearMax") && parsed["yearMax"].is_number_integer()) {
    c.yearMax = parsed["yearMax"].get<int>();
  }
  if (parsed.contains("playedAfterMillis") && parsed["playedAfterMillis"].is_number_integer()) {
    c.playedAfterMillis = parsed["playedAfterMillis"].get<int64_t>();
  }
  if (parsed.contains("minSecondsPlayed") && parsed["minSecondsPlayed"].is_number_integer()) {
    c.minSecondsPlayed = parsed["minSecondsPlayed"].get<int64_t>();
  }
  if (parsed.contains("playedWithinDays") && parsed["playedWithinDays"].is_number_integer()) {
    c.playedWithinDays = parsed["playedWithinDays"].get<int>();
  }
  if (parsed.contains("unplayed") && parsed["unplayed"].is_boolean()) {
    c.unplayed = parsed["unplayed"].get<bool>();
  }

  return c;
}

std::string SmartFolderCriteria::toJson() const {
  nlohmann::json j = nlohmann::json::object();
  // Only emit set/non-empty fields, so the stored JSON stays minimal and an
  // unset criterion round-trips back to "ignored"
  if (!contentDirectoryIds.empty()) {
    j["contentDirectoryIds"] = contentDirectoryIds;
  }
  if (!pathContains.empty()) {
    j["pathContains"] = pathContains;
  }
  if (!platformIds.empty()) {
    j["platformIds"] = platformIds;
  }
  if (favorite.has_value()) {
    j["favorite"] = *favorite;
  }
  if (!genres.empty()) {
    j["genres"] = genres;
  }
  if (!developer.empty()) {
    j["developer"] = developer;
  }
  if (!publisher.empty()) {
    j["publisher"] = publisher;
  }
  if (!nameContains.empty()) {
    j["nameContains"] = nameContains;
  }
  if (playable.has_value()) {
    j["playable"] = *playable;
  }
  if (yearMin.has_value()) {
    j["yearMin"] = *yearMin;
  }
  if (yearMax.has_value()) {
    j["yearMax"] = *yearMax;
  }
  if (playedAfterMillis.has_value()) {
    j["playedAfterMillis"] = *playedAfterMillis;
  }
  if (minSecondsPlayed.has_value()) {
    j["minSecondsPlayed"] = *minSecondsPlayed;
  }
  if (playedWithinDays.has_value()) {
    j["playedWithinDays"] = *playedWithinDays;
  }
  if (unplayed.has_value()) {
    j["unplayed"] = *unplayed;
  }
  return j.dump();
}

bool SmartFolderCriteria::isEmpty() const {
  return contentDirectoryIds.empty() && pathContains.empty() && platformIds.empty() && !favorite.has_value() &&
         genres.empty() && developer.empty() && publisher.empty() && nameContains.empty() && !playable.has_value() &&
         !yearMin.has_value() && !yearMax.has_value() && !playedAfterMillis.has_value() &&
         !minSecondsPlayed.has_value() && !playedWithinDays.has_value() && !unplayed.has_value();
}

bool SmartFolderCriteria::matches(const EntryFields &entry) const {
  const auto nowMillis =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
          .count();
  return matches(entry, nowMillis);
}

bool SmartFolderCriteria::matches(const EntryFields &entry, const int64_t nowMillis) const {
  const auto &criteria = *this;
  // --- Source: which pool of games this folder draws from ---
  if (!criteria.contentDirectoryIds.empty()) {
    const bool intersects =
        std::any_of(entry.contentDirectoryIds.begin(), entry.contentDirectoryIds.end(), [&](int id) {
          return std::find(criteria.contentDirectoryIds.begin(), criteria.contentDirectoryIds.end(), id) !=
                 criteria.contentDirectoryIds.end();
        });
    if (!intersects) {
      return false;
    }
  }
  if (!criteria.pathContains.empty()) {
    const bool anyPathMatches =
        std::any_of(entry.contentPaths.begin(), entry.contentPaths.end(),
                    [&](const std::string &p) { return strings::containsIgnoringCase(p, criteria.pathContains); });
    if (!anyPathMatches) {
      return false;
    }
  }

  // --- Filters: attribute predicates (AND across, OR within a list) ---
  if (!criteria.platformIds.empty() && std::find(criteria.platformIds.begin(), criteria.platformIds.end(),
                                                 entry.platformId) == criteria.platformIds.end()) {
    return false;
  }
  if (criteria.favorite.has_value() && entry.favorite != *criteria.favorite) {
    return false;
  }
  if (!criteria.genres.empty()) {
    const bool anyGenre = std::any_of(criteria.genres.begin(), criteria.genres.end(), [&](const std::string &wanted) {
      return std::any_of(entry.genres.begin(), entry.genres.end(),
                         [&](const std::string &held) { return strings::containsIgnoringCase(held, wanted); });
    });
    if (!anyGenre) {
      return false;
    }
  }
  if (!criteria.developer.empty() && !strings::containsIgnoringCase(entry.developer, criteria.developer)) {
    return false;
  }
  if (!criteria.publisher.empty() && !strings::containsIgnoringCase(entry.publisher, criteria.publisher)) {
    return false;
  }
  if (!criteria.nameContains.empty() && !strings::containsIgnoringCase(entry.searchText, criteria.nameContains)) {
    return false;
  }
  if (criteria.playable.has_value() && entry.playable != *criteria.playable) {
    return false;
  }
  // An unknown release year (0) satisfies no year bound, so a year-range folder
  // excludes entries with unknown years
  if (criteria.yearMin.has_value() && entry.releaseYear < *criteria.yearMin) {
    return false;
  }
  if (criteria.yearMax.has_value() && (entry.releaseYear == 0 || entry.releaseYear > *criteria.yearMax)) {
    return false;
  }
  if (criteria.playedAfterMillis.has_value() && entry.lastPlayedMillis < *criteria.playedAfterMillis) {
    return false;
  }
  if (criteria.minSecondsPlayed.has_value() && entry.secondsPlayed < *criteria.minSecondsPlayed) {
    return false;
  }
  if (criteria.unplayed.has_value() && (entry.lastPlayedMillis == 0) != *criteria.unplayed) {
    return false;
  }
  if (criteria.playedWithinDays.has_value()) {
    // Never-played entries fall outside any recency window
    if (entry.lastPlayedMillis == 0) {
      return false;
    }
    const int64_t windowMillis = static_cast<int64_t>(*criteria.playedWithinDays) * 86400000LL;
    if (entry.lastPlayedMillis < nowMillis - windowMillis) {
      return false;
    }
  }

  return true;
}

} // namespace firelight::library
