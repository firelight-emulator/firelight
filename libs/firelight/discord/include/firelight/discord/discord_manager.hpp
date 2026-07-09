#pragma once
#include <string>

namespace firelight::discord {
    class IDiscordManager {
    public:
        virtual ~IDiscordManager() = default;

        virtual void initialize() = 0;

        virtual void runCallbacks() = 0;

        virtual void startGameActivity(const std::string &contentHash,
                                       const std::string &displayName, int platformId,
                                       const std::string &iconUrl) = 0;

        virtual void clearActivity() = 0;

        virtual void setRichPresenceMessage(const std::string &message) = 0;
    };
} // namespace firelight::discord
