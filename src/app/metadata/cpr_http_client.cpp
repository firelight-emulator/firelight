#include "metadata/cpr_http_client.hpp"

#include <cpr/cpr.h>

namespace firelight::metadata {

HttpResponse CprHttpClient::get(const std::string &url,
                                const std::vector<HttpHeader> &headers) {
  cpr::Header cprHeaders;
  for (const auto &header : headers) {
    cprHeaders[header.name] = header.value;
  }

  const auto response =
      cpr::Get(cpr::Url{url}, cprHeaders, cpr::Timeout{15000});

  HttpResponse result;
  // A transport-level failure leaves status 0 (ok() == false); callers treat
  // that the same as an HTTP error and fall back to an empty result set
  if (response.error) {
    return result;
  }
  result.status = response.status_code;
  result.body = response.text;
  return result;
}

} // namespace firelight::metadata
