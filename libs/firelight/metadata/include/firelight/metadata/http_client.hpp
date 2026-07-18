#pragma once

#include <string>
#include <vector>

namespace firelight::metadata {

struct HttpHeader {
  std::string name;
  std::string value;
};

struct HttpResponse {
  long status = 0;
  std::string body;

  [[nodiscard]] bool ok() const { return status >= 200 && status < 300; }
};

// Minimal HTTP client seam so art providers can be unit-tested with a fake and
// the module stays free of a concrete HTTP dependency (cpr lives in the app)
class IHttpClient {
public:
  virtual ~IHttpClient() = default;

  [[nodiscard]] virtual HttpResponse
  get(const std::string &url, const std::vector<HttpHeader> &headers) = 0;
};

} // namespace firelight::metadata
