#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace firelight::achievements {
    struct Login2Response {
        std::string AccountType{};
        int Messages{};
        int Permissions{};
        int Score{};
        int SoftcoreScore{};
        bool Success{};
        std::string Token{};
        std::string User{};
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Login2Response, AccountType, Messages, Permissions, Score, SoftcoreScore,
                                       Success, Token, User)
}
