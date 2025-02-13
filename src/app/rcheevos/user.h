#pragma once

#include <string>

namespace firelight::achievements {
class User {
  public:
    std::string username;
    std::string token;
    int points;
    int hardcore_points;
};
}