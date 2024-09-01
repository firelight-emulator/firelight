#include "ra_cache_dumb.hpp"

namespace firelight::achievements {
    std::optional<PatchResponse> DumbAchievementCache::getPatchResponse(int gameId) {
    }

    void DumbAchievementCache::setStartSessionResponse(StartSessionResponse response) {
    }

    void DumbAchievementCache::setLastLoginResponse(Login2Response response) {
    }

    void DumbAchievementCache::setPatchResponse(int gameId, PatchResponse patch) {
    }

    void DumbAchievementCache::setGameId(const std::string &hash, int id) {
    }

    std::optional<int> DumbAchievementCache::getGameId(const std::string &hash) {
        return std::nullopt;
    }

    void DumbAchievementCache::prepareEarnedAchievement(EarnedAchievement achievement) {
    }

    void DumbAchievementCache::releaseEarnedAchievement(int id) {
    }

    void DumbAchievementCache::commitAllEarnedAchievements() {
    }

    std::vector<EarnedAchievement> DumbAchievementCache::getEarnedAchievements() {
    }
} // namespace firelight::achievements
