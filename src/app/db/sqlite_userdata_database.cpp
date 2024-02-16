//
// Created by alexs on 2/16/2024.
//

#include "sqlite_userdata_database.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <spdlog/spdlog.h>

static const char *create_query =
    "CREATE TABLE IF NOT EXISTS play_sessions("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "rom_md5 TEXT NOT NULL,"
    "start_time INTEGER NOT NULL,"
    "end_time INTEGER NOT NULL,"
    "unpaused_duration_seconds INTEGER NOT NULL);";

SqliteUserdataDatabase::SqliteUserdataDatabase(
    const std::filesystem::path &dbFile)
    : m_database_path(dbFile) {
  m_database = QSqlDatabase::addDatabase("QSQLITE", "userdata");
  m_database.setDatabaseName(QString::fromStdString(dbFile.string()));
  if (!m_database.open()) {
    printf("couldn't connect for some reason\n");
  }

  QSqlQuery create(m_database);
  create.prepare(create_query);
  if (!create.exec()) {
    spdlog::error("Table creation failed: {}",
                  create.lastError().text().toStdString());
  }
}

void SqliteUserdataDatabase::savePlaySession(
    const std::string romMd5, const long long startTime,
    const long long endTime, const long long unpausedDurationSeconds) {
  const QString queryString =
      "INSERT INTO play_sessions ("
      "rom_md5, "
      "start_time, "
      "end_time, "
      "unpaused_duration_seconds) "
      "VALUES (:rom_md5, :start_time, :end_time, :unpaused_duration_seconds);";

  auto db = QSqlDatabase::database("userdata");
  if (!db.isValid()) {
    // TODO: Give each thread its own connection name?
    db = QSqlDatabase::addDatabase("QSQLITE", "userdata");
    db.setDatabaseName(QString::fromStdString(m_database_path.string()));
    db.open();
  }

  auto query = QSqlQuery(db);
  query.prepare(queryString);

  query.bindValue(":rom_md5", QString::fromStdString(romMd5));
  query.bindValue(":start_time", startTime);
  query.bindValue(":end_time", endTime);
  query.bindValue(":unpaused_duration_seconds", unpausedDurationSeconds);

  if (!query.exec()) {
    spdlog::warn("Insert into play_sessions failed: {}",
                 query.lastError().text().toStdString());
  }
}