#include "firelight/event_dispatcher.hpp"

#include <firelight/library/library_events.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/migrations/migration_runner.hpp>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>
#include <algorithm>
#include <chrono>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>

namespace firelight::library {
namespace {
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

int64_t nowMs() { return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(); }

int64_t nowSecs() { return duration_cast<seconds>(system_clock::now().time_since_epoch()).count(); }

bool startsWith(const std::string &value, const std::string &prefix) {
  return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

// Builds an Entry from the current row of a `SELECT e.*` / `SELECT *` over
// entriesv1. Does not read folder ids or file locations (the loaders add those)
Entry deserializeEntry(const SQLite::Statement &query) {
  return Entry{
      .id = query.getColumn("id").getInt(),
      .displayName = query.getColumn("display_name").getString(),
      .nameUserSet = query.getColumn("name_user_set").getInt() != 0,
      .contentHash = query.getColumn("content_hash").getString(),
      .platformId = query.getColumn("platform_id").getUInt(),
      .activeSaveSlot = query.getColumn("active_save_slot").getUInt(),
      .hidden = query.getColumn("hidden").getInt() != 0,
      .favorite = query.getColumn("favorite").getInt() != 0,
      .icon1x1SourceUrl = query.getColumn("icon_1x1_source_url").getString(),
      .boxartFrontSourceUrl = query.getColumn("boxart_front_source_url").getString(),
      .boxartBackSourceUrl = query.getColumn("boxart_back_source_url").getString(),
      .description = query.getColumn("description").getString(),
      .releaseYear = query.getColumn("release_year").getUInt(),
      .developer = query.getColumn("developer").getString(),
      .publisher = query.getColumn("publisher").getString(),
      .genres = query.getColumn("genres").getString(),
      .regionIds = query.getColumn("region_ids").getString(),
      .retroachievementsSetId = query.getColumn("retroachievements_set_id").getUInt(),
      .createdAt = static_cast<uint64_t>(query.getColumn("created_at").getInt64()),
  };
}

// Reads a ContentFile from the current row of a `SELECT *` over content_files
ContentFile deserializeContentFile(SQLite::Statement &query) {
  return ContentFile{
      .m_id = query.getColumn("id").getInt(),
      .m_type = static_cast<ContentType>(query.getColumn("content_type").getInt()),
      .m_fileSizeBytes = static_cast<size_t>(query.getColumn("file_size").getInt64()),
      .m_filePath = query.getColumn("file_path").getString(),
      .m_fileMd5 = query.getColumn("file_md5").getString(),
      .m_inArchive = query.getColumn("in_archive").getInt() != 0,
      .m_archivePathName = query.getColumn("archive_file_path").getString(),
      .m_platformId = query.getColumn("platform_id").getInt(),
      .m_contentHash = query.getColumn("content_hash").getString(),
      .m_contentDirectoryId = query.getColumn("content_directory_id").getInt(),
  };
}
} // namespace

SqliteUserLibraryRepository::SqliteUserLibraryRepository(QString path) : m_databasePath(path.toStdString()) {
  m_db = std::make_unique<SQLite::Database>(m_databasePath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

  // Forward-only schema migrations (see migration_runner). A future change adds
  // the next-numbered migration. The ensureColumnExists() calls below remain a
  // belt-and-suspenders for databases predating this runner
  const std::vector<migrations::Migration> schema = {
      {1, [this] {
         m_db->exec("CREATE TABLE IF NOT EXISTS content_files("
                    "id INTEGER PRIMARY KEY,"
                    "file_path TEXT NOT NULL,"
                    "file_size INTEGER NOT NULL,"
                    "file_md5 TEXT NOT NULL,"
                    "file_crc32 TEXT NOT NULL,"
                    "in_archive INTEGER NOT NULL DEFAULT 0,"
                    "archive_file_path TEXT,"
                    "platform_id INTEGER NOT NULL,"
                    "content_hash TEXT NOT NULL,"
                    "content_type INTEGER NOT NULL DEFAULT 0,"
                    "content_directory_id INTEGER NOT NULL DEFAULT -1,"
                    "created_at INTEGER NOT NULL);");

         // A multi-file disc set's member tracks/discs, keyed to the primary
         // ContentFile (the cue/gdi/m3u sheet) in content_files
         m_db->exec("CREATE TABLE IF NOT EXISTS disc_members("
                    "id INTEGER PRIMARY KEY,"
                    "content_file_id INTEGER NOT NULL,"
                    "path TEXT NOT NULL,"
                    "role TEXT NOT NULL,"
                    "sort_index INTEGER NOT NULL DEFAULT 0,"
                    "created_at INTEGER NOT NULL,"
                    "UNIQUE (content_file_id, path));");

         m_db->exec("CREATE TABLE IF NOT EXISTS patch_files("
                    "id INTEGER PRIMARY KEY,"
                    "file_path TEXT UNIQUE NOT NULL,"
                    "file_size INTEGER NOT NULL,"
                    "file_md5 TEXT NOT NULL,"
                    "file_crc32 TEXT NOT NULL,"
                    "target_md5 TEXT,"
                    "patched_md5 TEXT,"
                    "patched_content_hash TEXT,"
                    "in_archive INTEGER NOT NULL DEFAULT 0,"
                    "archive_file_path TEXT,"
                    "created_at INTEGER NOT NULL);");

         m_db->exec("CREATE TABLE IF NOT EXISTS entriesv1("
                    "id INTEGER PRIMARY KEY,"
                    "display_name TEXT NOT NULL,"
                    "content_hash TEXT NOT NULL,"
                    "platform_id INTEGER NOT NULL,"
                    "active_save_slot INTEGER NOT NULL DEFAULT 1, "
                    "hidden INTEGER NOT NULL DEFAULT 0, "
                    "favorite INTEGER NOT NULL DEFAULT 0, "
                    "icon_1x1_source_url TEXT, "
                    "icon_2x3_source_url TEXT,"
                    "icon_92x43_source_url TEXT, "
                    "boxart_front_source_url TEXT, "
                    "boxart_back_source_url TEXT, "
                    "clear_logo_source_url TEXT, "
                    "hero_image_source_url TEXT, "
                    "description TEXT, "
                    "release_year INTEGER, "
                    "developer TEXT, "
                    "publisher TEXT, "
                    "genres TEXT, "
                    "region_ids TEXT, "
                    "retroachievements_set_id INTEGER, "
                    "name_user_set INTEGER NOT NULL DEFAULT 0, "
                    "created_at INTEGER NOT NULL);");

         m_db->exec("CREATE TABLE IF NOT EXISTS run_configurations("
                    "id INTEGER PRIMARY KEY,"
                    "type TEXT NOT NULL,"
                    "content_hash TEXT NOT NULL,"
                    "content_file_id INTEGER NOT NULL,"
                    "patch_id INTEGER,"
                    "created_at INTEGER NOT NULL,"
                    "UNIQUE (type, content_file_id, patch_id),"
                    "UNIQUE (type, content_file_id));");

         m_db->exec("CREATE TABLE IF NOT EXISTS content_directoriesv1("
                    "id INTEGER PRIMARY KEY,"
                    "path TEXT UNIQUE NOT NULL,"
                    "num_files INTEGER NOT NULL DEFAULT 0,"
                    "num_content_files INTEGER NOT NULL DEFAULT 0,"
                    "last_modified INTEGER DEFAULT 0,"
                    "recursive INTEGER NOT NULL DEFAULT 1,"
                    "created_at INTEGER NOT NULL);");

         m_db->exec("CREATE TABLE IF NOT EXISTS folders("
                    "id INTEGER PRIMARY KEY,"
                    "display_name TEXT UNIQUE NOT NULL,"
                    "description TEXT,"
                    "icon_source_url TEXT,"
                    "type INTEGER NOT NULL DEFAULT 0,"
                    "filter_json TEXT,"
                    "color TEXT,"
                    "sort_role TEXT,"
                    "sort_ascending INTEGER NOT NULL DEFAULT 1,"
                    "parent_id INTEGER NOT NULL DEFAULT -1,"
                    "position INTEGER NOT NULL DEFAULT 0,"
                    "created_at INTEGER NOT NULL);");

         m_db->exec("CREATE TABLE IF NOT EXISTS folder_entries("
                    "folder_id INTEGER NOT NULL,"
                    "entry_id INTEGER NOT NULL,"
                    "created_at INTEGER NOT NULL,"
                    "UNIQUE (folder_id, entry_id));");

         m_db->exec("CREATE UNIQUE INDEX IF NOT EXISTS pathIdx ON "
                    "content_files(file_path);");

         // folder_entries is looked up by entry_id (per-entry, in loops). The
         // UNIQUE(folder_id, entry_id) index can't serve entry_id-only lookups, so
         // add a dedicated index to avoid table scans
         m_db->exec("CREATE INDEX IF NOT EXISTS folderEntryEntryIdx ON "
                    "folder_entries(entry_id);");

         // content_hash is the primary lookup key for entries, run configurations and
         // content files (loadEntry + library scanning), so index each
         m_db->exec("CREATE INDEX IF NOT EXISTS entriesContentHashIdx ON "
                    "entriesv1(content_hash);");
         m_db->exec("CREATE INDEX IF NOT EXISTS runConfigContentHashIdx ON "
                    "run_configurations(content_hash);");
         m_db->exec("CREATE INDEX IF NOT EXISTS contentFileContentHashIdx ON "
                    "content_files(content_hash);");
       }}};

  try {
    SQLite::Transaction transaction(*m_db);
    const int currentVersion = m_db->execAndGet("PRAGMA user_version").getInt();
    migrations::applyMigrations(currentVersion, schema,
                                [this](const int v) { m_db->exec("PRAGMA user_version = " + std::to_string(v)); });
    transaction.commit();
  } catch (const std::exception &e) {
    spdlog::error("Failed to initialize user library schema: {}", e.what());
  }

  // Migrate databases created before these columns existed. CREATE TABLE IF
  // NOT EXISTS won't add columns to an existing table, so add them here;
  // otherwise reads/writes referencing them fail on older databases
  ensureColumnExists("content_files", "content_directory_id", "INTEGER NOT NULL DEFAULT -1");
  ensureColumnExists("folders", "type", "INTEGER NOT NULL DEFAULT 0");
  ensureColumnExists("folders", "filter_json", "TEXT");
  ensureColumnExists("folders", "color", "TEXT");
  ensureColumnExists("folders", "sort_role", "TEXT");
  ensureColumnExists("folders", "sort_ascending", "INTEGER NOT NULL DEFAULT 1");
  ensureColumnExists("folders", "parent_id", "INTEGER NOT NULL DEFAULT -1");
  ensureColumnExists("folders", "position", "INTEGER NOT NULL DEFAULT 0");
  ensureColumnExists("entriesv1", "name_user_set", "INTEGER NOT NULL DEFAULT 0");

  // Give pre-existing content files their directory (new files get
  // it stamped at insert time in create(ContentFile))
  backfillContentDirectoryIds();

  // The default content directory is guaranteed by UserLibraryService, not
  // seeded here. The scan-time orchestration (content file -> run
  // configuration -> entry) lives in LibraryIngestService, which subscribes to
  // the events published below
}

SqliteUserLibraryRepository::~SqliteUserLibraryRepository() = default;

void SqliteUserLibraryRepository::ensureColumnExists(const std::string &table, const std::string &column,
                                                     const std::string &definition) {
  try {
    // PRAGMA/DDL identifiers can't be parameterized; table/column/definition are
    // internal constants
    SQLite::Statement info(*m_db, "PRAGMA table_info(" + table + ");");
    while (info.executeStep()) {
      if (info.getColumn("name").getString() == column) {
        return; // already present
      }
    }
    m_db->exec("ALTER TABLE " + table + " ADD COLUMN " + column + " " + definition + ";");
  } catch (const std::exception &e) {
    spdlog::error("Failed to ensure column {}.{}: {}", table, column, e.what());
  }
}

int SqliteUserLibraryRepository::resolveContentDirectoryId(const std::string &onDiskPath) {
  int bestId = -1;
  int bestLen = -1;
  for (const auto &dir : getContentDirectories()) {
    if (startsWith(onDiskPath, dir.path) && static_cast<int>(dir.path.length()) > bestLen) {
      bestId = dir.id;
      bestLen = static_cast<int>(dir.path.length());
    }
  }
  return bestId;
}

void SqliteUserLibraryRepository::backfillContentDirectoryIds() {
  if (getContentDirectories().empty()) {
    return;
  }
  for (const auto &cf : getContentFiles()) {
    if (cf.m_contentDirectoryId >= 0) {
      continue; // already stamped
    }
    const auto onDisk = cf.m_inArchive ? cf.m_archivePathName : cf.m_filePath;
    const int dirId = resolveContentDirectoryId(onDisk);
    if (dirId < 0) {
      continue;
    }
    try {
      SQLite::Statement upd(*m_db, "UPDATE content_files SET content_directory_id = "
                                   ":dirId WHERE id = :id;");
      upd.bind(":dirId", dirId);
      upd.bind(":id", cf.m_id);
      upd.exec();
    } catch (const std::exception &e) {
      spdlog::error("Failed to backfill content_directory_id for file {}: {}", cf.m_id, e.what());
    }
  }
}

bool SqliteUserLibraryRepository::create(FolderInfo &folder) {
  std::lock_guard lock(m_mutex);
  try {
    // New folders append to the end of their parent's ordering
    folder.position = nextFolderPosition(folder.parentId);

    SQLite::Statement query(*m_db, "INSERT INTO folders(display_name, description, icon_source_url, "
                                   "type, filter_json, color, sort_role, sort_ascending, parent_id, "
                                   "position, created_at) VALUES"
                                   "(:displayName, :description, :iconSourceUrl, :type, :filterJson, "
                                   ":color, :sortRole, :sortAscending, :parentId, :position, "
                                   ":createdAt);");
    query.bind(":displayName", folder.displayName);
    query.bind(":description", folder.description);
    query.bind(":iconSourceUrl", folder.iconSourceUrl);
    query.bind(":type", folder.type);
    query.bind(":filterJson", folder.filterJson);
    query.bind(":color", folder.color);
    query.bind(":sortRole", folder.sortRole);
    query.bind(":sortAscending", folder.sortAscending ? 1 : 0);
    query.bind(":parentId", folder.parentId);
    query.bind(":position", folder.position);
    query.bind(":createdAt", nowSecs());
    query.exec();

    folder.id = static_cast<int>(m_db->getLastInsertRowid());
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to create folder: {}", e.what());
    return false;
  }
}

int SqliteUserLibraryRepository::nextFolderPosition(int parentId) {
  try {
    SQLite::Statement query(*m_db, "SELECT COALESCE(MAX(position), -1) + 1 AS next "
                                   "FROM folders WHERE parent_id = :parentId;");
    query.bind(":parentId", parentId);
    if (query.executeStep()) {
      return query.getColumn("next").getInt();
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to compute next folder position: {}", e.what());
  }
  return 0;
}

bool SqliteUserLibraryRepository::create(FolderEntryInfo &folderEntry) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "INSERT INTO folder_entries(folder_id, entry_id, "
                                   "created_at) VALUES(:folderId, :entryId, "
                                   ":createdAt);");
    query.bind(":folderId", folderEntry.folderId);
    query.bind(":entryId", folderEntry.entryId);
    query.bind(":createdAt", nowSecs());
    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to create folder entry: {}", e.what());
    return false;
  }
}

std::vector<FolderInfo> SqliteUserLibraryRepository::listFolders() {
  std::lock_guard lock(m_mutex);
  std::vector<FolderInfo> folders;
  try {
    // Order within each parent scope by the manual position, so callers get
    // folders in user order (and can group by parent_id for the nested tree)
    SQLite::Statement query(*m_db, "SELECT * FROM folders ORDER BY parent_id, position, id");
    while (query.executeStep()) {
      folders.emplace_back(FolderInfo{
          .id = query.getColumn("id").getInt(),
          .displayName = query.getColumn("display_name").getString(),
          .description = query.getColumn("description").getString(),
          .iconSourceUrl = query.getColumn("icon_source_url").getString(),
          .type = query.getColumn("type").getInt(),
          .filterJson = query.getColumn("filter_json").getString(),
          .color = query.getColumn("color").getString(),
          .sortRole = query.getColumn("sort_role").getString(),
          .sortAscending = query.getColumn("sort_ascending").getInt() != 0,
          .parentId = query.getColumn("parent_id").getInt(),
          .position = query.getColumn("position").getInt(),
          .createdAt = static_cast<uint64_t>(query.getColumn("created_at").getInt64()),
      });
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get folders: {}", e.what());
    return {};
  }
  return folders;
}

bool SqliteUserLibraryRepository::deleteFolder(int folderId) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "DELETE FROM folders WHERE id = :folderId;");
    query.bind(":folderId", folderId);
    query.exec();

