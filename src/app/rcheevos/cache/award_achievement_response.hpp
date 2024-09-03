#pragma once

#include <nlohmann/json.hpp>

namespace firelight::achievements {
    struct AwardAchievementResponse {
        bool Success;
        int Score;
        int SoftcoreScore;
        int AchievementID;
        int AchievementsRemaining;
        // const char* server_response = "{\"Success\":true,\"Score\":119102,\"SoftcoreScore\":777,\"AchievementID\":56481,\"AchievementsRemaining\":11}";
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AwardAchievementResponse, Success, Score, SoftcoreScore, AchievementID,
                                       AchievementsRemaining)
} // namespace firelight::achievements
