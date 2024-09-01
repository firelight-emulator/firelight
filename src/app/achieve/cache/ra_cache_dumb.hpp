#pragma once

#include <string>
#include <optional>
#include <vector>
#include "earned_achievement.hpp"
#include "ra_cache.hpp"

namespace firelight::achievements {
    class DumbAchievementCache : public IAchievementCache {
    public:
        std::optional<PatchResponse> getPatchResponse(int gameId) override;

        void setStartSessionResponse(StartSessionResponse response) override;

        void setLastLoginResponse(Login2Response response) override;

        void setPatchResponse(int gameId, PatchResponse patch) override;

        ~DumbAchievementCache() override = default;

        void setGameId(const std::string &hash, int id) override;

        std::optional<int> getGameId(const std::string &hash) override;

        void prepareEarnedAchievement(EarnedAchievement achievement) override;

        void releaseEarnedAchievement(int id) override;

        void commitAllEarnedAchievements() override;

        std::vector<EarnedAchievement> getEarnedAchievements() override;

    private:
        std::vector<EarnedAchievement> m_earnedAchievements;
    };
} // namespace firelight::retroachievements