    SQLite::Statement deleteEntriesQuery(*m_db, "DELETE FROM folder_entries WHERE folder_id = :folderId;");
    deleteEntriesQuery.bind(":folderId", folderId);
    deleteEntriesQuery.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to delete folder with ID {}: {}", folderId, e.what());
    return false;
  }
}

bool SqliteUserLibraryRepository::update(FolderInfo &folder) {
  std::lock_guard lock(m_mutex);
  if (folder.id <= 0) {
    spdlog::error("Cannot update folder with invalid ID: {}", folder.id);
    return false;
  }

  try {
    // Ordering (parent_id/position) is managed by reorderFolders/setFolderParent,
    // not here, so a stale FolderInfo can't clobber the user's arrangement
    SQLite::Statement query(*m_db, "UPDATE folders SET display_name = :displayName, "
                                   "description = :description, icon_source_url = :iconSourceUrl, "
                                   "type = :type, filter_json = :filterJson, color = :color, "
                                   "sort_role = :sortRole, sort_ascending = :sortAscending "
                                   "WHERE id = :folderId;");
    query.bind(":folderId", folder.id);
    query.bind(":displayName", folder.displayName);
    query.bind(":description", folder.description);
    query.bind(":iconSourceUrl", folder.iconSourceUrl);
    query.bind(":type", folder.type);
    query.bind(":filterJson", folder.filterJson);
    query.bind(":color", folder.color);
    query.bind(":sortRole", folder.sortRole);
    query.bind(":sortAscending", folder.sortAscending ? 1 : 0);

    if (query.exec() == 0) {
      spdlog::error("Failed to update folder with ID {}: no rows affected", folder.id);
      return false;
    }
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to update folder with ID {}: {}", folder.id, e.what());
    return false;
  }
}

