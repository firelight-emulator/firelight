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
  session.saveSlot = query.getColumn("save_slot").getInt();
  session.startedAt = query.getColumn("started_at").getInt64();
  session.endedAt = query.getColumn("ended_at").getInt64();
  session.unpausedDurationMillis = query.getColumn("unpaused_duration_millis").getInt64();
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
           // Durations were stored in seconds while the field was milliseconds, so every
           // session lost its sub-second remainder on the way in and gained fake precision
           // on the way back out
           m_db->exec("CREATE TABLE IF NOT EXISTS play_sessions("
                      "id INTEGER PRIMARY KEY,"
                      "content_hash TEXT NOT NULL,"
                      "save_slot INTEGER NOT NULL,"
                      "started_at INTEGER NOT NULL,"
                      "ended_at INTEGER NOT NULL,"
                      "unpaused_duration_millis INTEGER NOT NULL);");

           m_db->exec("CREATE INDEX IF NOT EXISTS playSessionContentHashIdx ON "
                      "play_sessions(content_hash);");
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

  if (session.startedAt == 0) {
    spdlog::warn("Attempted to create play session with start time of 0");
    return false;
  }

  if (session.endedAt == 0) {
    spdlog::warn("Attempted to create play session with end time of 0");
    return false;
  }

  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement insert(*m_db, "INSERT INTO play_sessions(content_hash, save_slot, "
                                    "started_at, ended_at, unpaused_duration_millis) "
                                    "VALUES(:contentHash, :saveSlot, :startedAt, :endedAt, :duration);");
    insert.bind(":contentHash", session.contentHash);
    insert.bind(":saveSlot", session.saveSlot);
    insert.bind(":startedAt", static_cast<int64_t>(session.startedAt));
    insert.bind(":endedAt", static_cast<int64_t>(session.endedAt));
    insert.bind(":duration", static_cast<int64_t>(session.unpausedDurationMillis));
    insert.exec();

    session.id = static_cast<int>(m_db->getLastInsertRowid());
    return true;
  } catch (const std::exception &e) {
    spdlog::warn("Insert into play_sessions failed: {}", e.what());
    return false;
  }
}

bool SqliteActivityLog::transferSessions(const std::string &fromContentHash, const std::string &toContentHash) {
  if (fromContentHash.empty() || toContentHash.empty() || fromContentHash == toContentHash) {
    return false;
  }

  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement update(*m_db, "UPDATE play_sessions SET content_hash = :to WHERE content_hash = :from;");
    update.bind(":to", toContentHash);
    update.bind(":from", fromContentHash);
    return update.exec() > 0;
  } catch (const std::exception &e) {
    spdlog::warn("Failed to transfer play sessions: {}", e.what());
    return false;
  }
}

std::optional<PlaySession> SqliteActivityLog::getLatestPlaySession(std::string contentHash) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM play_sessions WHERE content_hash = :contentHash "
                                   "ORDER BY started_at DESC LIMIT 1;");
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
                                   "ORDER BY started_at DESC;");
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
    SQLite::Statement query(*m_db, "SELECT * FROM play_sessions ORDER BY started_at DESC;");
    while (query.executeStep()) {
      sessions.push_back(readSession(query));
    }
  } catch (const std::exception &e) {
    spdlog::warn("Query failed: {}", e.what());
  }

  return sessions;
}
} // namespace firelight::activity
