#define DISCORDPP_IMPLEMENTATION

#include <firelight/discord/discord_manager_impl.hpp>

#include <platform_metadata.hpp>
#include <spdlog/spdlog.h>

namespace firelight::discord {
  void DiscordManager::initialize() {
    m_client.SetApplicationId(1208162396921929739);
    m_client.AddLogCallback(
      [](const auto &message, auto severity) {
        switch (severity) {
          case discordpp::LoggingSeverity::Info:
            spdlog::info("[Discord] " + message);
            break;
          case discordpp::LoggingSeverity::Warning:
            spdlog::warn("[Discord] " + message);
            break;
          case discordpp::LoggingSeverity::Error:
            spdlog::error("[Discord] " + message);
            break;
          case discordpp::LoggingSeverity::Verbose:
            spdlog::debug("[Discord] " + message);
            break;
          default:
            spdlog::error("[Discord] " + message);
        }
      },
      discordpp::LoggingSeverity::Warning);

    m_defaultActivity.SetDetails("Chilling in the menus");
    m_client.UpdateRichPresence(
      m_defaultActivity, [](const discordpp::ClientResult &result) {
        if (result.Successful()) {
          spdlog::info("[Discord] Rich presence updated");
        } else {
          spdlog::error("[Discord] Failed to update rich presence: {}", (int) result);
        }
      });
  }

  void DiscordManager::runCallbacks() { discordpp::RunCallbacks(); }

  void DiscordManager::startGameActivity(const std::string &contentHash,
                                         const std::string &displayName,
                                         int platformId,
                                         const std::string &iconUrl) {
    m_currentActivity = discordpp::Activity{};
    m_currentActivity.SetType(discordpp::ActivityTypes::Playing);
    m_currentActivity.SetDetails("Playing " + displayName);

    discordpp::ActivityTimestamps timestamps;
    timestamps.SetStart(time(nullptr)); /// Set the start time to the current time
    m_currentActivity.SetTimestamps(timestamps);

    discordpp::ActivityAssets assets;
    if (iconUrl != "" && iconUrl.starts_with("http://") || iconUrl.starts_with("https://")) {
      assets.SetLargeImage(iconUrl);
    } else {
      assets.SetLargeImage(
        PlatformMetadata::getDiscordLargeImageName(platformId));
    }

    assets.SetSmallImage("firelight-logo-white");
    assets.SetSmallText("Firelight Emulator");
    m_currentActivity.SetAssets(assets);

    m_client.UpdateRichPresence(
      m_currentActivity, [](const discordpp::ClientResult &result) {
        if (result.Successful()) {
          spdlog::info("[Discord] Rich presence updated");
        } else {
          spdlog::error("[Discord] Failed to update rich presence: {}", static_cast<int>(result));
        }
      });
  }

  void DiscordManager::clearActivity() {
    m_client.UpdateRichPresence(
      m_defaultActivity, [](const discordpp::ClientResult &result) {
        if (result.Successful()) {
          spdlog::info("[Discord] Rich presence updated");
        } else {
          spdlog::error("[Discord] Failed to update rich presence: {}", static_cast<int>(result));
        }
      });
  }

  void DiscordManager::setRichPresenceMessage(const std::string &message) {
    m_currentActivity.SetState(message);

    m_client.UpdateRichPresence(
      m_currentActivity, [](const discordpp::ClientResult &result) {
        if (result.Successful()) {
          spdlog::info("Rich presence updated");
        } else {
          spdlog::error("Failed to update rich presence: {}", static_cast<int>(result));
        }
      });
  }
} // namespace firelight::discord
