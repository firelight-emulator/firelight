#pragma once

#include <memory>
#include <unordered_map>

#include "ra_http_client.hpp"
#include "cache/ra_cache.hpp"

namespace firelight::achievements {
    class RegularHttpClient final : public IRetroAchievementsHttpClient {
    public:
        explicit RegularHttpClient(const std::shared_ptr<IAchievementCache> &cache) : m_cache(cache) {
        }

        rc_api_server_response_t sendRequest(const std::string &url, const std::string &postBody,
                                             const std::string &contentType) override;

    private:
        bool m_online = true;
        std::shared_ptr<IAchievementCache> m_cache = nullptr;
        std::unordered_map<std::string, std::string> gameidResponsesByHash;
        std::unordered_map<std::string, std::string> patchesByGameId;
    };
} // namespace firelight::achievements
