#include "sqlite_emulation_settings_manager.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <spdlog/spdlog.h>

constexpr auto EMULATION_SETTINGS_DATABASE_PREFIX = "emulation_settings_";

namespace firelight {
namespace achievements {} // namespace achievements
settings::SqliteEmulationSettingsManager::SqliteEmulationSettingsManager(
    const QString &databaseFile)
    : m_databaseFile(databaseFile) {
  std::string setupQueryString = R"(
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
        )";

  QStringList queries =
      QString::fromStdString(setupQueryString).split(';', Qt::SkipEmptyParts);

  for (const QString &query : queries) {
    if (query.isEmpty() || query.trimmed().isEmpty()) {
      continue;
    }
    if (QSqlQuery setupQuery(getDatabase()); !setupQuery.exec(query)) {
      spdlog::error("Failed to execute query: {}",
                    setupQuery.lastError().text().toStdString());
    }
  }
}
settings::SqliteEmulationSettingsManager::~SqliteEmulationSettingsManager() {}

std::optional<std::string>
settings::SqliteEmulationSettingsManager::getGlobalValue(
    const std::string &key) {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT value FROM global_settings WHERE key = :key");

  query.bindValue(":key", QString::fromStdString(key));

  if (!query.exec()) {
    spdlog::error("Failed to get global value: {}",
                  query.lastError().text().toStdString());
    return std::nullopt;
  }

  if (!query.next()) {
    return std::nullopt;
  }

  return query.value("value").toString().toStdString();
}

std::optional<std::string>
settings::SqliteEmulationSettingsManager::getPlatformValue(
    int platformId, const std::string &key) {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT value FROM platform_settings WHERE platform_id = "
                ":platformId AND key = :key");
  query.bindValue(":platformId", platformId);
  query.bindValue(":key", QString::fromStdString(key));

  if (!query.exec()) {
    spdlog::error("Failed to get platform value: {}",
                  query.lastError().text().toStdString());
    return std::nullopt;
  }

  if (!query.next()) {
    return std::nullopt;
  }

  return query.value("value").toString().toStdString();
}

std::optional<std::string>
settings::SqliteEmulationSettingsManager::getGameValue(
    const std::string &contentHash, const int platformId,
    const std::string &key) {
  QSqlQuery query(getDatabase());
  query.prepare(
      "SELECT value FROM game_settings WHERE content_hash = :contentHash "
      "AND platform_id = :platformId AND key = :key");

  query.bindValue(":contentHash", QString::fromStdString(contentHash));
  query.bindValue(":platformId", platformId);
  query.bindValue(":key", QString::fromStdString(key));

  if (!query.exec()) {
    spdlog::error("Failed to get game value: {}",
                  query.lastError().text().toStdString());
    return std::nullopt;
  }

  if (!query.next()) {
    return std::nullopt;
  }

  return query.value("value").toString().toStdString();
}

void settings::SqliteEmulationSettingsManager::setGlobalValue(
    const std::string &key, const std::string &value) {
  QSqlQuery query(getDatabase());
  query.prepare("INSERT OR REPLACE INTO global_settings (key, value) "
                "VALUES (:key, :value)");
  query.bindValue(":key", QString::fromStdString(key));
  query.bindValue(":value", QString::fromStdString(value));

  if (!query.exec()) {
    spdlog::error("Failed to set global value: {}",
                  query.lastError().text().toStdString());
  }

  if (m_globalValueChangedCallback) {
    m_globalValueChangedCallback(key, value);
  }
}

void settings::SqliteEmulationSettingsManager::setPlatformValue(
    const int platformId, const std::string &key, const std::string &value) {
  QSqlQuery query(getDatabase());
  query.prepare("INSERT OR REPLACE INTO platform_settings (platform_id, key, "
                "value) VALUES (:platformId, :key, :value)");
  query.bindValue(":platformId", platformId);
  query.bindValue(":key", QString::fromStdString(key));
  query.bindValue(":value", QString::fromStdString(value));

  if (!query.exec()) {
    spdlog::error("Failed to set platform value: {}",
                  query.lastError().text().toStdString());
  }

  if (m_platformValueChangedCallback) {
    m_platformValueChangedCallback(platformId, key, value);
  }
}

