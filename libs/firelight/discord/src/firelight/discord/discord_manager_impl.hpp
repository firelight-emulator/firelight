#pragma once
#include <discordpp.h>
#include <firelight/discord/idiscord_manager.hpp>

namespace firelight::discord {

class DiscordManager final : public IDiscordManager {
public:
  void initialize() override;
  void runCallbacks() override;
  void startGameActivity(const std::string &contentHash,
                         const std::string &displayName, int platformId,
                         const std::string &iconUrl) override;
  void clearActivity() override;

private:
  discordpp::Client m_client;
  discordpp::Activity m_defaultActivity{};
};

} // namespace firelight::discord
