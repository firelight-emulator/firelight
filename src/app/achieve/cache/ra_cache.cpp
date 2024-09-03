#include "ra_cache.hpp"

namespace firelight::achievements {
    std::optional<int> RetroAchievementsCache::getGameIdFromAchievementId(int achievementId) {
        return std::nullopt;
    }

    std::optional<int> RetroAchievementsCache::getGameIdFromHash(std::string hash) {
        return std::nullopt;
    }

    int RetroAchievementsCache::getNumRemainingAchievements(const std::string &username, int gameId) {
        return 0;
    }

    bool RetroAchievementsCache::awardAchievement(const std::string &username, const std::string &token,
                                                  int achievementId, bool hardcore) {
        return false;
    }

    std::vector<CachedAchievement> RetroAchievementsCache::getUserAchievements(std::string username, int gameId) {
        return {};
    }

    int RetroAchievementsCache::getUserScore(std::string username, bool hardcore) {
        return 0;
    }

    // int DumbAchievementCache::awardAchievement(const int id, long long epochMillis, bool hardcore) {
    //     if (!m_cachedAchievements.contains(id)) {
    //         return -1;
    //     }
    //
    //     auto score = 0;
    //
    //     auto achievement = m_cachedAchievements[id];
    //     if (hardcore) {
    //         achievement.EarnedHardcore = true;
    //         achievement.WhenHardcore = epochMillis;
    //
    //         score = m_lastKnownScore;
    //     } else {
    //         achievement.Earned = true;
    //         achievement.When = epochMillis;
    //
    //         score = m_lastKnownSoftcoreScore;
    //     }
    //
    //     score += achievement.Points;
    //     return score;
    // }

    std::optional<PatchResponse> RetroAchievementsCache::getPatchResponse(const int gameId) {
        if (m_patchResponses.contains(gameId)) {
            return m_patchResponses[gameId];
        }

        return std::nullopt;
    }

    // void DumbAchievementCache::setStartSessionResponse(const int gameId, const StartSessionResponse &response) {
    //     m_startSessionResponses[gameId] = response;
    //     for (const auto a&: response.Unlocks) {
    //         if (m_cachedAchievements.contains(a.ID)) {
    //             m_cachedAchievements[a.ID].Earned = true;
    //             m_cachedAchievements[a.ID].When = a.When;
    //         }
    //     }
    //
    //     for (const auto a&: response.HardcoreUnlocks) {
    //         if (m_cachedAchievements.contains(a.ID)) {
    //             m_cachedAchievements[a.ID].EarnedHardcore = true;
    //             m_cachedAchievements[a.ID].WhenHardcore = a.When;
    //         }
    //     }
    // }

    void RetroAchievementsCache::setLastLoginResponse(Login2Response response) {
        m_lastLoginResponse = response;
    }

    void RetroAchievementsCache::setPatchResponse(const int gameId, const PatchResponse patch) {
        // m_patchResponses[gameId] = patch;
        // for (const auto a&: patch.PatchData.Achievements) {
        //     if (!m_cachedAchievements.contains(a.ID)) {
        //         m_cachedAchievements[a.ID] = CachedAchievement{
        //             .ID = a.ID,
        //             .Points = a.Points,
        //             .GameID = gameId,
        //             .Earned = false,
        //             .EarnedHardcore = false
        //         };
        //     }
        // }
    }

    void RetroAchievementsCache::setGameId(const std::string &hash, const int id) {
        m_gameIds[hash] = id;
    }
} // namespace firelight::achievements
