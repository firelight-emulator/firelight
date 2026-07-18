#pragma once

#include <SQLiteCpp/Database.h>
#include <firelight/settings/settings_repository.hpp>

namespace firelight::settings {

// TODO
// Persistence for the three settings tiers (global / platform / game). A plain
// class implementing the std-typed ISettingsRepository — no Qt. The QML layer
// goes through SettingsService, not this repository directly
class SqliteSettingsRepository : public ISettingsRepository {
public:
  explicit SqliteSettingsRepository(std::string databaseFile);
  ~SqliteSettingsRepository() override;

  std::optional<std::string> getGlobalValue(const std::string &key) override;
  bool setGlobalValue(const std::string &key, const std::string &value) override;
  bool resetGlobalValue(const std::string &key) override;

  std::optional<std::string> getPlatformValue(int platformId,
                                              const std::string &key) override;
  bool setPlatformValue(int platformId, const std::string &key,
                        const std::string &value) override;
  bool resetPlatformValue(int platformId, const std::string &key) override;

  std::optional<std::string> getGameValue(const std::string &contentHash,
                                          const std::string &key) override;
  bool setGameValue(const std::string &contentHash, const std::string &key,
                    const std::string &value) override;
  bool resetGameValue(const std::string &contentHash, const std::string &key) override;

  // Canonical resolution: game override -> platform override -> global -> the
  // caller's catalog default
  std::string getEffectiveValue(const std::string &contentHash, int platformId,
                                const std::string &key,
                                const std::string &defaultValue) {
    if (auto v = getGameValue(contentHash, key)) {
      return *v;
    }
    if (auto v = getPlatformValue(platformId, key)) {
      return *v;
    }
    if (auto v = getGlobalValue(key)) {
      return *v;
    }
    return defaultValue;
  }

private:
  std::string m_databaseFile;
  std::unique_ptr<SQLite::Database> m_database;
};

} // namespace firelight::settings
