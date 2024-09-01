#include "RegularHttpClient.hpp"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "cache/login2_response.hpp"
#include "cache/gameid_response.hpp"
#include "cache/patch_response.hpp"
#include "cache/startsession_response.hpp"

namespace firelight::achievements {
    static const rc_api_server_response_t OFFLINE_RESPONSE = {
        "",
        0,
        500
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

    rc_api_server_response_t RegularHttpClient::sendRequest(const std::string &url, const std::string &postBody,
                                                            const std::string &contentType) {
        auto params = parseQueryParams(postBody);

        const auto headers = cpr::Header{
            {"User-Agent", "FirelightEmulator/1.0"},
            {"Content-Type", contentType}
        };

        // If online, just send the dang thing.
        if (m_online) {
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

            auto json = nlohmann::json::parse(rcResponse.body);
            if (params["r"] == "login2") {
                auto loginResponse = json.get<Login2Response>();
                m_cache->setLastLoginResponse(loginResponse);
            } else if (params["r"] == "gameid") {
                auto gameidResponse = json.get<GameIdResponse>();
                m_cache->setGameId(params["m"], gameidResponse.GameID);
            } else if (params["r"] == "patch") {
                auto patchResponse = json.get<PatchResponse>();
                m_cache->setPatchResponse(std::stoi(params["g"]), patchResponse);
            } else if (params["r"] == "startsession") {
                // nlohmann lib doesn't support optional/nullable fields...
                if (!json.contains("Unlocks") || !json.at("Unlocks").is_null()) {
                    json["Unlocks"] = std::vector<Unlock>();
                }

                if (!json.contains("HardcoreUnlocks") || !json.at("HardcoreUnlocks").is_null()) {
                    json["HardcoreUnlocks"] = std::vector<Unlock>();
                }

                auto startSessionResponse = json.get<StartSessionResponse>();
                m_cache->setStartSessionResponse(startSessionResponse);
            }

            return rcResponse;
        }

        // HANDLE OFFLINE REQUESTS

        if (params["r"] == "gameid") {
            auto cached = m_cache->getGameId(params["m"]);
            if (!cached.has_value()) {
                return OFFLINE_RESPONSE;
            }

            GameIdResponse gameIdResponse{
                .Success = true,
                .GameID = cached.value()
            };

            auto thing = nlohmann::json(gameIdResponse).dump();

            rc_api_server_response_t rcResponse;
            rcResponse.http_status_code = 200;
            rcResponse.body = thing.c_str();
            rcResponse.body_length = thing.size();
            return rcResponse;
        }

        if (params["r"] == "patch") {
            auto cached = m_cache->getPatchResponse(std::stoi(params["g"]));
            if (!cached.has_value()) {
                return OFFLINE_RESPONSE;
            }

            auto thing = nlohmann::json(cached.value()).dump();

            rc_api_server_response_t rcResponse;
            rcResponse.http_status_code = 200;
            rcResponse.body = thing.c_str();
            rcResponse.body_length = thing.size();
            return rcResponse;
        }

        if (params["r"] == "startsession") {
            // TODO
            // Get the last startsession response for the gameId
            // Go through any offline unlocks and add them to the response
            // Update the last startsession response
            // Return the response
        }

        if (params["r"] == "awardachievement") {
            // TODO
            // Cache the request
            // Return success
        }

        if (params["r"] == "login2") {
            // TODO
            // Get the last login response
            // Update the scores somehow??
            // Update the last login response
            // Return the response
        }

        if (params["r"] == "ping") {
            // TODO
            // Return success
        }

        return OFFLINE_RESPONSE;
    }
} // achievements
// firelight