bool SqliteUserLibraryRepository::reorderFolders(const int parentId, const std::vector<int> &orderedFolderIds) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Transaction transaction(*m_db);
    for (size_t i = 0; i < orderedFolderIds.size(); ++i) {
      SQLite::Statement query(*m_db, "UPDATE folders SET position = :position "
                                     "WHERE id = :folderId AND parent_id = :parentId;");
      query.bind(":position", static_cast<int>(i));
      query.bind(":folderId", orderedFolderIds[i]);
      query.bind(":parentId", parentId);
      query.exec();
    }
    transaction.commit();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to reorder folders: {}", e.what());
    return false;
  }
}

bool SqliteUserLibraryRepository::setFolderParent(const int folderId, const int newParentId) {
  std::lock_guard lock(m_mutex);
  try {
    // Moving to a new parent appends the folder to the end of that parent's
    // ordering
    SQLite::Statement query(*m_db, "UPDATE folders SET parent_id = :parentId, position = :position "
                                   "WHERE id = :folderId;");
    query.bind(":parentId", newParentId);
    query.bind(":position", nextFolderPosition(newParentId));
    query.bind(":folderId", folderId);
    if (query.exec() == 0) {
      spdlog::error("Failed to set parent of folder {} to {}: no rows affected", folderId, newParentId);
      return false;
    }
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to set parent of folder {} to {}: {}", folderId, newParentId, e.what());
    return false;
  }
}

