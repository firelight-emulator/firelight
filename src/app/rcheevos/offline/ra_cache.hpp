#pragma once

#include <string>
#include <optional>
#include <QSqlDatabase>
#include <vector>
#include "cached_achievement.hpp"
#include "../login2_response.hpp"
#include "../patch_response.hpp"
#include "../startsession_response.hpp"
#include "earned_achievement.hpp"

namespace firelight::achievements {
    class RetroAchievementsCache {
    public:
        explicit RetroAchievementsCache(std::filesystem::path dbFile);

        ~RetroAchievementsCache();

        std::optional<int> getGameIdFromAchievementId(int achievementId) const;

        std::optional<int> getGameIdFromHash(const std::string &hash) const;

        int getNumRemainingAchievements(const std::string &username, int gameId, bool hardcore) const;

        bool awardAchievement(const std::string &username, const std::string &token, int achievementId,
                              bool hardcore) const;

        bool markAchievementUnlocked(const std::string &username, int achievementId, bool hardcore,
                                     long long epochMillis) const;

        std::vector<CachedAchievement> getUserAchievements(const std::string &username, int gameId) const;

        int getUserScore(const std::string &username, bool hardcore) const;

        std::optional<PatchResponse> getPatchResponse(int gameId) const;

        void setPatchResponse(const std::string &username, int gameId, const PatchResponse &patch) const;

        void setGameId(const std::string &hash, int id) const;

        void setUserScore(const std::string &username, int score, bool hardcore) const;

    private:
        std::unordered_map<int, StartSessionResponse> m_startSessionResponses;
        std::unordered_map<int, PatchResponse> m_patchResponses;
        std::unordered_map<std::string, int> m_gameIds;
        std::optional<Login2Response> m_lastLoginResponse;
        std::vector<EarnedAchievement> m_earnedAchievements;

        std::unordered_map<int, CachedAchievement> m_cachedAchievements;

        int m_lastKnownScore = 0;
        int m_lastKnownSoftcoreScore = 0;

        std::filesystem::path databaseFile;

        [[nodiscard]] QSqlDatabase getDatabase() const;
    };
} // namespace firelight::retroachievements
