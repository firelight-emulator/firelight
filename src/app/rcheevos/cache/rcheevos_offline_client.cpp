#include "rcheevos_offline_client.hpp"

#include <unordered_map>
#include "cached_achievement.hpp"
#include <sstream>
#include "award_achievement_response.hpp"
#include "login2_response.hpp"
#include "gameid_response.hpp"
#include "patch_response.hpp"
#include "startsession_response.hpp"

namespace firelight::achievements {
    static const std::string GAMEID = "gameid";
    static const std::string PATCH = "patch";
    static const std::string START_SESSION = "startsession";
    static const std::string AWARD_ACHIEVEMENT = "awardachievement";
    static const std::string LOGIN2 = "login2";
    static const std::string PING = "ping";
    static const std::string SUBMIT_LB_ENTRY = "submitlbentry";

    static const rc_api_server_response_t GENERIC_SERVER_ERROR = {
        "",
        0,
        500
    };

    static const rc_api_server_response_t GENERIC_SUCCESS = {
        "{\"Success\":true}",
        16,
        200
    };

    std::unordered_map<std::string, std::string> parseQueryParams(const std::string &query) {
        std::unordered_map<std::string, std::string> params;
        std::istringstream stream(query);
        std::string param;

        while (std::getline(stream, param, '&')) {
            std::size_t pos = param.find('=');
            if (pos != std::string::npos) {
                std::string key = param.substr(0, pos);
                std::string value = param.substr(pos + 1);
                params[key] = value;
            }
        }

        return params;
    }

    rc_api_server_response_t RetroAchievementsOfflineClient::handleRequest(const std::string &url,
                                                                           const std::string &postBody,
                                                                           const std::string &contentType) {
        auto params = parseQueryParams(postBody);
        // const auto duration = std::chrono::system_clock::now().time_since_epoch();
        // const auto nowEpochMillis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        if (params["r"] == GAMEID) {
            return handleGameIdRequest(params["m"]);
        }

        if (params["r"] == PATCH) {
            return handlePatchRequest(stoi(params["g"]));
        }

        if (params["r"] == START_SESSION) {
            const auto user = params["u"];
            const auto gameId = stoi(params["g"]);
            const auto hardcore = params["h"] == "1";

            return handleStartSessionRequest(user, gameId, hardcore);
        }

        if (params["r"] == AWARD_ACHIEVEMENT) {
            const auto user = params["u"];
            const auto token = params["t"];
            const auto achievementId = stoi(params["a"]);
            const auto hardcore = params["h"] == "1";

            return handleAwardAchievementRequest(user, token, achievementId, hardcore);
        }

        if (params["r"] == LOGIN2) {
            // TODO
            // Get the last login response
            // Update the scores somehow??
            // Update the last login response
            // Return the response
        }

        if (params["r"] == PING) {
            return handlePingRequest();
        }

        if (params["r"] == SUBMIT_LB_ENTRY) {
            return handleSubmitLbEntryRequest();
        }