bool SqliteUserLibraryRepository::deleteFolderEntry(FolderEntryInfo &info) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "DELETE FROM folder_entries WHERE folder_id = "
                                   ":folderId AND entry_id = :entryId;");
    query.bind(":folderId", info.folderId);
    query.bind(":entryId", info.entryId);
    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to delete folder entry: {}", e.what());
    return false;
  }
}

bool SqliteUserLibraryRepository::update(Entry &entry) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "UPDATE entriesv1 SET display_name = :displayName, "
                                   "active_save_slot = :activeSaveSlot, hidden = :hidden, "
                                   "favorite = :favorite, name_user_set = :nameUserSet "
                                   "WHERE id = :id;");
    query.bind(":id", entry.id);
    query.bind(":displayName", entry.displayName);
    query.bind(":activeSaveSlot", entry.activeSaveSlot);
    query.bind(":hidden", entry.hidden ? 1 : 0);
    query.bind(":favorite", entry.favorite ? 1 : 0);
    query.bind(":nameUserSet", entry.nameUserSet ? 1 : 0);
    if (query.exec() == 0) {
      spdlog::error("Failed to update entry with ID {}: no rows affected", entry.id);
      return false;
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to update entry with ID {}: {}", entry.id, e.what());
    return false;
  }

  EventDispatcher::instance().publish(EntryUpdatedEvent{.entryId = entry.id});
  return true;
}

