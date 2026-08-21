#include <firelight/migrations/migration_runner.hpp>
#include <firelight/saves/sqlite_save_database.hpp>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>
#include <chrono>
#include <spdlog/spdlog.h>

namespace firelight::saves {
namespace {
int64_t nowMs() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

SavefileMetadata readSavefile(SQLite::Statement &q) {
  SavefileMetadata m;
  m.id = q.getColumn("id").getInt();
  m.contentHash = q.getColumn("content_hash").getString();
  m.saveSlot = static_cast<unsigned int>(q.getColumn("save_slot").getInt());
  m.savefileMd5 = q.getColumn("savefile_md5").getString();
  m.lastModifiedAt = q.getColumn("last_modified_at").getInt64();
  m.createdAt = q.getColumn("created_at").getInt64();
  return m;
}

SuspendPointMetadata readSuspend(SQLite::Statement &q) {
  SuspendPointMetadata m;
  m.id = q.getColumn("id").getInt();
  m.contentHash = q.getColumn("content_hash").getString();
  m.saveSlot = q.getColumn("save_slot").getInt();
  m.pointIndex = static_cast<unsigned int>(q.getColumn("point_index").getInt());
  m.locked = q.getColumn("locked").getInt() != 0;
  m.lastModifiedAt = q.getColumn("last_modified_at").getInt64();
  m.createdAt = q.getColumn("created_at").getInt64();
  return m;
}
} // namespace

SqliteSaveDatabase::SqliteSaveDatabase(const std::string &dbFile) : m_databaseFile(dbFile) {
  m_db = std::make_unique<SQLite::Database>(m_databaseFile, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
  m_db->exec("PRAGMA journal_mode=WAL;");
  m_db->exec("PRAGMA synchronous=NORMAL;");

  // Forward-only schema migrations (see migration_runner). A future change adds
  // the next-numbered migration
  // Fresh schema rather than a chain: nothing has shipped, so the tables are simply
  // declared correctly. Timestamps are epoch milliseconds throughout
  const std::vector<migrations::Migration> schema = {
      {1,
       [this] {
         m_db->exec("CREATE TABLE IF NOT EXISTS savefile_metadata("
                    "id INTEGER PRIMARY KEY,"
                    "content_hash TEXT NOT NULL,"
                    "save_slot INTEGER NOT NULL,"
                    "savefile_md5 TEXT NOT NULL,"
                    "last_modified_at INTEGER NOT NULL,"
                    "created_at INTEGER NOT NULL,"
                    "UNIQUE(content_hash, save_slot));");

         // A suspend point is addressed by all three, and the key says so. Leaving the
         // save slot out is what made two playthroughs collide on the same index
         m_db->exec("CREATE TABLE IF NOT EXISTS suspend_point_metadata("
                    "id INTEGER PRIMARY KEY,"
                    "content_hash TEXT NOT NULL,"
                    "save_slot INTEGER NOT NULL,"
                    "point_index INTEGER NOT NULL,"
                    "locked INTEGER NOT NULL DEFAULT 0,"
                    "last_modified_at INTEGER NOT NULL,"
                    "created_at INTEGER NOT NULL,"
                    "UNIQUE(content_hash, save_slot, point_index));");
       }},
  };

  try {
    SQLite::Transaction transaction(*m_db);
    const int currentVersion = m_db->execAndGet("PRAGMA user_version").getInt();
    migrations::applyMigrations(currentVersion, schema,
                                [this](const int v) { m_db->exec("PRAGMA user_version = " + std::to_string(v)); });
    transaction.commit();
  } catch (const std::exception &e) {
    spdlog::error("Failed to initialize save database: {}", e.what());
  }
}

SqliteSaveDatabase::~SqliteSaveDatabase() = default;

bool SqliteSaveDatabase::createSavefileMetadata(SavefileMetadata &metadata) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement q(*m_db, "INSERT INTO savefile_metadata (content_hash, save_slot, "
                               "savefile_md5, last_modified_at, created_at) VALUES "
                               "(:contentHash, :saveSlot, :savefileMd5, :lastModifiedAt, "
                               ":createdAt);");
    q.bind(":contentHash", metadata.contentHash);
    q.bind(":saveSlot", static_cast<int>(metadata.saveSlot));
    q.bind(":savefileMd5", metadata.savefileMd5);
    q.bind(":lastModifiedAt", metadata.lastModifiedAt);
    q.bind(":createdAt", metadata.createdAt != 0 ? metadata.createdAt : nowMs());
    q.exec();
    metadata.id = static_cast<int>(m_db->getLastInsertRowid());
    return true;
  } catch (const std::exception &e) {
    spdlog::error("createSavefileMetadata failed: {}", e.what());
    return false;
  }
}

std::optional<SavefileMetadata> SqliteSaveDatabase::getSavefileMetadata(std::string contentHash, int saveSlot) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement q(*m_db, "SELECT * FROM savefile_metadata WHERE content_hash = "
                               ":contentHash AND save_slot = :saveSlot LIMIT 1;");
    q.bind(":contentHash", contentHash);
    q.bind(":saveSlot", saveSlot);
    if (q.executeStep()) {
      return readSavefile(q);
    }
  } catch (const std::exception &e) {
    spdlog::error("getSavefileMetadata failed: {}", e.what());
  }
  return std::nullopt;
}

bool SqliteSaveDatabase::updateSavefileMetadata(SavefileMetadata metadata) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement q(*m_db, "UPDATE savefile_metadata SET savefile_md5 = :savefileMd5, "
                               "last_modified_at = :lastModifiedAt WHERE id = :id;");
    q.bind(":savefileMd5", metadata.savefileMd5);
    q.bind(":lastModifiedAt", metadata.lastModifiedAt);
    q.bind(":id", metadata.id);
    return q.exec() >= 1;
  } catch (const std::exception &e) {
    spdlog::error("updateSavefileMetadata failed: {}", e.what());
    return false;
  }
}

