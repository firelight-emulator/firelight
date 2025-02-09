#pragma once

#include "ra_cache.hpp"
#include "../ra_http_client.hpp"

namespace firelight::achievements {
    class RetroAchievementsOfflineClient final {
    public:
        explicit RetroAchievementsOfflineClient(RetroAchievementsCache &cache) : m_cache(cache) {
        }

        rc_api_server_response_t handleRequest(const std::string &url, const std::string &postBody,
                                               const std::string &contentType);

        void processResponse(const std::string &request, const std::string &response) const;

        void syncOfflineAchievements();

        void clearSessionAchievements();

        void startOnlineHardcoreSession();

    private:
        rc_api_server_response_t handleGameIdRequest(const std::string &hash) const;

        rc_api_server_response_t handlePatchRequest(int gameId) const;

        rc_api_server_response_t
        handleStartSessionRequest(const std::string &username, int gameId, bool hardcore) const;

        rc_api_server_response_t handleAwardAchievementRequest(const std::string &username, const std::string &token,
                                                               int achievementId,
                                                               bool hardcore);

        rc_api_server_response_t handleLogin2Response(const std::string &username, const std::string &password,
                                                      const std::string &token) const;

        rc_api_server_response_t handlePingRequest();

        rc_api_server_response_t handleSubmitLbEntryRequest();

        void processLogin2Response(const std::string &username, const std::string &token, const std::string &response) const;

        void processGameIdResponse(const std::string &hash, const std::string &response) const;

        void processPatchResponse(const std::string &username, int gameId, const std::string &response) const;

        void processStartSessionResponse(const std::string &username, int gameId, const std::string &response) const;

        void processAwardAchievementResponse(const std::string &username, bool hardcore,
                                             const std::string &response) const;

        bool m_inHardcoreSession = false;
        RetroAchievementsCache &m_cache;
        std::vector<CachedAchievement> m_currentSessionAchievements{};
    };
}