bool SqliteUserLibraryRepository::updateEntryMetadata(const Entry &entry) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "UPDATE entriesv1 SET display_name = :displayName, "
                                   "name_user_set = :nameUserSet, description = :description, "
                                   "developer = :developer, publisher = :publisher, "
                                   "genres = :genres, region_ids = :regionIds, "
                                   "release_year = :releaseYear, "
                                   "retroachievements_set_id = :raSetId, "
                                   "icon_1x1_source_url = :icon1x1, "
                                   "boxart_front_source_url = :boxartFront, "
                                   "boxart_back_source_url = :boxartBack WHERE id = :id;");
    query.bind(":id", entry.id);
    query.bind(":displayName", entry.displayName);
    query.bind(":nameUserSet", entry.nameUserSet ? 1 : 0);
    query.bind(":description", entry.description);
    query.bind(":developer", entry.developer);
    query.bind(":publisher", entry.publisher);
    query.bind(":genres", entry.genres);
    query.bind(":regionIds", entry.regionIds);
    query.bind(":releaseYear", entry.releaseYear);
    query.bind(":raSetId", entry.retroachievementsSetId);
    query.bind(":icon1x1", entry.icon1x1SourceUrl);
    query.bind(":boxartFront", entry.boxartFrontSourceUrl);
    query.bind(":boxartBack", entry.boxartBackSourceUrl);

    if (query.exec() == 0) {
      return false;
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to update entry metadata for ID {}: {}", entry.id, e.what());
    return false;
  }

  EventDispatcher::instance().publish(EntryUpdatedEvent{.entryId = entry.id});
  return true;
}

bool SqliteUserLibraryRepository::deleteContentDirectory(int id) {
  std::lock_guard lock(m_mutex);
  std::string path;
  try {
    SQLite::Statement selectQuery(*m_db, "SELECT path FROM content_directoriesv1 WHERE id = :id;");
    selectQuery.bind(":id", id);

    if (!selectQuery.executeStep()) {
      return true; // Nothing to delete
    }

    path = selectQuery.getColumn("path").getString();
  } catch (const std::exception &e) {
    spdlog::error("Failed to get content directory with ID {}: {}", id, e.what());
    return false;
  }

  try {
    SQLite::Statement query(*m_db, "DELETE FROM content_directoriesv1 WHERE id = :id;");
    query.bind(":id", id);
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to delete content directory with ID {}: {}", id, e.what());
    return false;
  }

  for (const auto &rom : getContentFiles()) {
    auto romPath = rom.m_inArchive ? rom.m_archivePathName : rom.m_filePath;

    if (startsWith(romPath, path)) {
      auto found = false;
      for (const auto &contentPath : getContentDirectories()) {
        if (startsWith(romPath, contentPath.path)) {
          found = true;
          break;
        }
      }

      if (!found) {
        deleteContentFile(rom.m_id);
      }
    }
  }

  EventDispatcher::instance().publish(ContentDirectoryRemovedEvent{.id = id, .path = path});
  return true;
}

bool SqliteUserLibraryRepository::create(ContentFile &romFile) {
  std::lock_guard lock(m_mutex);

  if (romFile.m_contentDirectoryId < 0) {
    const auto onDisk = romFile.m_inArchive ? romFile.m_archivePathName : romFile.m_filePath;
    romFile.m_contentDirectoryId = resolveContentDirectoryId(onDisk);
  }

  try {
    SQLite::Statement query(*m_db, "INSERT INTO content_files (file_path, file_size, file_md5, "
                                   "file_crc32, in_archive, archive_file_path, platform_id, "
                                   "content_hash, content_type, content_directory_id, created_at) "
                                   "VALUES (:filePath, :fileSize, :fileMd5, :fileCrc32, :inArchive, "
                                   ":archiveFilePath, :platformId, :contentHash, :contentType, "
                                   ":contentDirectoryId, :createdAt);");
    query.bind(":filePath", romFile.m_filePath);
    query.bind(":fileSize", static_cast<int64_t>(romFile.m_fileSizeBytes));
    query.bind(":fileMd5", romFile.m_fileMd5);
    query.bind(":fileCrc32", romFile.m_fileCrc32);
    query.bind(":inArchive", romFile.m_inArchive ? 1 : 0);
    query.bind(":archiveFilePath", romFile.m_archivePathName);
    query.bind(":platformId", romFile.m_platformId);
    query.bind(":contentHash", romFile.m_contentHash);
    query.bind(":contentType", static_cast<int>(romFile.m_type));
    query.bind(":contentDirectoryId", romFile.m_contentDirectoryId);
    query.bind(":createdAt", nowMs());
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to add rom file with path {}: {}", romFile.m_filePath, e.what());
    return false;
  }

  romFile.m_id = static_cast<int>(m_db->getLastInsertRowid());

  EventDispatcher::instance().publish(ContentFileAddedEvent{.id = romFile.m_id,
                                                            .filePath = romFile.m_filePath,
                                                            .platformId = romFile.m_platformId,
                                                            .contentHash = romFile.m_contentHash});
  return true;
}

