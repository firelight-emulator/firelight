#pragma once
#include <firelight/discord/discord_manager.hpp>

#include <discordpp.h>

namespace firelight::platforms {
class IPlatformService;
}

namespace firelight::discord {
class DiscordManager final : public IDiscordManager {
public:
  explicit DiscordManager(platforms::IPlatformService &platformService);

  void initialize() override;

  void runCallbacks() override;

  void startGameActivity(const std::string &contentHash, const std::string &displayName, int platformId,
                         const std::string &iconUrl) override;

  void clearActivity() override;

  void setRichPresenceMessage(const std::string &message) override;

  // The one process-wide SDK client; the lobby backend shares it
  discordpp::Client &client() { return m_client; }

private:
  platforms::IPlatformService &m_platformService;
  discordpp::Client m_client;
  discordpp::Activity m_defaultActivity{};

  discordpp::Activity m_currentActivity{};
  std::string m_richPresenceMessage;
};
} // namespace firelight::discord
