#pragma once
#include <cstdint>
#include <string>

namespace firelight::achievements {
    struct EarnedAchievement {
        int id;
        int gameId;
        uint64_t earnedTimestampEpochMs;
        std::string hash;
        bool hardcore;
    };
} // namespace firelight::retroachievements
