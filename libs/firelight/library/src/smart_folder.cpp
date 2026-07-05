#include <firelight/library/smart_folder.hpp>

#include <algorithm>
#include <cctype>
#include <nlohmann/json.hpp>

namespace firelight::library {

namespace {
// Case-insensitive ASCII substring test. An empty needle matches anything.
bool containsCaseInsensitive(const std::string &haystack,
                             const std::string &needle) {
  if (needle.empty()) {
    return true;
  }
  const auto toLower = [](const std::string &s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return out;
  };
  return toLower(haystack).find(toLower(needle)) != std::string::npos;
}
} // namespace

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
  if (parsed.contains("yearMin") && parsed["yearMin"].is_number_integer()) {
    c.yearMin = parsed["yearMin"].get<int>();
  }
  if (parsed.contains("yearMax") && parsed["yearMax"].is_number_integer()) {
    c.yearMax = parsed["yearMax"].get<int>();
  }
  if (parsed.contains("playedAfterMillis") &&
      parsed["playedAfterMillis"].is_number_integer()) {
    c.playedAfterMillis = parsed["playedAfterMillis"].get<int64_t>();
  }
  if (parsed.contains("minSecondsPlayed") &&
      parsed["minSecondsPlayed"].is_number_integer()) {
    c.minSecondsPlayed = parsed["minSecondsPlayed"].get<int64_t>();
  }

  return c;
}

std::string SmartFolderCriteria::toJson() const {
  nlohmann::json j = nlohmann::json::object();
  // Only emit set/non-empty fields, so the stored JSON stays minimal and an
  // unset criterion round-trips back to "ignored".
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
  return j.dump();
}

bool SmartFolderCriteria::isEmpty() const {
  return contentDirectoryIds.empty() && pathContains.empty() &&
         platformIds.empty() && !favorite.has_value() && genres.empty() &&
         developer.empty() && publisher.empty() && !yearMin.has_value() &&
         !yearMax.has_value() && !playedAfterMillis.has_value() &&
         !minSecondsPlayed.has_value();
}

bool SmartFolderCriteria::matches(const EntryFields &entry) const {
  const auto &criteria = *this;
  // --- Source: which pool of games this folder draws from ---
  if (!criteria.contentDirectoryIds.empty()) {
    const bool intersects =
        std::any_of(entry.contentDirectoryIds.begin(),
                    entry.contentDirectoryIds.end(), [&](int id) {
                      return std::find(criteria.contentDirectoryIds.begin(),
                                       criteria.contentDirectoryIds.end(),
                                       id) != criteria.contentDirectoryIds.end();
                    });
    if (!intersects) {
      return false;
    }
  }
  if (!criteria.pathContains.empty()) {
    const bool anyPathMatches =
        std::any_of(entry.contentPaths.begin(), entry.contentPaths.end(),
                    [&](const std::string &p) {
                      return containsCaseInsensitive(p, criteria.pathContains);
                    });
    if (!anyPathMatches) {
      return false;
    }
  }

  // --- Filters: attribute predicates (AND across, OR within a list) ---
  if (!criteria.platformIds.empty() &&
      std::find(criteria.platformIds.begin(), criteria.platformIds.end(),
                entry.platformId) == criteria.platformIds.end()) {
    return false;
  }
  if (criteria.favorite.has_value() && entry.favorite != *criteria.favorite) {
    return false;
  }
  if (!criteria.genres.empty()) {
    const bool anyGenre =
        std::any_of(criteria.genres.begin(), criteria.genres.end(),
                    [&](const std::string &g) {
                      return containsCaseInsensitive(entry.genres, g);
                    });
    if (!anyGenre) {
      return false;
    }
  }
  if (!criteria.developer.empty() &&
      !containsCaseInsensitive(entry.developer, criteria.developer)) {
    return false;
  }
  if (!criteria.publisher.empty() &&
      !containsCaseInsensitive(entry.publisher, criteria.publisher)) {
    return false;
  }
  // An unknown release year (0) satisfies no year bound, so a year-range folder
  // excludes entries with unknown years.
  if (criteria.yearMin.has_value() && entry.releaseYear < *criteria.yearMin) {
    return false;
  }
  if (criteria.yearMax.has_value() &&
      (entry.releaseYear == 0 || entry.releaseYear > *criteria.yearMax)) {
    return false;
  }
  if (criteria.playedAfterMillis.has_value() &&
      entry.lastPlayedMillis < *criteria.playedAfterMillis) {
    return false;
  }
  if (criteria.minSecondsPlayed.has_value() &&
      entry.secondsPlayed < *criteria.minSecondsPlayed) {
    return false;
  }

  return true;
}

} // namespace firelight::library
