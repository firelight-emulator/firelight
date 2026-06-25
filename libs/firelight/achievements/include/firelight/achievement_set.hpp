#pragma once
#include "achievement.hpp"
#include "leaderboard.hpp"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace firelight::achievements {
struct AchievementSet {
  unsigned id{};
  unsigned gameId{};
  std::string title{};
  std::string type{};
  std::string imageIconUrl{};

  // Calculated fields
  unsigned numAchievements{};
  unsigned totalPoints{};

  // Read-only, populated from database
  std::vector<Achievement> achievements{};
  std::vector<Leaderboard> leaderboards{};
};

inline void from_json(const nlohmann::json &j, AchievementSet &a) {
  j.at("AchievementSetId").get_to(a.id);
  j.at("GameId").get_to(a.gameId);

  if (j.contains("Title") && !j.at("Title").is_null()) {
    j.at("Title").get_to(a.title);
  }

  if (j.contains("Type") && !j.at("Type").is_null()) {
    j.at("Type").get_to(a.type);
  }

  if (j.contains("ImageIconUrl") && !j.at("ImageIconUrl").is_null()) {
    j.at("ImageIconUrl").get_to(a.imageIconUrl);
  }

  j.at("Achievements").get_to(a.achievements);
  j.at("Leaderboards").get_to(a.leaderboards);
}

inline void to_json(nlohmann::json &j, const AchievementSet &a) {
  j = nlohmann::json{{"AchievementSetId", a.id},
                     {"GameId", a.gameId},
                     {"Title", a.title},
                     {"Type", a.type},
                     {"ImageIconUrl", a.imageIconUrl},
                     {"Achievements", a.achievements},
                     {"Leaderboards", a.leaderboards}};
}
} // namespace firelight::achievements