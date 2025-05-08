#pragma once

#include "rcheevos/rc_client.h"
#include <string>

namespace firelight::achievements {
    class IRetroAchievementsRequestHandler {
    public:
        virtual ~IRetroAchievementsRequestHandler() = default;

        virtual rc_api_server_response_t handleRequest(const std::string &url, const std::string &postBody,
                                                     const std::string &contentType) =
        0;
    };
} // namespace firelight::retroachievements