std::optional<ContentFile> SqliteUserLibraryRepository::getContentFileWithPathAndSize(const std::string &filePath,
                                                                                      const size_t fileSizeBytes,
                                                                                      const bool inArchive) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM content_files WHERE file_path = :filePath AND "
                                   "file_size = :fileSize AND in_archive = :inArchive;");
    query.bind(":filePath", filePath);
    query.bind(":fileSize", static_cast<int64_t>(fileSizeBytes));
    query.bind(":inArchive", inArchive ? 1 : 0);

    if (!query.executeStep()) {
      return std::nullopt;
    }

    return ContentFile{
        .m_id = query.getColumn("id").getInt(),
        .m_type = static_cast<ContentType>(query.getColumn("content_type").getInt()),
        .m_fileSizeBytes = static_cast<size_t>(query.getColumn("file_size").getInt64()),
        .m_filePath = query.getColumn("file_path").getString(),
        .m_fileMd5 = query.getColumn("file_md5").getString(),
        .m_inArchive = query.getColumn("in_archive").getInt() != 0,
        .m_archivePathName = query.getColumn("archive_file_path").getString(),
        .m_platformId = query.getColumn("platform_id").getInt(),
        .m_contentHash = query.getColumn("content_hash").getString(),
        .m_contentDirectoryId = query.getColumn("content_directory_id").getInt(),
    };
  } catch (const std::exception &e) {
    spdlog::error("Failed to get rom file with path {}: {}", filePath, e.what());
    return std::nullopt;
  }
}

bool SqliteUserLibraryRepository::deleteContentFile(int id) {
  std::lock_guard lock(m_mutex);
  std::string contentHash;

  try {
    SQLite::Statement query(*m_db, "SELECT content_hash FROM content_files WHERE id = :id;");
    query.bind(":id", id);
    if (!query.executeStep()) {
      return true; // Nothing to delete
    }
    contentHash = query.getColumn("content_hash").getString();

    SQLite::Statement deleteQuery(*m_db, "DELETE FROM content_files WHERE id = :id;");
    deleteQuery.bind(":id", id);
    deleteQuery.exec();

    SQLite::Statement deleteRunConfigsQuery(*m_db, "DELETE FROM run_configurations WHERE type = 'rom' AND "
                                                   "content_file_id = :contentFileId;");
    deleteRunConfigsQuery.bind(":contentFileId", id);
    deleteRunConfigsQuery.exec();

    SQLite::Statement deleteDiscMembersQuery(*m_db, "DELETE FROM disc_members WHERE content_file_id = :id;");
    deleteDiscMembersQuery.bind(":id", id);
    deleteDiscMembersQuery.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to delete rom file with ID {}: {}", id, e.what());
    return false;
  }

  EventDispatcher::instance().publish(RunConfigurationDeletedEvent{.contentHash = contentHash});
  return true;
}