void settings::SqliteEmulationSettingsManager::setGameValue(
    const std::string &contentHash, int platformId, const std::string &key,
    const std::string &value) {
  QSqlQuery query(getDatabase());
  query.prepare(
      "INSERT OR REPLACE INTO game_settings (content_hash, platform_id, key, "
      "value) VALUES (:contentHash, :platformId, :key, :value)");

  query.bindValue(":contentHash", QString::fromStdString(contentHash));
  query.bindValue(":platformId", platformId);
  query.bindValue(":key", QString::fromStdString(key));
  query.bindValue(":value", QString::fromStdString(value));

  if (!query.exec()) {
    spdlog::error("Failed to set game value: {}",
                  query.lastError().text().toStdString());
  }

  if (m_gameValueChangedCallback) {
    m_gameValueChangedCallback(contentHash, platformId, key, value);
  }
}

void settings::SqliteEmulationSettingsManager::resetGlobalValue(
    const std::string &key) {
  QSqlQuery query(getDatabase());
  query.prepare("DELETE FROM global_settings WHERE key = :key");
  query.bindValue(":key", QString::fromStdString(key));

  if (!query.exec()) {
    spdlog::error("Failed to reset global value: {}",
                  query.lastError().text().toStdString());
  }

  // TODO: Call the callback for global value reset
}

void settings::SqliteEmulationSettingsManager::resetPlatformValue(
    int platformId, const std::string &key) {
  QSqlQuery query(getDatabase());
  query.prepare("DELETE FROM platform_settings WHERE platform_id = "
                ":platformId AND key = :key");

  query.bindValue(":platformId", platformId);
  query.bindValue(":key", QString::fromStdString(key));

  if (!query.exec()) {
    spdlog::error("Failed to reset platform value: {}",
                  query.lastError().text().toStdString());
  }

  // TODO: Call the callback for platform value reset
}

void settings::SqliteEmulationSettingsManager::resetGameValue(
    const std::string &contentHash, int platformId, const std::string &key) {
  QSqlQuery query(getDatabase());
  query.prepare("DELETE FROM game_settings WHERE content_hash = :contentHash "
                "AND platform_id = :platformId AND key = :key");

  query.bindValue(":contentHash", QString::fromStdString(contentHash));
  query.bindValue(":platformId", platformId);
  query.bindValue(":key", QString::fromStdString(key));

  if (!query.exec()) {
    spdlog::error("Failed to reset game value: {}",
                  query.lastError().text().toStdString());
  }

  // TODO: Call the callback for game value reset
}

void settings::SqliteEmulationSettingsManager::setOnGlobalValueChanged(
    const std::function<void(const std::string &key, const std::string &value)>
        callback) {
  m_globalValueChangedCallback = callback;
}

void settings::SqliteEmulationSettingsManager::setOnPlatformValueChanged(
    const std::function<void(int platformId, const std::string &key,
                             const std::string &value)>
        callback) {
  m_platformValueChangedCallback = callback;
}

void settings::SqliteEmulationSettingsManager::setOnGameValueChanged(
    const std::function<void(const std::string &contentHash, int platformId,
                             const std::string &key, const std::string &value)>
        callback) {
  m_gameValueChangedCallback = callback;
}

QSqlDatabase settings::SqliteEmulationSettingsManager::getDatabase() const {
  const auto name =
      EMULATION_SETTINGS_DATABASE_PREFIX +
      QString::number(reinterpret_cast<quint64>(QThread::currentThread()), 16);
  if (QSqlDatabase::contains(name)) {
    return QSqlDatabase::database(name);
  }

  spdlog::debug("Database connection with name {} does not exist; creating",
                name.toStdString());
  auto db = QSqlDatabase::addDatabase("QSQLITE", name);
  db.setDatabaseName(m_databaseFile);
  db.open();
  return db;
}
} // namespace firelight