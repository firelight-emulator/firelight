#include "firelight/activity/sqlite_activity_log.hpp"

#include <firelight/activity/play_session.hpp>
#include <firelight/migrations/migration_runner.hpp>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::activity {
namespace {
PlaySession readSession(SQLite::Statement &query) {
  PlaySession session;
  session.id = query.getColumn("id").getInt();
  session.contentHash = query.getColumn("content_hash").getString();
  session.slotNumber = query.getColumn("savefile_slot_number").getInt();
  session.startTime = query.getColumn("start_time").getInt64();
  session.endTime = query.getColumn("end_time").getInt64();
  session.unpausedDurationMillis = query.getColumn("unpaused_duration_seconds").getInt64() * 1000;
  return session;
}
} // namespace

SqliteActivityLog::SqliteActivityLog(std::string databaseFile) : m_databaseFile(std::move(databaseFile)) {
  m_db = std::make_unique<SQLite::Database>(m_databaseFile, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
  try {
    // Forward-only schema migrations (see migration_runner). A future change
    // adds the next-numbered migration
    const std::vector<migrations::Migration> schema = {
        {1,
         [this] {
           m_db->exec("CREATE TABLE IF NOT EXISTS play_sessions("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "content_hash TEXT NOT NULL,"
                      "savefile_slot_number INTEGER NOT NULL,"
                      "start_time INTEGER NOT NULL,"
                      "end_time INTEGER NOT NULL,"
                      "unpaused_duration_seconds INTEGER NOT NULL);");
         }},
    };

    SQLite::Transaction transaction(*m_db);
    const int currentVersion = m_db->execAndGet("PRAGMA user_version").getInt();
    migrations::applyMigrations(currentVersion, schema,
                                [this](const int v) { m_db->exec("PRAGMA user_version = " + std::to_string(v)); });
    transaction.commit();
  } catch (const std::exception &e) {
    spdlog::error("Failed to initialize activity store: {}", e.what());
  }
}

SqliteActivityLog::~SqliteActivityLog() = default;

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

  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement insert(*m_db, "INSERT INTO play_sessions(content_hash, savefile_slot_number, "
                                    "start_time, end_time, unpaused_duration_seconds) "
                                    "VALUES(:contentHash, :slotNumber, :startTime, :endTime, :duration);");
    insert.bind(":contentHash", session.contentHash);
    insert.bind(":slotNumber", session.slotNumber);
    insert.bind(":startTime", static_cast<int64_t>(session.startTime));
    insert.bind(":endTime", static_cast<int64_t>(session.endTime));
    insert.bind(":duration", static_cast<int64_t>(session.unpausedDurationMillis / 1000));
    insert.exec();

    session.id = static_cast<int>(m_db->getLastInsertRowid());
    return true;
  } catch (const std::exception &e) {
    spdlog::warn("Insert into play_sessions failed: {}", e.what());
    return false;
  }
}

std::optional<PlaySession> SqliteActivityLog::getLatestPlaySession(std::string contentHash) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM play_sessions WHERE content_hash = :contentHash "
                                   "ORDER BY start_time DESC LIMIT 1;");
    query.bind(":contentHash", contentHash);

    if (query.executeStep()) {
      return readSession(query);
    }
  } catch (const std::exception &e) {
    spdlog::warn("Query failed: {}", e.what());
  }

  return std::nullopt;
}

std::vector<PlaySession> SqliteActivityLog::getPlaySessions(const std::string contentHash) {
  std::lock_guard lock(m_mutex);
  std::vector<PlaySession> sessions;
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM play_sessions WHERE content_hash = :contentHash "
                                   "ORDER BY start_time DESC;");
    query.bind(":contentHash", contentHash);

    while (query.executeStep()) {
      sessions.push_back(readSession(query));
    }
  } catch (const std::exception &e) {
    spdlog::warn("Query failed: {}", e.what());
  }

  return sessions;
}

std::vector<PlaySession> SqliteActivityLog::getPlaySessions() {
  std::lock_guard lock(m_mutex);
  std::vector<PlaySession> sessions;
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM play_sessions ORDER BY start_time DESC;");
    while (query.executeStep()) {
      sessions.push_back(readSession(query));
    }
  } catch (const std::exception &e) {
    spdlog::warn("Query failed: {}", e.what());
  }

  return sessions;
}
} // namespace firelight::activity
