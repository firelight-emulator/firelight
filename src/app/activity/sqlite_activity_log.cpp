#include "sqlite_activity_log.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::activity {
constexpr auto DATABASE_PREFIX = "activity_";

SqliteActivityLog::SqliteActivityLog(QString dbPath)
    : databasePath(std::move(dbPath)) {
  QSqlQuery createPlaySessions(getDatabase());
  createPlaySessions.prepare("CREATE TABLE IF NOT EXISTS play_sessions("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "content_hash TEXT NOT NULL,"
                             "savefile_slot_number INTEGER NOT NULL,"
                             "start_time INTEGER NOT NULL,"
                             "end_time INTEGER NOT NULL,"
                             "unpaused_duration_seconds INTEGER NOT NULL);");

  if (!createPlaySessions.exec()) {
    spdlog::error("Table creation failed: {}",
                  createPlaySessions.lastError().text().toStdString());
  }
}

bool SqliteActivityLog::createPlaySession(PlaySession &session) {
  if (session.contentHash.empty()) {
    spdlog::warn("Attempted to create play session with empty content hash");
    return false;
  }

  if (session.startTime == 0) {
    spdlog::warn("Attempted to create play session with start time of 0");
    return false;
  }

  if (session.endTime == 0) {
    spdlog::warn("Attempted to create play session with end time of 0");
    return false;
  }

  const QString queryString = "INSERT INTO play_sessions ("
                              "content_hash, "
                              "savefile_slot_number, "
                              "start_time, "
                              "end_time, "
                              "unpaused_duration_seconds) "
                              "VALUES (:contentHash, :slotNumber, :startTime,"
                              ":endTime, :unpausedDurationSeconds);";
  auto query = QSqlQuery(getDatabase());
  query.prepare(queryString);

  query.bindValue(":contentHash", QString::fromStdString(session.contentHash));
  query.bindValue(":slotNumber", session.slotNumber);
  query.bindValue(":startTime", session.startTime);
  query.bindValue(":endTime", session.endTime);
  query.bindValue(":unpausedDurationSeconds",
                  session.unpausedDurationMillis / 1000);

  if (!query.exec()) {
    spdlog::warn("Insert into play_sessions failed: {}",
                 query.lastError().text().toStdString());
    return false;
  }

  session.id = query.lastInsertId().toInt();

  return true;
}

std::optional<PlaySession>
SqliteActivityLog::getLatestPlaySession(std::string contentHash) {
  return std::nullopt;
}

std::vector<PlaySession>
SqliteActivityLog::getPlaySessions(const std::string contentHash) {
  // Query DB for all play sessions with the given content hash
  const QString queryString =
      "SELECT * FROM play_sessions WHERE content_hash = :contentHash;";

  auto query = QSqlQuery(getDatabase());
  query.prepare(queryString);

  query.bindValue(":contentHash", QString::fromStdString(contentHash));

  if (!query.exec()) {
    spdlog::warn("Query failed: {}", query.lastError().text().toStdString());
    return {};
  }

  std::vector<PlaySession> playSessions;
  while (query.next()) {
    // Create a PlaySession object from the query result
    PlaySession session;
    session.contentHash = contentHash;
    session.slotNumber = query.value("savefile_slot_number").toInt();
    session.startTime = query.value("start_time").toULongLong();
    session.endTime = query.value("end_time").toULongLong();
    session.unpausedDurationMillis =
        query.value("unpaused_duration_seconds").toULongLong() * 1000;
    session.slotNumber = query.value("savefile_slot_number").toInt();

    playSessions.push_back(session);
  }

  return playSessions;
}

QSqlDatabase SqliteActivityLog::getDatabase() const {
  const auto name =
      DATABASE_PREFIX +
      QString::number(reinterpret_cast<quint64>(QThread::currentThread()), 16);
  if (QSqlDatabase::contains(name)) {
    return QSqlDatabase::database(name);
  }

  spdlog::debug("Database connection with name {} does not exist; creating",
                name.toStdString());
  auto db = QSqlDatabase::addDatabase("QSQLITE", name);
  db.setDatabaseName(databasePath);
  db.open();
  return db;
}
} // namespace firelight::activity
// firelight