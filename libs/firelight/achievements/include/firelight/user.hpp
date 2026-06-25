#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace firelight::achievements {
class User {
public:
  std::string username;
  std::string displayName;
  std::string avatarUrl;
  std::string token;
  unsigned score;
  unsigned softcoreScore;
  unsigned messages;
  unsigned permissions;
  std::string accountType;
};

inline void from_json(const nlohmann::json &j, User &u) {
  j.at("User").get_to(u.username);
  j.at("Token").get_to(u.token);

  if (j.contains("AvatarUrl") && !j.at("AvatarUrl").is_null()) {
    j.at("AvatarUrl").get_to(u.avatarUrl);
  }

  if (j.contains("Score") && !j.at("Score").is_null()) {
    j.at("Score").get_to(u.score);
  }

  if (j.contains("SoftcoreScore") && !j.at("SoftcoreScore").is_null()) {
    j.at("SoftcoreScore").get_to(u.softcoreScore);
  }

  if (j.contains("Messages") && !j.at("Messages").is_null()) {
    j.at("Messages").get_to(u.messages);
  }

  if (j.contains("Permissions") && !j.at("Permissions").is_null()) {
    j.at("Permissions").get_to(u.permissions);
  }

  if (j.contains("AccountType") && !j.at("AccountType").is_null()) {
    j.at("AccountType").get_to(u.accountType);
  }
}

inline void to_json(nlohmann::json &j, const User &u) {
  j = nlohmann::json{{"User", u.username},
                     {"Token", u.token},
                     {"AvatarUrl", u.avatarUrl},
                     {"Score", u.score},
                     {"SoftcoreScore", u.softcoreScore},
                     {"Messages", u.messages},
                     {"Permissions", u.permissions},
                     {"AccountType", u.accountType}};
}
} // namespace firelight::achievements