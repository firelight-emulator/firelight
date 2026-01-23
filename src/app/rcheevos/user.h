#pragma once

#include <string>

namespace firelight::achievements {
class User {
public:
  std::string username;
  std::string avatarUrl;
  std::string token;
  int softcoreScore;
  int score;
};
} // namespace firelight::achievements