std::vector<SavefileMetadata> SqliteSaveDatabase::getSavefileMetadataForContent(std::string contentHash) {
  std::lock_guard lock(m_mutex);
  std::vector<SavefileMetadata> result;
  try {
    SQLite::Statement q(*m_db, "SELECT * FROM savefile_metadata WHERE content_hash = :contentHash;");
    q.bind(":contentHash", contentHash);
    while (q.executeStep()) {
      result.emplace_back(readSavefile(q));
    }
  } catch (const std::exception &e) {
    spdlog::error("getSavefileMetadataForContent failed: {}", e.what());
  }
  return result;
}

bool SqliteSaveDatabase::createSuspendPointMetadata(SuspendPointMetadata &metadata) {
  std::lock_guard lock(m_mutex);
  try {
    const int64_t now = nowMs();
    SQLite::Statement q(*m_db, "INSERT INTO suspend_point_metadata (content_hash, "
                               "save_slot, point_index, locked, last_modified_at, "
                               "created_at) VALUES (:contentHash, :saveSlot, :pointIndex, "
                               ":locked, :lastModifiedAt, :createdAt);");
    q.bind(":contentHash", metadata.contentHash);
    q.bind(":saveSlot", metadata.saveSlot);
    q.bind(":pointIndex", static_cast<int>(metadata.pointIndex));
    q.bind(":locked", metadata.locked ? 1 : 0);
    q.bind(":lastModifiedAt", metadata.lastModifiedAt != 0 ? metadata.lastModifiedAt : now);
    q.bind(":createdAt", metadata.createdAt != 0 ? metadata.createdAt : now);
    q.exec();
    metadata.id = static_cast<int>(m_db->getLastInsertRowid());
    return true;
  } catch (const std::exception &e) {
    spdlog::error("createSuspendPointMetadata failed: {}", e.what());
    return false;
  }
}

std::optional<SuspendPointMetadata> SqliteSaveDatabase::getSuspendPointMetadata(std::string contentHash, int saveSlot,
                                                                                int pointIndex) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement q(*m_db, "SELECT * FROM suspend_point_metadata WHERE content_hash = "
                               ":contentHash AND save_slot = :saveSlot AND "
                               "point_index = :pointIndex LIMIT 1;");
    q.bind(":contentHash", contentHash);
    q.bind(":saveSlot", saveSlot);
    q.bind(":pointIndex", pointIndex);
    if (q.executeStep()) {
      return readSuspend(q);
    }
  } catch (const std::exception &e) {
    spdlog::error("getSuspendPointMetadata failed: {}", e.what());
  }
  return std::nullopt;
}

bool SqliteSaveDatabase::updateSuspendPointMetadata(const SuspendPointMetadata &metadata) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement q(*m_db, "UPDATE suspend_point_metadata SET locked = :locked, "
                               "last_modified_at = :lastModifiedAt WHERE id = :id;");
    q.bind(":locked", metadata.locked ? 1 : 0);
    q.bind(":lastModifiedAt", metadata.lastModifiedAt);
    q.bind(":id", metadata.id);
    return q.exec() >= 1;
  } catch (const std::exception &e) {
    spdlog::error("updateSuspendPointMetadata failed: {}", e.what());
    return false;
  }
}

std::vector<SuspendPointMetadata> SqliteSaveDatabase::getSuspendPointMetadataForContent(std::string contentHash,
                                                                                        int saveSlot) {
  std::lock_guard lock(m_mutex);
  std::vector<SuspendPointMetadata> result;
  try {
    SQLite::Statement q(*m_db, "SELECT * FROM suspend_point_metadata WHERE content_hash = "
                               ":contentHash AND save_slot = :saveSlot;");
    q.bind(":contentHash", contentHash);
    q.bind(":saveSlot", saveSlot);
    while (q.executeStep()) {
      result.emplace_back(readSuspend(q));
    }
  } catch (const std::exception &e) {
    spdlog::error("getSuspendPointMetadataForContent failed: {}", e.what());
  }
  return result;
}

bool SqliteSaveDatabase::transferContent(const std::string &fromContentHash, const std::string &toContentHash) {
  std::lock_guard lock(m_mutex);
  try {
    // OR IGNORE leaves a row alone when the destination already holds that slot, matching what
    // the file move does
    SQLite::Statement savefiles(*m_db, "UPDATE OR IGNORE savefile_metadata SET content_hash = :to "
                                       "WHERE content_hash = :from;");
    savefiles.bind(":to", toContentHash);
    savefiles.bind(":from", fromContentHash);
    savefiles.exec();

    SQLite::Statement points(*m_db, "UPDATE OR IGNORE suspend_point_metadata SET content_hash = :to "
                                    "WHERE content_hash = :from;");
    points.bind(":to", toContentHash);
    points.bind(":from", fromContentHash);
    points.exec();
  } catch (const std::exception &e) {
    spdlog::error("transferContent failed: {}", e.what());
    return false;
  }

  return true;
}

bool SqliteSaveDatabase::deleteSuspendPointMetadata(int id) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement q(*m_db, "DELETE FROM suspend_point_metadata WHERE id = :id;");
    q.bind(":id", id);
    return q.exec() >= 1;
  } catch (const std::exception &e) {
    spdlog::error("deleteSuspendPointMetadata failed: {}", e.what());
    return false;
  }
}
} // namespace firelight::saves
