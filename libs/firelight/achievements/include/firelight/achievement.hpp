#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace firelight::achievements {
struct Achievement {
  unsigned id;
  unsigned achievementSetId;
  std::string memAddr;
  std::string title;
  std::string description;
  unsigned points;
  std::string author;
  unsigned modified;
  unsigned created;
  std::string badgeName;
  int flags;
  std::string type;
  float rarity;
  float rarityHardcore;
  std::string badgeUrl;
  std::string badgeLockedUrl;
  int displayOrder;
};

inline void from_json(const nlohmann::json &j, Achievement &a) {
  j.at("ID").get_to(a.id);

  if (j.contains("MemAddr") && !j.at("MemAddr").is_null()) {
    j.at("MemAddr").get_to(a.memAddr);
  }

  if (j.contains("Title") && !j.at("Title").is_null()) {
    j.at("Title").get_to(a.title);
  }

  if (j.contains("Description") && !j.at("Description").is_null()) {
    j.at("Description").get_to(a.description);
  }

  if (j.contains("Points") && !j.at("Points").is_null()) {
    j.at("Points").get_to(a.points);
  }

  if (j.contains("Author") && !j.at("Author").is_null()) {
    j.at("Author").get_to(a.author);
  }

  if (j.contains("Modified") && !j.at("Modified").is_null()) {
    j.at("Modified").get_to(a.modified);
  }

  if (j.contains("Created") && !j.at("Created").is_null()) {
    j.at("Created").get_to(a.created);
  }

  if (j.contains("BadgeName") && !j.at("BadgeName").is_null()) {
    j.at("BadgeName").get_to(a.badgeName);
  }

  if (j.contains("Flags") && !j.at("Flags").is_null()) {
    j.at("Flags").get_to(a.flags);
  }

  if (j.contains("Type") && !j.at("Type").is_null()) {
    j.at("Type").get_to(a.type);
  }

  if (j.contains("Rarity") && !j.at("Rarity").is_null()) {
    j.at("Rarity").get_to(a.rarity);
  }

  if (j.contains("RarityHardcore") && !j.at("RarityHardcore").is_null()) {
    j.at("RarityHardcore").get_to(a.rarityHardcore);
  }

  if (j.contains("BadgeURL") && !j.at("BadgeURL").is_null()) {
    j.at("BadgeURL").get_to(a.badgeUrl);
  }

  if (j.contains("BadgeLockedURL") && !j.at("BadgeLockedURL").is_null()) {
    j.at("BadgeLockedURL").get_to(a.badgeLockedUrl);
  }
}

inline void to_json(nlohmann::json &j, const Achievement &a) {
  j = nlohmann::json{{"ID", a.id},
                     {"MemAddr", a.memAddr},
                     {"Title", a.title},
                     {"Description", a.description},
                     {"Points", a.points},
                     {"Author", a.author},
                     {"Modified", a.modified},
                     {"Created", a.created},
                     {"BadgeName", a.badgeName},
                     {"Flags", a.flags},
                     {"Type", a.type},
                     {"Rarity", a.rarity},
                     {"RarityHardcore", a.rarityHardcore},
                     {"BadgeURL", a.badgeUrl},
                     {"BadgeLockedURL", a.badgeLockedUrl}};
}
} // namespace firelight::achievements
