#pragma once

#include <optional>
#include <string>

namespace firelight::discord {

struct StoredAuthToken {
  std::string accessToken;
  std::string refreshToken;
};

// Persists the OAuth tokens between runs so sign-in is a one-time browser
// round-trip. An adapter-internal detail — nothing outside the Discord
// adapter sees tokens.
class ITokenStore {
public:
  virtual ~ITokenStore() = default;
  virtual std::optional<StoredAuthToken> load() = 0;
  virtual void save(const StoredAuthToken &token) = 0;
  virtual void clear() = 0;
};

class FileTokenStore final : public ITokenStore {
public:
  explicit FileTokenStore(std::string path);

  std::optional<StoredAuthToken> load() override;
  void save(const StoredAuthToken &token) override;
  void clear() override;

private:
  std::string m_path;
};

} // namespace firelight::discord
