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
  m.contentId = q.getColumn("content_id").getString();
  m.slotNumber = static_cast<unsigned int>(q.getColumn("slot_number").getInt());
  m.savefileMd5 = q.getColumn("savefile_md5").getString();
  m.lastModifiedAt = q.getColumn("last_modified_at").getInt64();
  m.createdAt = q.getColumn("created_at").getInt64();
  return m;
}

SuspendPointMetadata readSuspend(SQLite::Statement &q) {
  SuspendPointMetadata m;
  m.id = q.getColumn("id").getInt();
  m.contentId = q.getColumn("content_id").getString();
  m.saveSlotNumber = q.getColumn("save_slot_number").getInt();
  m.slotNumber = static_cast<unsigned int>(q.getColumn("slot_number").getInt());
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
  const std::vector<migrations::Migration> schema = {
      {1,
       [this] {
         m_db->exec("CREATE TABLE IF NOT EXISTS savefile_metadata("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "content_id TEXT NOT NULL,"
                    "slot_number INTEGER NOT NULL,"
                    "savefile_md5 TEXT NOT NULL,"
                    "last_modified_at INTEGER NOT NULL,"
                    "created_at INTEGER NOT NULL,"
                    "UNIQUE(content_id, slot_number));");

         m_db->exec("CREATE TABLE IF NOT EXISTS suspend_point_metadata("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "content_id TEXT NOT NULL,"
                    "save_slot_number INTEGER NOT NULL,"
                    "slot_number INTEGER NOT NULL,"
                    "locked INTEGER NOT NULL DEFAULT 0,"
                    "last_modified_at INTEGER NOT NULL,"
                    "created_at INTEGER NOT NULL,"
                    "UNIQUE(content_id, slot_number));");
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
    SQLite::Statement q(*m_db, "INSERT INTO savefile_metadata (content_id, slot_number, "
                               "savefile_md5, last_modified_at, created_at) VALUES "
                               "(:contentId, :slotNumber, :savefileMd5, :lastModifiedAt, "
                               ":createdAt);");
    q.bind(":contentId", metadata.contentId);
    q.bind(":slotNumber", static_cast<int>(metadata.slotNumber));
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

std::optional<SavefileMetadata> SqliteSaveDatabase::getSavefileMetadata(std::string contentId, int slotNumber) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement q(*m_db, "SELECT * FROM savefile_metadata WHERE content_id = "
                               ":contentId AND slot_number = :slotNumber LIMIT 1;");
    q.bind(":contentId", contentId);
    q.bind(":slotNumber", slotNumber);
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

std::vector<SavefileMetadata> SqliteSaveDatabase::getSavefileMetadataForContent(std::string contentId) {
  std::lock_guard lock(m_mutex);
  std::vector<SavefileMetadata> result;
  try {
    SQLite::Statement q(*m_db, "SELECT * FROM savefile_metadata WHERE content_id = :contentId;");
    q.bind(":contentId", contentId);
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
    SQLite::Statement q(*m_db, "INSERT INTO suspend_point_metadata (content_id, "
                               "save_slot_number, slot_number, locked, last_modified_at, "
                               "created_at) VALUES (:contentId, :saveSlotNumber, :slotNumber, "
                               ":locked, :lastModifiedAt, :createdAt);");
    q.bind(":contentId", metadata.contentId);
    q.bind(":saveSlotNumber", metadata.saveSlotNumber);
    q.bind(":slotNumber", static_cast<int>(metadata.slotNumber));
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

std::optional<SuspendPointMetadata> SqliteSaveDatabase::getSuspendPointMetadata(std::string contentId,
                                                                                int saveSlotNumber, int slotNumber) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement q(*m_db, "SELECT * FROM suspend_point_metadata WHERE content_id = "
                               ":contentId AND save_slot_number = :saveSlotNumber AND "
                               "slot_number = :slotNumber LIMIT 1;");
    q.bind(":contentId", contentId);
    q.bind(":saveSlotNumber", saveSlotNumber);
    q.bind(":slotNumber", slotNumber);
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

std::vector<SuspendPointMetadata> SqliteSaveDatabase::getSuspendPointMetadataForContent(std::string contentId,
                                                                                        int saveSlotNumber) {
  std::lock_guard lock(m_mutex);
  std::vector<SuspendPointMetadata> result;
  try {
    SQLite::Statement q(*m_db, "SELECT * FROM suspend_point_metadata WHERE content_id = "
                               ":contentId AND save_slot_number = :saveSlotNumber;");
    q.bind(":contentId", contentId);
    q.bind(":saveSlotNumber", saveSlotNumber);
    while (q.executeStep()) {
      result.emplace_back(readSuspend(q));
    }
  } catch (const std::exception &e) {
    spdlog::error("getSuspendPointMetadataForContent failed: {}", e.what());
  }
  return result;
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
