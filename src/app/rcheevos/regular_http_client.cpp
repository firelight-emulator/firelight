#include "regular_http_client.hpp"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

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
    }

    void RegularHttpClient::setOnlineForTesting(const bool online) {
        m_online = online;
    }
} // firelight::achievements
