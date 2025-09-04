#include "sqlite_settings_repository.hpp"

#include <spdlog/spdlog.h>
#include <utility>

constexpr auto EMULATION_SETTINGS_DATABASE_PREFIX = "emulation_settings_";

namespace firelight {
settings::SqliteSettingsRepository::SqliteSettingsRepository(
    std::string databaseFile)
    : m_databaseFile(std::move(databaseFile)) {
  m_database = std::make_unique<SQLite::Database>(
      m_databaseFile, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

  const std::string setupQueryString = R"(
            CREATE TABLE IF NOT EXISTS platform_settings (
                platform_id INTEGER NOT NULL,
                key TEXT NOT NULL,
                value TEXT NOT NULL,
                PRIMARY KEY (platform_id, key)
            );

            CREATE TABLE IF NOT EXISTS game_settings (
                content_hash TEXT NOT NULL,
                platform_id INTEGER NOT NULL,
                key TEXT NOT NULL,
                value TEXT NOT NULL,
                PRIMARY KEY (content_hash, platform_id, key)
            );

            UPDATE game_settings SET platform_id = -1 WHERE platform_id != -1;

            CREATE TABLE IF NOT EXISTS game_setting_levels (
                content_hash TEXT NOT NULL,
                level INTEGER NOT NULL,
                PRIMARY KEY (content_hash)
            );
        )";

  m_database->exec(setupQueryString);
}
settings::SqliteSettingsRepository::~SqliteSettingsRepository() = default;
settings::SettingsLevel settings::SqliteSettingsRepository::getSettingsLevel(
    const std::string contentHash) {
  try {
    SQLite::Statement query(*m_database,
                            "SELECT level FROM game_setting_levels WHERE "
                            "content_hash = :contentHash");
    query.bind(":contentHash", contentHash);

    if (query.executeStep()) {
      int level = query.getColumn(0);
      return static_cast<SettingsLevel>(level);
    }

    // Default to Platform level if no entry found
    return Platform;
  } catch (const std::exception &e) {
    spdlog::error("Failed to get settings level: {}", e.what());
    return Platform;
  }
}
bool settings::SqliteSettingsRepository::setSettingsLevel(
    const std::string contentHash, const SettingsLevel level) {
  try {
    SQLite::Statement query(
        *m_database, "INSERT OR REPLACE INTO game_setting_levels "
                     "(content_hash, level) VALUES (:contentHash, :level)");
    query.bind(":contentHash", contentHash);
    query.bind(":level", level);

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to set settings level: {}", e.what());
    return false;
  }
}

std::optional<std::string>
settings::SqliteSettingsRepository::getPlatformValue(const int platformId,
                                                     const std::string &key) {
  try {
    SQLite::Statement query(*m_database,
                            "SELECT value FROM platform_settings WHERE "
                            "platform_id = :platformId AND key = :key");
    query.bind(":platformId", platformId);
    query.bind(":key", key);

    if (query.executeStep()) {
      std::string value = query.getColumn(0);
      return value;
    }

    return std::nullopt;
  } catch (const std::exception &e) {
    spdlog::error("Failed to get platform value: {}", e.what());
    return std::nullopt;
  }
}

std::string settings::SqliteSettingsRepository::getEffectiveValue(
    const std::string &contentHash, const int platformId,
    const std::string &key, const std::string &defaultValue) {
  if (auto v = getGameValue(contentHash, key)) {
    return *v;
  }

  if (auto v = getPlatformValue(platformId, key)) {
    return *v;
  }

  return defaultValue;
}

bool settings::SqliteSettingsRepository::setPlatformValue(
    const int platformId, const std::string &key, const std::string &value) {
  try {
    SQLite::Statement query(
        *m_database,
        "INSERT OR REPLACE INTO platform_settings "
        "(platform_id, key, value) VALUES (:platformId, :key, :value)");
    query.bind(":platformId", platformId);
    query.bind(":key", key);
    query.bind(":value", value);

    query.exec();

    emit platformValueChanged(platformId, QString::fromStdString(key),
                              QString::fromStdString(value));
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to set platform value: {}", e.what());
    return false;
  }
}

bool settings::SqliteSettingsRepository::resetPlatformValue(
    int platformId, const std::string &key) {
  try {
    SQLite::Statement query(*m_database,
                            "DELETE FROM platform_settings WHERE platform_id = "
                            ":platformId AND key = :key");
    query.bind(":platformId", platformId);
    query.bind(":key", key);

    query.exec();

    emit platformValueReset(platformId, QString::fromStdString(key));
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to reset platform value: {}", e.what());
    return false;
  }
}

std::optional<std::string>
settings::SqliteSettingsRepository::getGameValue(const std::string &contentHash,
                                                 const std::string &key) {
  try {
    SQLite::Statement query(
        *m_database,
        "SELECT value FROM game_settings WHERE content_hash = :contentHash AND "
        "platform_id = :platformId AND key = :key");
    query.bind(":contentHash", contentHash);
    query.bind(":platformId", -1);
    query.bind(":key", key);

    if (query.executeStep()) {
      std::string value = query.getColumn(0);
      return value;
    }

    return std::nullopt;
  } catch (const std::exception &e) {
    spdlog::error("Failed to get game value: {}", e.what());
    return std::nullopt;
  }
}
bool settings::SqliteSettingsRepository::setGameValue(
    const std::string &contentHash, const std::string &key,
    const std::string &value) {
  try {
    SQLite::Statement query(*m_database,
                            "INSERT OR REPLACE INTO game_settings "
                            "(content_hash, platform_id, key, value) VALUES "
                            "(:contentHash, :platformId, :key, :value)");
    query.bind(":contentHash", contentHash);
    query.bind(":platformId", -1);
    query.bind(":key", key);
    query.bind(":value", value);

    query.exec();

    emit gameValueChanged(QString::fromStdString(contentHash), -1,
                          QString::fromStdString(key),
                          QString::fromStdString(value));
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to set game value: {}", e.what());
    return false;
  }
}
bool settings::SqliteSettingsRepository::resetGameValue(
    const std::string &contentHash, const std::string &key) {
  try {
    SQLite::Statement query(
        *m_database,
        "DELETE FROM game_settings WHERE content_hash = :contentHash AND "
        "platform_id = :platformId AND key = :key");
    query.bind(":contentHash", contentHash);
    query.bind(":platformId", -1);
    query.bind(":key", key);

    query.exec();

    emit gameValueReset(QString::fromStdString(contentHash), -1,
                        QString::fromStdString(key));
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to reset game value: {}", e.what());
    return false;
  }
}
} // namespace firelight