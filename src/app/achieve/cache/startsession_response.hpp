#pragma once
#include <vector>
#include <nlohmann/json.hpp>

namespace firelight::achievements {
    struct Unlock {
        int ID{};
        long long When{};
    };

    struct StartSessionResponse {
        bool Success{};
        std::vector<Unlock> HardcoreUnlocks;
        std::vector<Unlock> Unlocks;
        long long ServerNow{};
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Unlock, ID, When)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StartSessionResponse, Success, HardcoreUnlocks, Unlocks, ServerNow)
} // namespace firelight::achievements
