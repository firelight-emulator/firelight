#include "regular_http_client.hpp"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "cache/award_achievement_response.hpp"
#include "cache/login2_response.hpp"
#include "cache/gameid_response.hpp"
#include "cache/patch_response.hpp"
#include "cache/startsession_response.hpp"

namespace firelight::achievements {
    rc_api_server_response_t RegularHttpClient::sendRequest(const std::string &url, const std::string &postBody,
                                                            const std::string &contentType) {
        if (m_online) {
            const auto headers = cpr::Header{
                {"User-Agent", "FirelightEmulator/1.0"},
                {"Content-Type", contentType}
            };

            const auto response = Post(cpr::Url{url}, headers, cpr::Body{postBody});

            rc_api_server_response_t rcResponse;
            rcResponse.http_status_code = response.status_code;

            if (response.error) {
                rcResponse.body = "";
                rcResponse.body_length = 0;
                return rcResponse;
            }

            rcResponse.body = strdup(response.text.c_str());
            rcResponse.body_length = response.text.size();

            m_offlineClient->processResponse(postBody, response.text);

            return rcResponse;
        }

        return m_offlineClient->handleRequest(url, postBody, contentType);

        // HANDLE OFFLINE REQUESTS

        // if (params["r"] == "gameid") {
        //     auto cached = m_cache->getGameIdFromAchievementId(params["m"]);
        //     if (!cached.has_value()) {
        //         return GENERIC_SERVER_ERROR;
        //     }
        //
        //     GameIdResponse gameIdResponse{
        //         .Success = true,
        //         .GameID = cached.value()
        //     };
        //
        //     auto thing = nlohmann::json(gameIdResponse).dump();
        //
        //     rc_api_server_response_t rcResponse;
        //     rcResponse.http_status_code = 200;
        //     rcResponse.body = strdup(thing.c_str());
        //     rcResponse.body_length = strlen(rcResponse.body);
        //     return rcResponse;
        // }
        //
        // if (params["r"] == "patch") {
        //     auto cached = m_cache->getPatchResponse(std::stoi(params["g"]));
        //     if (!cached.has_value()) {
        //         return GENERIC_SERVER_ERROR;
        //     }
        //
        //     auto thing = nlohmann::json(cached.value()).dump();
        //
        //     rc_api_server_response_t rcResponse;
        //     rcResponse.http_status_code = 200;
        //     rcResponse.body = strdup(thing.c_str());
        //     rcResponse.body_length = strlen(rcResponse.body);
        //     return rcResponse;
        // }
        //
        // if (params["r"] == "startsession") {
        //     auto duration = std::chrono::system_clock::now().time_since_epoch();
        //     auto epochMillis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        //
        //     StartSessionResponse startSessionResponse{
        //         .Success = true,
        //         .Unlocks = {},
        //         .HardcoreUnlocks = {},
        //         .ServerNow = epochMillis
        //     };
        //
        //     // auto allUnlocks = m_cache->getEarnedAchievements(std::stoi(params["g"]));
        //
        //
        //     auto cached = m_cache->getStartSessionResponse(std::stoi(params["g"]));
        //     if (!cached.has_value()) {
        //         return GENERIC_SERVER_ERROR;
        //     }
        //
        //     // TODO:
        //     // Go through any offline unlocks and add them to the response
        //     // Update the last startsession response
        //     // Probably make our own response...
        //
        //     auto thing = nlohmann::json(cached.value()).dump();
        //
        //     rc_api_server_response_t rcResponse;
        //     rcResponse.http_status_code = 200;
        //     rcResponse.body = strdup(thing.c_str());
        //     rcResponse.body_length = strlen(rcResponse.body);
        //     return rcResponse;
        // }
        //
        // if (params["r"] == "awardachievement") {
        //     auto duration = std::chrono::system_clock::now().time_since_epoch();
        //     auto epochMillis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        //
        //     auto gameId = 0;
        //
        //     EarnedAchievement achieve{
        //         .id = stoi(params["a"]),
        //         .gameId = gameId,
        //         .earnedTimestampEpochMs = epochMillis,
        //         .hash = params.contains("m") ? params["m"] : "",
        //         .hardcore = params.contains("h") && params["h"] == "1"
        //     };
        //
        //     m_earnedAchievements.emplace_back(achieve);
        //     m_cache->awardAchievement(stoi(params["a"]), epochMillis, params.contains("h") && params["h"] == "1");
        //
        //     AwardAchievementResponse resp{
        //         .Success = true,
        //         .Score = m_cache->getLastKnownScore(),
        //         .SoftcoreScore = m_cache->getLastKnownSoftcoreScore(),
        //         .AchievementID = stoi(params["a"]),
        //         .AchievementsRemaining = m_cache->getNumRemainingAchievements(gameId)
        //     };
        //
        //     auto thing = nlohmann::json(resp).dump();
        //
        //     rc_api_server_response_t rcResponse;
        //     rcResponse.http_status_code = 200;
        //     rcResponse.body = strdup(thing.c_str());
        //     rcResponse.body_length = strlen(rcResponse.body);
        //     return rcResponse;
        // }
        //
        // if (params["r"] == "login2") {
        //     // TODO
        //     // Get the last login response
        //     // Update the scores somehow??
        //     // Update the last login response
        //     // Return the response
        // }
        //
        // if (params["r"] == "ping") {
        //     return GENERIC_SUCCESS;
        // }
        //
        // if (params["r"] == "submitlbentry") {
        //     return GENERIC_SERVER_ERROR;
        // }
        //
        // return GENERIC_SERVER_ERROR;
    }

    void RegularHttpClient::setOnlineForTesting(bool online) {
        m_online = online;
    }
} // achievements
// firelight
