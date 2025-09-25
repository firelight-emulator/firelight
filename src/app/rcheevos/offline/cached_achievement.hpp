#pragma once

namespace firelight::achievements {
    struct CachedAchievement {
        unsigned ID{};
        int GameID{};
        unsigned long long When{};
        unsigned long long WhenHardcore{};
        int Points{};
        bool Earned{};
        bool EarnedHardcore{};
        bool Synced{};
        bool SyncedHardcore{};
    };
}
