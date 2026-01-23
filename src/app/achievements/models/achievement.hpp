#pragma once

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
} // namespace firelight::achievements
