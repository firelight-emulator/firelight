#include "regular_http_client.hpp"
#include "ra_constants.h"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace firelight::achievements {
rc_api_server_response_t
RegularHttpClient::sendRequest(const std::string &url,
                               const std::string &postBody,
                               const std::string &contentType) {
  spdlog::info("Online status: {}", m_online);
  const auto headers =
      cpr::Header{{"User-Agent", USER_AGENT}, {"Content-Type", contentType}};

  spdlog::info("Sending request to URL: {} with body {}", url, postBody);
  const auto response = Post(cpr::Url{url}, headers, cpr::Body{postBody});

  spdlog::info("response: {}", response.text);
  spdlog::info("response status code: {}", response.status_code);

  rc_api_server_response_t rcResponse;
  rcResponse.http_status_code = response.status_code;

  if (response.error) {
    // printf("Woah theah boah I said woah theah\n");
    // m_online = false;
    //  TODO: I don't love this buttttttt it might be fine.
    return m_offlineClient.handleRequest(url, postBody, contentType);
  }

  rcResponse.body = strdup(response.text.c_str());
  rcResponse.body_length = response.text.size();

  m_offlineClient.processResponse(postBody, response.text);

  return rcResponse;
}

void RegularHttpClient::setOnlineForTesting(const bool online) {
  m_online = online;
}
} // namespace firelight::achievements
