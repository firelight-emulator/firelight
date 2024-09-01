#pragma once

#include <string>
#include <optional>
#include <vector>
#include "earned_achievement.hpp"
#include "login2_response.hpp"
#include "patch_response.hpp"
#include "startsession_response.hpp"

namespace firelight::achievements {
    class IAchievementCache {
    public:
        virtual ~IAchievementCache() = default;

        virtual void setGameId(const std::string &hash, int id) = 0;

        virtual std::optional<int> getGameId(const std::string &hash) = 0;

        virtual void setPatchResponse(int gameId, PatchResponse patch) = 0;

        virtual std::optional<PatchResponse> getPatchResponse(int gameId) = 0;

        virtual void setLastLoginResponse(Login2Response response) = 0;

        virtual void setStartSessionResponse(StartSessionResponse response) = 0;

        virtual void prepareEarnedAchievement(EarnedAchievement achievement) = 0;

        virtual void releaseEarnedAchievement(int id) = 0;

        virtual void commitAllEarnedAchievements() = 0;

        virtual std::vector<EarnedAchievement> getEarnedAchievements() = 0;
    };
} // namespace firelight::retroachievements
