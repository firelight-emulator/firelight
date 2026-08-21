#include <firelight/discord/token_store.hpp>

#include <cstdio>
#include <fstream>

namespace firelight::discord {

FileTokenStore::FileTokenStore(std::string path) : m_path(std::move(path)) {}

std::optional<StoredAuthToken> FileTokenStore::load() {
  std::ifstream file(m_path);
  if (!file.is_open()) {
    return std::nullopt;
  }
  StoredAuthToken token;
  std::getline(file, token.accessToken);
  std::getline(file, token.refreshToken);
  if (token.accessToken.empty()) {
    return std::nullopt;
  }
  return token;
}

void FileTokenStore::save(const StoredAuthToken &token) {
  std::ofstream file(m_path, std::ios::trunc);
  if (file.is_open()) {
    file << token.accessToken << "\n" << token.refreshToken << "\n";
  }
}

void FileTokenStore::clear() { std::remove(m_path.c_str()); }

} // namespace firelight::discord
