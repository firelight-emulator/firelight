#pragma once

#include <string>
#include <optional>
#include <vector>
#include "cached_achievement.hpp"
#include "login2_response.hpp"
#include "patch_response.hpp"
#include "startsession_response.hpp"
#include "earned_achievement.hpp"

namespace firelight::achievements {
    class RetroAchievementsCache {
    public:
        std::optional<int> getGameIdFromAchievementId(int achievementId);

        std::optional<int> getGameIdFromHash(std::string hash);

        int getNumRemainingAchievements(const std::string &username, int gameId);

        bool awardAchievement(const std::string &username, const std::string &token, int achievementId,
                              bool hardcore);

        std::vector<CachedAchievement> getUserAchievements(std::string username, int gameId);

        int getUserScore(std::string username, bool hardcore);

        ~RetroAchievementsCache() = default;

        std::optional<PatchResponse> getPatchResponse(int gameId);

        void setLastLoginResponse(Login2Response response);

        void setPatchResponse(int gameId, PatchResponse patch);

        void setGameId(const std::string &hash, int id);

    private:
        std::unordered_map<int, StartSessionResponse> m_startSessionResponses;
        std::unordered_map<int, PatchResponse> m_patchResponses;
        std::unordered_map<std::string, int> m_gameIds;
        std::optional<Login2Response> m_lastLoginResponse;
        std::vector<EarnedAchievement> m_earnedAchievements;

        std::unordered_map<int, CachedAchievement> m_cachedAchievements;

        int m_lastKnownScore = 0;
        int m_lastKnownSoftcoreScore = 0;
    };
} // namespace firelight::retroachievements
