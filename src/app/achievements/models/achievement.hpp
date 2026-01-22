#pragma once

#include <string>

namespace firelight::achievements {
struct Achievement {
  unsigned id;
  std::string name;
  std::string description;
  std::string imageUrl;
  int points;
  std::string type;
  int displayOrder;
  unsigned gameId;
  int flags;
};
} // namespace firelight::achievements
