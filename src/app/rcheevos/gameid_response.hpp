#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace firelight::achievements {
    struct GameIdResponse {
        bool Success{};
        int GameID{};
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GameIdResponse, Success, GameID)
}
