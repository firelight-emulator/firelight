#include "discord_manager.hpp"

#include <platform_metadata.hpp>
#include <spdlog/spdlog.h>

namespace firelight::discord {

void DiscordManager::initialize() {
  m_client.SetApplicationId(1208162396921929739);
  m_client.AddLogCallback(
      [](const auto &message, auto severity) { spdlog::info(message); },
      discordpp::LoggingSeverity::Info);

  m_defaultActivity.SetDetails("Chilling in the menus");
  m_client.UpdateRichPresence(
      m_defaultActivity, [](const discordpp::ClientResult &result) {
        if (result.Successful()) {
          spdlog::info("✅ Rich presence updated!");
        } else {
          spdlog::error("Failed to update rich presence: {}", (int)result);
        }
      });
}
void DiscordManager::runCallbacks() { discordpp::RunCallbacks(); }
void DiscordManager::startGameActivity(const std::string &contentHash,
                                       const std::string &displayName,
                                       int platformId) {
  discordpp::Activity activity;
  activity.SetType(discordpp::ActivityTypes::Playing);
  activity.SetDetails("Playing " + displayName);
  // activity.SetState(PlatformMetadata::getPlatformName(platformId));

  discordpp::ActivityAssets assets;
  assets.SetLargeImage(PlatformMetadata::getDiscordLargeImageName(platformId));
  assets.SetSmallImage("firelight-logo-white");
  activity.SetAssets(assets);

  m_client.UpdateRichPresence(
      activity, [](const discordpp::ClientResult &result) {
        if (result.Successful()) {
          spdlog::info("✅ Rich presence updated!");
        } else {
          spdlog::error("Failed to update rich presence: {}", (int)result);
        }
      });
}

void DiscordManager::clearActivity() {
  m_client.UpdateRichPresence(
      m_defaultActivity, [](const discordpp::ClientResult &result) {
        if (result.Successful()) {
          spdlog::info("✅ Rich presence updated!");
        } else {
          spdlog::error("Failed to update rich presence: {}", (int)result);
        }
      });
}
} // namespace firelight::discord