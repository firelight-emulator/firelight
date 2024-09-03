#pragma once

#include <memory>

#include "ra_http_client.hpp"
#include "offline/ra_cache.hpp"
#include "offline/rcheevos_offline_client.hpp"

namespace firelight::achievements {
    class RegularHttpClient final : public IRetroAchievementsHttpClient {
    public:
        explicit RegularHttpClient(const std::shared_ptr<RetroAchievementsOfflineClient> &offlineClient,
                                   const std::shared_ptr<RetroAchievementsCache> &cache) : m_cache(cache),
            m_offlineClient(offlineClient) {
        }

        rc_api_server_response_t sendRequest(const std::string &url, const std::string &postBody,
                                             const std::string &contentType) override;

        void setOnlineForTesting(bool online);

    private:
        bool m_online = true;
        std::shared_ptr<RetroAchievementsCache> m_cache = nullptr;
        std::shared_ptr<RetroAchievementsOfflineClient> m_offlineClient = nullptr;

        // std::vector<EarnedAchievement> m_earnedAchievements;
    };
} // namespace firelight::achievements
