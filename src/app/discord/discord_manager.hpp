#pragma once
#include <discordpp.h>

namespace firelight::discord {

class DiscordManager {
public:
  void initialize();
  void runCallbacks();
  void startGameActivity(const std::string &contentHash,
                         const std::string &displayName, int platformId,
                         const std::string &iconUrl);
  void clearActivity();

private:
  discordpp::Client m_client;
  discordpp::Activity m_defaultActivity{};
};

} // namespace firelight::discord
// firelight