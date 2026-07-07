#include <firelight/media/sqlite_game_capture_repository.hpp>

#include <firelight/migrations/migration_runner.hpp>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>

#include <spdlog/spdlog.h>

#include <chrono>
#include <utility>

namespace firelight::media {
namespace {
int64_t nowMs() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
      .count();
}

GameCapture readCapture(const SQLite::Statement &query) {
  GameCapture c;
  c.id = query.getColumn("id").getInt();
  c.contentHash = query.getColumn("content_hash").getString();
  c.type = static_cast<CaptureType>(query.getColumn("capture_type").getInt());
  c.filePath = query.getColumn("file_path").getString();
  c.thumbnailPath = query.getColumn("thumbnail_path").getString();
  c.timestamp = query.getColumn("timestamp").getInt64();
  c.favorite = query.getColumn("favorite").getInt() != 0;
  c.createdAt = query.getColumn("created_at").getInt64();
  return c;
}
} // namespace

SqliteGameCaptureRepository::SqliteGameCaptureRepository(
    std::string databaseFile)
    : m_databaseFile(std::move(databaseFile)) {
  m_db = std::make_unique<SQLite::Database>(
      m_databaseFile, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
  try {
    // Forward-only schema migrations (see migration_runner). A future change
    // adds the next-numbered migration.
    const std::vector<migrations::Migration> schema = {
        {1,
         [this] {
           m_db->exec("CREATE TABLE IF NOT EXISTS captures("
                      "id INTEGER PRIMARY KEY,"
                      "content_hash TEXT NOT NULL,"
                      "capture_type INTEGER NOT NULL,"
                      "file_path TEXT NOT NULL,"
                      "thumbnail_path TEXT NOT NULL DEFAULT '',"
                      "timestamp INTEGER NOT NULL,"
                      "favorite INTEGER NOT NULL DEFAULT 0,"
                      "created_at INTEGER NOT NULL);");
           m_db->exec("CREATE INDEX IF NOT EXISTS captureContentHashIdx ON "
                      "captures(content_hash);");
           m_db->exec("CREATE UNIQUE INDEX IF NOT EXISTS captureFilePathIdx ON "
                      "captures(file_path);");
         }},
    };

    SQLite::Transaction transaction(*m_db);
    const int currentVersion = m_db->execAndGet("PRAGMA user_version").getInt();
    migrations::applyMigrations(currentVersion, schema, [this](const int v) {
      m_db->exec("PRAGMA user_version = " + std::to_string(v));
    });
    transaction.commit();
  } catch (const std::exception &e) {
    spdlog::error("Failed to initialize captures store: {}", e.what());
  }
}

SqliteGameCaptureRepository::~SqliteGameCaptureRepository() = default;

bool SqliteGameCaptureRepository::add(GameCapture &capture) {
  std::lock_guard lock(m_mutex);
  try {
    {
      SQLite::Statement insert(
          *m_db, "INSERT OR IGNORE INTO captures(content_hash, capture_type, "
                 "file_path, thumbnail_path, timestamp, favorite, created_at) "
                 "VALUES(:contentHash, :captureType, :filePath, :thumbnailPath, "
                 ":timestamp, :favorite, :createdAt)");
      insert.bind(":contentHash", capture.contentHash);
      insert.bind(":captureType", static_cast<int>(capture.type));
      insert.bind(":filePath", capture.filePath);
      insert.bind(":thumbnailPath", capture.thumbnailPath);
      insert.bind(":timestamp", static_cast<int64_t>(
                                    capture.timestamp ? capture.timestamp
                                                      : nowMs()));
      insert.bind(":favorite", capture.favorite ? 1 : 0);
      insert.bind(":createdAt", static_cast<int64_t>(
                                    capture.createdAt ? capture.createdAt
                                                      : nowMs()));
      insert.exec();
    }
    {
      SQLite::Statement select(
          *m_db, "SELECT id FROM captures WHERE file_path = :filePath");
      select.bind(":filePath", capture.filePath);
      if (select.executeStep()) {
        capture.id = select.getColumn(0).getInt();
      }
    }
    return capture.id > 0;
  } catch (const std::exception &e) {
    spdlog::error("Failed to add capture {}: {}", capture.filePath, e.what());
    return false;
  }
}

std::vector<GameCapture> SqliteGameCaptureRepository::listAll() {
  std::lock_guard lock(m_mutex);
  std::vector<GameCapture> out;
  try {
    SQLite::Statement query(
        *m_db, "SELECT * FROM captures ORDER BY timestamp DESC, id DESC");
    while (query.executeStep()) {
      out.push_back(readCapture(query));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to list captures: {}", e.what());
  }
  return out;
}

std::vector<GameCapture>
SqliteGameCaptureRepository::listForGame(const std::string &contentHash) {
  std::lock_guard lock(m_mutex);
  std::vector<GameCapture> out;
  try {
    SQLite::Statement query(*m_db,
                            "SELECT * FROM captures WHERE content_hash = "
                            ":contentHash ORDER BY timestamp DESC, id DESC");
    query.bind(":contentHash", contentHash);
    while (query.executeStep()) {
      out.push_back(readCapture(query));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to list captures for {}: {}", contentHash, e.what());
  }
  return out;
}

std::optional<GameCapture> SqliteGameCaptureRepository::getById(int id) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM captures WHERE id = :id");
    query.bind(":id", id);
    if (query.executeStep()) {
      return readCapture(query);
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get capture {}: {}", id, e.what());
  }
  return std::nullopt;
}

bool SqliteGameCaptureRepository::setFavorite(int id, bool favorite) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(
        *m_db, "UPDATE captures SET favorite = :fav WHERE id = :id");
    query.bind(":fav", favorite ? 1 : 0);
    query.bind(":id", id);
    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to set favorite for capture {}: {}", id, e.what());
    return false;
  }
}

bool SqliteGameCaptureRepository::remove(int id) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "DELETE FROM captures WHERE id = :id");
    query.bind(":id", id);
    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to remove capture {}: {}", id, e.what());
    return false;
  }
}

bool SqliteGameCaptureRepository::existsForPath(const std::string &filePath) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(
        *m_db, "SELECT 1 FROM captures WHERE file_path = :filePath LIMIT 1");
    query.bind(":filePath", filePath);
    return query.executeStep();
  } catch (const std::exception &e) {
    spdlog::error("Failed to check capture path {}: {}", filePath, e.what());
    return false;
  }
}

} // namespace firelight::media