std::vector<Entry> SqliteUserLibraryRepository::getEntries(int offset, int limit) {
  std::lock_guard lock(m_mutex);
  std::vector<Entry> entries;

  try {
    SQLite::Statement query(*m_db, R"(
            SELECT e.*,
                   CASE
                       WHEN EXISTS (SELECT 1 FROM content_files rf WHERE rf.content_hash = e.content_hash)
                       THEN 1
                       ELSE 0
                   END AS has_rom
            FROM entriesv1 e
            ORDER BY e.display_name ASC;
        )");

    while (query.executeStep()) {
      entries.emplace_back(deserializeEntry(query));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get entries: {}", e.what());
    return {};
  }

  for (auto &entry : entries) {
    try {
      SQLite::Statement folderQuery(*m_db, "SELECT folder_id FROM folder_entries WHERE entry_id = :entryId;");
      folderQuery.bind(":entryId", entry.id);

      while (folderQuery.executeStep()) {
        entry.folderIds.push_back(folderQuery.getColumn("folder_id").getInt());
      }
    } catch (const std::exception &e) {
      spdlog::error("Failed to get folder IDs for entry {}: {}", entry.id, e.what());
    }
    populateEntrySource(entry);
  }

  return entries;
}

std::optional<Entry> SqliteUserLibraryRepository::getEntry(const int entryId) {
  std::lock_guard lock(m_mutex);
  Entry entry;

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM entriesv1 WHERE id = :entryId LIMIT 1;");
    query.bind(":entryId", entryId);

    if (!query.executeStep()) {
      spdlog::error("Failed to get entry with ID {}: not found", entryId);
      return {};
    }

    entry = deserializeEntry(query);
  } catch (const std::exception &e) {
    spdlog::error("Failed to get entry: {}", e.what());
    return {};
  }

  try {
    SQLite::Statement folderQuery(*m_db, "SELECT folder_id FROM folder_entries WHERE entry_id = :entryId;");
    folderQuery.bind(":entryId", entry.id);

    while (folderQuery.executeStep()) {
      entry.folderIds.push_back(folderQuery.getColumn("folder_id").getInt());
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get folder IDs for entry {}: {}", entry.id, e.what());
  }

  populateEntrySource(entry);
  return entry;
}

std::optional<Entry> SqliteUserLibraryRepository::getEntryWithContentHash(const std::string &contentHash) {
  std::lock_guard lock(m_mutex);
  Entry entry;

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM entriesv1 WHERE content_hash = :contentHash LIMIT 1;");
    query.bind(":contentHash", contentHash);

    if (!query.executeStep()) {
      return {};
    }

    entry = deserializeEntry(query);
  } catch (const std::exception &e) {
    spdlog::error("Failed to get entry with content hash {}: {}", contentHash, e.what());
    return {};
  }

  try {
    SQLite::Statement folderQuery(*m_db, "SELECT folder_id FROM folder_entries WHERE entry_id = :entryId;");
    folderQuery.bind(":entryId", entry.id);

    while (folderQuery.executeStep()) {
      entry.folderIds.push_back(folderQuery.getColumn("folder_id").getInt());
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get folder IDs for entry {}: {}", entry.id, e.what());
  }

  populateEntrySource(entry);
  return entry;
}

void SqliteUserLibraryRepository::populateEntrySource(Entry &entry) {
  try {
    SQLite::Statement query(*m_db, "SELECT content_directory_id, file_path, in_archive, "
                                   "archive_file_path FROM content_files WHERE content_hash = "
                                   ":contentHash;");
    query.bind(":contentHash", entry.contentHash);

    while (query.executeStep()) {
      const int dirId = query.getColumn("content_directory_id").getInt();
      if (dirId >= 0 && std::ranges::find(entry.contentDirectoryIds, dirId) == entry.contentDirectoryIds.end()) {
        entry.contentDirectoryIds.push_back(dirId);
      }

      const auto path = query.getColumn("in_archive").getInt() != 0 ? query.getColumn("archive_file_path").getString()
                                                                    : query.getColumn("file_path").getString();
      entry.contentPaths.push_back(path);
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get content locations for entry {}: {}", entry.id, e.what());
  }
}

std::vector<RunConfiguration> SqliteUserLibraryRepository::getRunConfigurations(const std::string &contentHash) {
  std::lock_guard lock(m_mutex);
  std::vector<RunConfiguration> runConfigurations;

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM run_configurations WHERE content_hash = :contentHash;");
    query.bind(":contentHash", contentHash);

    while (query.executeStep()) {
      runConfigurations.push_back(RunConfiguration{
          .id = query.getColumn("id").getInt(),
          .type = query.getColumn("type").getString(),
          .contentHash = contentHash,
          .contentFileId = query.getColumn("content_file_id").getInt(),
          .patchId = query.getColumn("patch_id").getInt(),
          .createdAt = static_cast<uint32_t>(query.getColumn("created_at").getInt64()),
      });
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get run configurations: {}", e.what());
  }
  return runConfigurations;
}

std::vector<ContentFile> SqliteUserLibraryRepository::getContentFiles() {
  std::lock_guard lock(m_mutex);
  std::vector<ContentFile> romFiles;

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM content_files;");
    while (query.executeStep()) {
      romFiles.emplace_back(deserializeContentFile(query));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get rom files: {}", e.what());
  }

  return romFiles;
}

std::optional<ContentFile> SqliteUserLibraryRepository::getContentFile(int id) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM content_files WHERE id = :id LIMIT 1;");
    query.bind(":id", id);

    if (!query.executeStep()) {
      return std::nullopt;
    }

    return deserializeContentFile(query);
  } catch (const std::exception &e) {
    spdlog::error("Failed to get rom file with id {}: {}", id, e.what());
    return std::nullopt;
  }
}

bool SqliteUserLibraryRepository::create(DiscMember &member) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement query(*m_db, "INSERT OR IGNORE INTO disc_members (content_file_id, path, "
                                   "role, sort_index, created_at) VALUES (:contentFileId, :path, "
                                   ":role, :sortIndex, :createdAt);");
    query.bind(":contentFileId", member.m_contentFileId);
    query.bind(":path", member.m_path);
    query.bind(":role", member.m_role);
    query.bind(":sortIndex", member.m_sortIndex);
    query.bind(":createdAt", nowMs());
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to add disc member {}: {}", member.m_path, e.what());
    return false;
  }

  member.m_id = static_cast<int>(m_db->getLastInsertRowid());
  return true;
}

std::optional<PatchFile> SqliteUserLibraryRepository::getPatchFile(int id) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM patch_files WHERE id = :id LIMIT 1;");
    query.bind(":id", id);

    if (!query.executeStep()) {
      return std::nullopt;
    }

    PatchFile patchFile;
    patchFile.m_filePath = query.getColumn("file_path").getString();
    patchFile.m_fileSize = query.getColumn("file_size").getInt();
    patchFile.m_fileMd5 = query.getColumn("file_md5").getString();

    return {patchFile};
  } catch (const std::exception &e) {
    spdlog::error("Failed to get patch file with id {}: {}", id, e.what());
    return std::nullopt;
  }
}

