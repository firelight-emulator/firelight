#include "firelight/settings/sqlite_settings_repository.hpp"

#include <firelight/migrations/migration_runner.hpp>

#include <SQLiteCpp/Transaction.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::settings {

SqliteSettingsRepository::SqliteSettingsRepository(std::string databaseFile) : m_databaseFile(std::move(databaseFile)) {
  m_database = std::make_unique<SQLite::Database>(m_databaseFile, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

  // Forward-only schema migrations (see migration_runner). A future change adds
  // the next-numbered migration
  const std::vector<migrations::Migration> schema = {
      {1,
       [this] {
         m_database->exec(R"(
           CREATE TABLE IF NOT EXISTS global_settings (
               key TEXT NOT NULL,
               value TEXT NOT NULL,
               PRIMARY KEY (key)
           );
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
           CREATE TABLE IF NOT EXISTS core_options (
               core_name TEXT NOT NULL,
               key TEXT NOT NULL,
               label TEXT NOT NULL,
               description TEXT NOT NULL,
               default_value TEXT NOT NULL,
               values_json TEXT NOT NULL,
               category_key TEXT NOT NULL DEFAULT '',
               category_label TEXT NOT NULL DEFAULT '',
               position INTEGER NOT NULL,
               PRIMARY KEY (core_name, key)
           );
         )");
       }},
  };

  try {
    SQLite::Transaction transaction(*m_database);
    const int currentVersion = m_database->execAndGet("PRAGMA user_version").getInt();
    migrations::applyMigrations(currentVersion, schema, [this](const int v) {
      m_database->exec("PRAGMA user_version = " + std::to_string(v));
    });
    transaction.commit();
  } catch (const std::exception &e) {
    spdlog::error("Failed to initialize settings store: {}", e.what());
  }
}

SqliteSettingsRepository::~SqliteSettingsRepository() = default;

std::optional<std::string> SqliteSettingsRepository::getGlobalValue(const std::string &key) {
  try {
    SQLite::Statement query(*m_database, "SELECT value FROM global_settings WHERE key = :key");
    query.bind(":key", key);
    if (query.executeStep()) {
      return std::string(query.getColumn(0));
    }
    return std::nullopt;
  } catch (const std::exception &e) {
    spdlog::error("Failed to get global value: {}", e.what());
    return std::nullopt;
  }
}

bool SqliteSettingsRepository::setGlobalValue(const std::string &key, const std::string &value) {
  try {
    SQLite::Statement query(*m_database, "INSERT OR REPLACE INTO global_settings "
                                         "(key, value) VALUES (:key, :value)");
    query.bind(":key", key);
    query.bind(":value", value);
    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to set global value: {}", e.what());
    return false;
  }
}

bool SqliteSettingsRepository::resetGlobalValue(const std::string &key) {
  try {
    SQLite::Statement query(*m_database, "DELETE FROM global_settings WHERE key = :key");
    query.bind(":key", key);
    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to reset global value: {}", e.what());
    return false;
  }
}

std::optional<std::string> SqliteSettingsRepository::getPlatformValue(int platformId, const std::string &key) {
  try {
    SQLite::Statement query(*m_database, "SELECT value FROM platform_settings WHERE "
                                         "platform_id = :platformId AND key = :key");
    query.bind(":platformId", platformId);
    query.bind(":key", key);
    if (query.executeStep()) {
      return std::string(query.getColumn(0));
    }
    return std::nullopt;
  } catch (const std::exception &e) {
    spdlog::error("Failed to get platform value: {}", e.what());
    return std::nullopt;
  }
}

bool SqliteSettingsRepository::setPlatformValue(int platformId, const std::string &key, const std::string &value) {
  try {
    SQLite::Statement query(*m_database, "INSERT OR REPLACE INTO platform_settings "
                                         "(platform_id, key, value) VALUES (:platformId, :key, :value)");
    query.bind(":platformId", platformId);
    query.bind(":key", key);
    query.bind(":value", value);
    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to set platform value: {}", e.what());
    return false;
  }
}

bool SqliteSettingsRepository::resetPlatformValue(int platformId, const std::string &key) {
  try {
    SQLite::Statement query(*m_database, "DELETE FROM platform_settings WHERE platform_id = "
                                         ":platformId AND key = :key");
    query.bind(":platformId", platformId);
    query.bind(":key", key);
    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to reset platform value: {}", e.what());
    return false;
  }
}

std::optional<std::string> SqliteSettingsRepository::getGameValue(const std::string &contentHash,
                                                                  const std::string &key) {
  try {
    SQLite::Statement query(*m_database, "SELECT value FROM game_settings WHERE content_hash = :contentHash AND "
                                         "platform_id = :platformId AND key = :key");
    query.bind(":contentHash", contentHash);
    query.bind(":platformId", -1);
    query.bind(":key", key);
    if (query.executeStep()) {
      return std::string(query.getColumn(0));
    }
    return std::nullopt;
  } catch (const std::exception &e) {
    spdlog::error("Failed to get game value: {}", e.what());
    return std::nullopt;
  }
}

bool SqliteSettingsRepository::setGameValue(const std::string &contentHash, const std::string &key,
                                            const std::string &value) {
  try {
    SQLite::Statement query(*m_database, "INSERT OR REPLACE INTO game_settings "
                                         "(content_hash, platform_id, key, value) VALUES "
                                         "(:contentHash, :platformId, :key, :value)");
    query.bind(":contentHash", contentHash);
    query.bind(":platformId", -1);
    query.bind(":key", key);
    query.bind(":value", value);
    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to set game value: {}", e.what());
    return false;
  }
}

bool SqliteSettingsRepository::resetGameValue(const std::string &contentHash, const std::string &key) {
  try {
    SQLite::Statement query(*m_database, "DELETE FROM game_settings WHERE content_hash = :contentHash AND "
                                         "platform_id = :platformId AND key = :key");
    query.bind(":contentHash", contentHash);
    query.bind(":platformId", -1);
    query.bind(":key", key);
    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to reset game value: {}", e.what());
    return false;
  }
}

void SqliteSettingsRepository::upsertCoreOptions(const std::string &coreName, const std::vector<CoreOption> &options) {
  try {
    // Replace-all: a core's declared option set is authoritative, so options it
    // no longer declares should disappear from the cache
    SQLite::Transaction transaction(*m_database);
    {
      SQLite::Statement del(*m_database, "DELETE FROM core_options WHERE core_name = :core");
      del.bind(":core", coreName);
      del.exec();
    }

    SQLite::Statement insert(*m_database, "INSERT INTO core_options (core_name, key, label, description, "
                                          "default_value, values_json, category_key, category_label, position) "
                                          "VALUES (:core, :key, :label, :desc, :def, :values, :catKey, "
                                          ":catLabel, :pos)");
    int position = 0;
    for (const auto &option : options) {
      nlohmann::json values = nlohmann::json::array();
      for (const auto &v : option.values) {
        values.push_back({{"value", v.value}, {"label", v.label}});
      }
      insert.bind(":core", coreName);
      insert.bind(":key", option.key);
      insert.bind(":label", option.label);
      insert.bind(":desc", option.description);
      insert.bind(":def", option.defaultValue);
      insert.bind(":values", values.dump());
      insert.bind(":catKey", option.category);
      insert.bind(":catLabel", option.categoryLabel);
      insert.bind(":pos", position++);
      insert.exec();
      insert.reset();
    }

    transaction.commit();
  } catch (const std::exception &e) {
    spdlog::error("Failed to upsert core options for {}: {}", coreName, e.what());
  }
}

std::vector<CoreOption> SqliteSettingsRepository::getCoreOptions(const std::string &coreName) {
  std::vector<CoreOption> options;
  try {
    SQLite::Statement query(*m_database, "SELECT key, label, description, default_value, values_json, "
                                         "category_key, category_label FROM core_options WHERE core_name = :core "
                                         "ORDER BY position");
    query.bind(":core", coreName);
    while (query.executeStep()) {
      CoreOption option;
      option.key = query.getColumn(0).getString();
      option.label = query.getColumn(1).getString();
      option.description = query.getColumn(2).getString();
      option.defaultValue = query.getColumn(3).getString();
      option.category = query.getColumn(5).getString();
      option.categoryLabel = query.getColumn(6).getString();

      const auto valuesJson =
          nlohmann::json::parse(query.getColumn(4).getString(), nullptr, /*allow_exceptions=*/false);
      if (valuesJson.is_array()) {
        for (const auto &v : valuesJson) {
          option.values.push_back({v.value("value", std::string{}), v.value("label", std::string{})});
        }
      }
      options.push_back(std::move(option));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to read core options for {}: {}", coreName, e.what());
  }
  return options;
}

} // namespace firelight::settings
