#pragma once

#include <firelight/metadata/http_client.hpp>

namespace firelight::metadata {

// TODO
// cpr-backed IHttpClient. Lives in the app layer so the firelight_metadata
// module (and its art providers) stay free of a concrete HTTP dependency and
// remain unit-testable with a fake client
class CprHttpClient final : public IHttpClient {
public:
  [[nodiscard]] HttpResponse
  get(const std::string &url,
      const std::vector<HttpHeader> &headers) override;
};

} // namespace firelight::metadata
