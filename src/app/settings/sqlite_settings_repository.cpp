#include "sqlite_settings_repository.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <spdlog/spdlog.h>

constexpr auto EMULATION_SETTINGS_DATABASE_PREFIX = "emulation_settings_";

namespace firelight {
namespace achievements {} // namespace achievements
settings::SqliteSettingsRepository::SqliteSettingsRepository(
    const QString &databaseFile)
    : m_databaseFile(databaseFile) {
  std::string setupQueryString = R"(
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
settings::SqliteSettingsRepository::~SqliteSettingsRepository() = default;

std::optional<std::string>
settings::SqliteSettingsRepository::getPlatformValue(int platformId,
                                                     const std::string &key) {
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
settings::SqliteSettingsRepository::getGameValue(const std::string &contentHash,
                                                 const int platformId,
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

std::string settings::SqliteSettingsRepository::getEffectiveValue(
    const std::string &contentHash, int platformId, const std::string &key,
    const std::string &defaultValue) {
  if (auto v = getGameValue(contentHash, platformId, key)) {
    return *v;
  }

  if (auto v = getPlatformValue(platformId, key)) {
    return *v;
  }

  return defaultValue;
}

void settings::SqliteSettingsRepository::setPlatformValue(
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

  emit platformValueChanged(platformId, QString::fromStdString(key),
                            QString::fromStdString(value));
}

void settings::SqliteSettingsRepository::setGameValue(
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

  emit gameValueChanged(QString::fromStdString(contentHash), platformId,
                        QString::fromStdString(key),
                        QString::fromStdString(value));
}

void settings::SqliteSettingsRepository::resetPlatformValue(
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

  emit platformValueReset(platformId, QString::fromStdString(key));
}

void settings::SqliteSettingsRepository::resetGameValue(
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

  emit gameValueReset(QString::fromStdString(contentHash), platformId,
                      QString::fromStdString(key));
}

QSqlDatabase settings::SqliteSettingsRepository::getDatabase() const {
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