void SqliteUserLibraryRepository::create(PatchFile &file) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement query(*m_db, "INSERT OR IGNORE INTO patch_files (file_path, file_size, "
                                   "file_md5, file_crc32, target_md5, patched_md5, "
                                   "patched_content_hash, in_archive, archive_file_path, "
                                   "created_at) VALUES (:filePath, :fileSize, :fileMd5, :fileCrc32, "
                                   ":targetMd5, :patchedMd5, :patchedContentHash, :inArchive, "
                                   ":archiveFilePath, :createdAt);");
    query.bind(":filePath", file.m_filePath);
    query.bind(":fileSize", static_cast<int64_t>(file.m_fileSize));
    query.bind(":fileMd5", file.m_fileMd5);
    query.bind(":fileCrc32", file.m_fileCrc32);
    query.bind(":targetMd5", file.m_targetFileMd5);
    query.bind(":patchedMd5", file.m_patchedMd5);
    query.bind(":patchedContentHash", file.m_patchedContentHash);
    query.bind(":inArchive", file.m_inArchive ? 1 : 0);
    query.bind(":archiveFilePath", file.m_archiveFilePath);
    query.bind(":createdAt", nowMs());
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to add patch file with path {}: {}", file.m_filePath, e.what());
  }
}

std::vector<ContentDirectory> SqliteUserLibraryRepository::getContentDirectories() {
  std::lock_guard lock(m_mutex);
  std::vector<ContentDirectory> directories;

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM content_directoriesv1;");
    while (query.executeStep()) {
      directories.emplace_back(ContentDirectory{
          .id = query.getColumn("id").getInt(),
          .path = query.getColumn("path").getString(),
          .numFiles = query.getColumn("num_files").getInt(),
          .numContentFiles = query.getColumn("num_content_files").getInt(),
          .lastModifiedEpochMs = static_cast<uint64_t>(query.getColumn("last_modified").getInt64()),
          .recursive = query.getColumn("recursive").getInt() != 0,
          .createdAt = query.getColumn("created_at").getUInt(),
      });
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get directories: {}", e.what());
  }

  return directories;
}

bool SqliteUserLibraryRepository::create(ContentDirectory &directory) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement query(*m_db, "INSERT OR IGNORE INTO content_directoriesv1 (path, created_at) "
                                   "VALUES (:path, :createdAt);");
    query.bind(":path", directory.path);
    query.bind(":createdAt", nowMs());

    if (query.exec() == 0) {
      return false; // ignored (already present)
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to add directory with path {}: {}", directory.path, e.what());
    return false;
  }

  directory.id = static_cast<int>(m_db->getLastInsertRowid());

  EventDispatcher::instance().publish(ContentDirectoryAddedEvent{.id = directory.id, .path = directory.path});
  return true;
}

bool SqliteUserLibraryRepository::update(const ContentDirectory &directory) {
  std::lock_guard lock(m_mutex);
  std::string oldPath;

  try {
    SQLite::Statement selectQuery(*m_db, "SELECT path FROM content_directoriesv1 WHERE id = :id;");
    selectQuery.bind(":id", directory.id);

    if (!selectQuery.executeStep()) {
      return true; // nothing to update
    }

    oldPath = selectQuery.getColumn("path").getString();

    SQLite::Statement query(*m_db, "UPDATE content_directoriesv1 SET path = :path WHERE id = :id;");
    query.bind(":path", directory.path);
    query.bind(":id", directory.id);
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to update content directory {}: {}", directory.id, e.what());
    return false;
  }

  EventDispatcher::instance().publish(
      ContentDirectoryUpdatedEvent{.id = directory.id, .oldPath = oldPath, .newPath = directory.path});
  return true;
}

void SqliteUserLibraryRepository::createRunConfiguration(const int contentFileId, const std::string &path,
                                                         const int platformId, const std::string &contentHash) {
  std::lock_guard lock(m_mutex);
  int64_t rowId = 0;

  try {
    SQLite::Statement query(*m_db, "INSERT OR IGNORE INTO run_configurations "
                                   "(type, content_hash, content_file_id, created_at) "
                                   "VALUES (:type, :contentHash, :contentFileId, :createdAt);");
    query.bind(":type", "rom");
    query.bind(":contentHash", contentHash);
    query.bind(":contentFileId", contentFileId);
    query.bind(":createdAt", nowSecs());
    query.exec();

    rowId = m_db->getLastInsertRowid();
  } catch (const std::exception &e) {
    spdlog::error("Failed to create run configuration: {}", e.what());
  }

  EventDispatcher::instance().publish(RunConfigurationCreatedEvent{
      .id = static_cast<int>(rowId), .filePath = path, .platformId = platformId, .contentHash = contentHash});
}

bool SqliteUserLibraryRepository::createEntry(Entry &entry) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement selectQuery(*m_db, "SELECT id FROM entriesv1 WHERE content_hash = :contentHash;");
    selectQuery.bind(":contentHash", entry.contentHash);

    if (selectQuery.executeStep()) {
      spdlog::debug("Entry with content hash {} already exists, skipping creation", entry.contentHash);
      return false;
    }

    SQLite::Statement entryQuery(*m_db, "INSERT INTO entriesv1 (display_name, content_hash, platform_id, "
                                        "created_at) VALUES (:displayName, :contentHash, :platformId, "
                                        ":createdAt);");
    entryQuery.bind(":displayName", entry.displayName);
    entryQuery.bind(":contentHash", entry.contentHash);
    entryQuery.bind(":platformId", entry.platformId);
    entryQuery.bind(":createdAt", nowMs());
    entryQuery.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to add entry with content hash {}: {}", entry.contentHash, e.what());
    return false;
  }

  entry.id = static_cast<int>(m_db->getLastInsertRowid());
  EventDispatcher::instance().publish(EntryCreatedEvent{.entryId = entry.id});
  return true;
}
} // namespace firelight::library
