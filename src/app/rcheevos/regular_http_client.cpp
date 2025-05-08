#include "regular_http_client.hpp"
#include "ra_constants.h"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace firelight::achievements {
rc_api_server_response_t
RegularHttpClient::handleRequest(const std::string &url,
                                 const std::string &postBody,
                                 const std::string &contentType) {
  spdlog::info("Online status: {}", m_online);
  const auto headers =
      cpr::Header{{"User-Agent", USER_AGENT}, {"Content-Type", contentType}};

  const auto response = Post(cpr::Url{url}, headers, cpr::Body{postBody});

  rc_api_server_response_t rcResponse;
  rcResponse.http_status_code = response.status_code;

  if (response.error) {
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