        return GENERIC_SERVER_ERROR;
    }

    void RetroAchievementsOfflineClient::processResponse(const std::string &request, const std::string &response) {
        // auto params = parseQueryParams(request);
        // auto json = nlohmann::json::parse(response);
        //
        // if (params["r"] == "login2") {
        //     auto loginResponse = json.get<Login2Response>();
        //     m_cache->setLastLoginResponse(loginResponse);
        //
        //     if (json.contains("Score") && json["Score"].is_number()) {
        //         m_cache->setLastKnownScore(json["Score"]);
        //     }
        //
        //     if (json.contains("SoftcoreScore") && json["SoftcoreScore"].is_number()) {
        //         m_cache->setLastKnownSoftcoreScore(json["SoftcoreScore"]);
        //     }
        // } else if (params["r"] == "gameid") {
        //     auto gameidResponse = json.get<GameIdResponse>();
        //     m_cache->setGameId(params["m"], gameidResponse.GameID);
        // } else if (params["r"] == "patch") {
        //     auto patchResponse = json.get<PatchResponse>();
        //     m_cache->setPatchResponse(std::stoi(params["g"]), patchResponse);
        // } else if (params["r"] == "startsession") {
        //     // nlohmann lib doesn't support optional/nullable fields...
        //     if (!json.contains("Unlocks") || !json.at("Unlocks").is_null()) {
        //         json["Unlocks"] = std::vector<Unlock>();
        //     }
        //
        //     if (!json.contains("HardcoreUnlocks") || !json.at("HardcoreUnlocks").is_null()) {
        //         json["HardcoreUnlocks"] = std::vector<Unlock>();
        //     }
        //
        //     auto startSessionResponse = json.get<StartSessionResponse>();
        //     m_cache->setStartSessionResponse(stoi(params["g"]), startSessionResponse);
        // } else if (params["r"] == "awardachievement") {
        //     if (json.contains("Score") && json["Score"].is_number()) {
        //         m_cache->setLastKnownScore(json["Score"]);
        //     }
        //
        //     if (json.contains("SoftcoreScore") && json["SoftcoreScore"].is_number()) {
        //         m_cache->setLastKnownSoftcoreScore(json["SoftcoreScore"]);
        //     }
        // }
    }

    rc_api_server_response_t RetroAchievementsOfflineClient::handleGameIdRequest(const std::string &hash) const {
        const auto cached = m_cache->getGameIdFromHash(hash);
        if (!cached.has_value()) {
            return GENERIC_SERVER_ERROR;
        }

        GameIdResponse gameIdResponse{
            .Success = true,
            .GameID = cached.value()
        };

        const auto json = nlohmann::json(gameIdResponse).dump();

        rc_api_server_response_t rcResponse;
        rcResponse.http_status_code = 200;
        rcResponse.body = strdup(json.c_str());
        rcResponse.body_length = strlen(rcResponse.body);
        return rcResponse;
    }

    rc_api_server_response_t RetroAchievementsOfflineClient::handlePatchRequest(const int gameId) const {
        auto cached = m_cache->getPatchResponse(gameId);
        if (!cached.has_value()) {
            return GENERIC_SERVER_ERROR;
        }

        const auto json = nlohmann::json(cached.value()).dump();

        rc_api_server_response_t rcResponse;
        rcResponse.http_status_code = 200;
        rcResponse.body = strdup(json.c_str());
        rcResponse.body_length = strlen(rcResponse.body);
        return rcResponse;
    }

    rc_api_server_response_t RetroAchievementsOfflineClient::handleStartSessionRequest(
        const std::string &username, const int gameId, bool hardcore) {
        const auto duration = std::chrono::system_clock::now().time_since_epoch();
        const auto epochMillis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        StartSessionResponse startSessionResponse{
            .Success = true,
            .HardcoreUnlocks = {},
            .Unlocks = {},
            .ServerNow = epochMillis
        };

        for (auto &achieve: m_cache->getUserAchievements(username, gameId)) {
            if (achieve.Earned) {
                startSessionResponse.Unlocks.emplace_back(Unlock{
                    .ID = achieve.ID,
                    .When = achieve.When
                });
            }

            if (achieve.EarnedHardcore) {
                startSessionResponse.HardcoreUnlocks.emplace_back(Unlock{
                    .ID = achieve.ID,
                    .When = achieve.WhenHardcore
                });
            }
        }

        const auto json = nlohmann::json(startSessionResponse).dump();

        rc_api_server_response_t rcResponse;
        rcResponse.http_status_code = 200;
        rcResponse.body = strdup(json.c_str());
        rcResponse.body_length = strlen(rcResponse.body);
        return rcResponse;
    }

    rc_api_server_response_t RetroAchievementsOfflineClient::handleAwardAchievementRequest(const std::string &username,
        const std::string &token, const int achievementId, const bool hardcore) const {
        m_cache->awardAchievement(username, token, achievementId, hardcore);

        auto gameId = 0;

        const auto cachedGameId = m_cache->getGameIdFromAchievementId(achievementId);
        if (cachedGameId.has_value()) {
            gameId = cachedGameId.value();
        }

        AwardAchievementResponse resp{
            .Success = true,
            .Score = m_cache->getUserScore(username, true),
            .SoftcoreScore = m_cache->getUserScore(username, false),
            .AchievementID = achievementId,
            .AchievementsRemaining = m_cache->getNumRemainingAchievements(username, gameId)
        };

        const auto json = nlohmann::json(resp).dump();

        rc_api_server_response_t rcResponse;
        rcResponse.http_status_code = 200;
        rcResponse.body = strdup(json.c_str());
        rcResponse.body_length = strlen(rcResponse.body);
        return rcResponse;
    }

    rc_api_server_response_t RetroAchievementsOfflineClient::handlePingRequest() {
        return GENERIC_SUCCESS;
    }

    rc_api_server_response_t RetroAchievementsOfflineClient::handleSubmitLbEntryRequest() {
        return GENERIC_SUCCESS;
    }
}
