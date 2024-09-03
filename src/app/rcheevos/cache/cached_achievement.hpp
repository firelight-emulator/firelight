#pragma once

namespace firelight::achievements {
    struct CachedAchievement {
        int ID{};
        int GameID{};
        long long When{};
        long long WhenHardcore{};
        int Points{};
        bool Earned{};
        bool EarnedHardcore{};
    };
}
