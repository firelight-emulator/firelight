#pragma once

#include "offline/rcheevos_offline_client.hpp"
#include "ra_http_client.hpp"

namespace firelight::achievements {
class RegularHttpClient final : public IRetroAchievementsRequestHandler {
public:
  explicit RegularHttpClient(RetroAchievementsOfflineClient &offlineClient)
      : m_offlineClient(offlineClient) {}

  rc_api_server_response_t
  handleRequest(const std::string &url, const std::string &postBody,
                const std::string &contentType) override;

  void setOnlineForTesting(bool online);

private:
  bool m_online = true;
  RetroAchievementsOfflineClient &m_offlineClient;

  // std::vector<EarnedAchievement> m_earnedAchievements;
};
} // namespace firelight::achievements
