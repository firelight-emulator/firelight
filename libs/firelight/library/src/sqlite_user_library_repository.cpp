// TODO: NEEDS REVIEW
#include "firelight/event_dispatcher.hpp"

#include <firelight/library/library_events.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/migrations/migration_runner.hpp>
#include <firelight/util/strings.hpp>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>
#include <algorithm>
#include <chrono>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_set>
#include <utility>

namespace firelight::library {
namespace {
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

// Epoch milliseconds, matching every other database
int64_t nowMs() { return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(); }

// Builds an Entry from the current row of a `SELECT e.*` / `SELECT *` over
// entries. Does not read folder ids or file locations (the loaders add those)
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
      .rating = query.getColumn("rating").getUInt(),
      .icon1x1SourceUrl = query.getColumn("icon_1x1_source_url").getString(),
      .boxartFrontSourceUrl = query.getColumn("boxart_front_source_url").getString(),
      .boxartBackSourceUrl = query.getColumn("boxart_back_source_url").getString(),
      .metadata = GameMetadata::parse(query.getColumn("metadata_json").getString()),
      .metadataOverrides = MetadataOverrides::parse(query.getColumn("metadata_overrides_json").getString()),
      .normalizedTitle = query.getColumn("normalized_title").getString(),
      .discSetId = query.getColumn("disc_set_id").isNull() ? std::nullopt
                                                           : std::optional(query.getColumn("disc_set_id").getInt()),
      .discSetUserSet = query.getColumn("disc_set_user_set").getInt() != 0,
      .variantGroupId = query.getColumn("variant_group_id").isNull()
                            ? std::nullopt
                            : std::optional(query.getColumn("variant_group_id").getInt()),
      .variantGroupUserSet = query.getColumn("variant_group_user_set").getInt() != 0,
      .artFetchedAt = query.getColumn("art_fetched_at").isNull()
                          ? std::nullopt
                          : std::optional(static_cast<uint64_t>(query.getColumn("art_fetched_at").getInt64())),
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
      .m_discSetId = query.getColumn("disc_set_id").isNull() ? std::nullopt
                                                             : std::optional(query.getColumn("disc_set_id").getInt()),
      .m_discNumber = query.getColumn("disc_number").getInt(),
      .m_discNumberUserSet = query.getColumn("disc_number_user_set").getInt() != 0,
      .m_regions = strings::split(query.getColumn("region").getString(), ','),
      .m_gameId = query.getColumn("game_id").getInt(),
      .m_contentDirectoryId = query.getColumn("content_directory_id").getInt(),
      .m_missingSince = query.getColumn("missing_since").getInt64(),
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
                    "disc_number INTEGER NOT NULL DEFAULT 0,"
                    "disc_number_user_set INTEGER NOT NULL DEFAULT 0,"
                    "region TEXT NOT NULL DEFAULT '',"
                    "game_id INTEGER NOT NULL DEFAULT 0,"
                    "disc_set_id INTEGER,"
                    "content_directory_id INTEGER NOT NULL DEFAULT -1,"
                    "missing_since INTEGER NOT NULL DEFAULT 0,"
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

         m_db->exec("CREATE TABLE IF NOT EXISTS entries("
                    "id INTEGER PRIMARY KEY,"
                    "display_name TEXT NOT NULL,"
                    "content_hash TEXT NOT NULL,"
                    "platform_id INTEGER NOT NULL,"
                    "active_save_slot INTEGER NOT NULL DEFAULT 1, "
                    "hidden INTEGER NOT NULL DEFAULT 0, "
                    "favorite INTEGER NOT NULL DEFAULT 0, "
                    "rating INTEGER NOT NULL DEFAULT 0, "
                    "icon_1x1_source_url TEXT, "
                    "boxart_front_source_url TEXT, "
                    "boxart_back_source_url TEXT, "
                    "normalized_title TEXT NOT NULL DEFAULT '', "
                    "metadata_json TEXT, "
                    "metadata_overrides_json TEXT, "
                    "variant_group_id INTEGER, "
                    "disc_set_id INTEGER, "
                    "disc_set_user_set INTEGER NOT NULL DEFAULT 0, "
                    "variant_group_user_set INTEGER NOT NULL DEFAULT 0, "
                    // When art was last looked up for this entry. NULL means never
                    // tried, which is what makes the sweep resumable
                    "art_fetched_at INTEGER, "
                    "name_user_set INTEGER NOT NULL DEFAULT 0, "
                    "created_at INTEGER NOT NULL);");

         // COLLATE NOCASE on the name is what stops "sci-fi" and "Sci-Fi" becoming two
         // tags. Only the user writes these, so one spelling per idea is achievable
         // here in a way it is not for scraped genres
         m_db->exec("CREATE TABLE IF NOT EXISTS tags("
                    "id INTEGER PRIMARY KEY,"
                    "name TEXT UNIQUE NOT NULL COLLATE NOCASE, "
                    "color TEXT, "
                    "created_at INTEGER NOT NULL);");

         m_db->exec("CREATE TABLE IF NOT EXISTS entry_tags("
                    "entry_id INTEGER NOT NULL,"
                    "tag_id INTEGER NOT NULL,"
                    "created_at INTEGER NOT NULL,"
                    "UNIQUE(entry_id, tag_id));");

         m_db->exec("CREATE INDEX IF NOT EXISTS entriesNormalizedTitleIdx "
                    "ON entries(platform_id, normalized_title);");

         // A multi-disc game. The discs are content files pointing here; the entry points
         // here too, so one game is one row in the library however many discs it has
         m_db->exec("CREATE TABLE IF NOT EXISTS disc_sets("
                    "id INTEGER PRIMARY KEY,"
                    "title TEXT NOT NULL,"
                    "title_user_set INTEGER NOT NULL DEFAULT 0,"
                    // The playlist we generate and own, empty when the set has only one disc.
                    // but never rewrite
                    "disc_count INTEGER NOT NULL DEFAULT 0,"
                    "created_at INTEGER NOT NULL);");

         // Which disc a save slot was last on, so resuming picks up where it left off.
         // Per slot because two playthroughs genuinely sit on different discs
         m_db->exec("CREATE TABLE IF NOT EXISTS entry_disc_state("
                    "entry_id INTEGER NOT NULL,"
                    "save_slot INTEGER NOT NULL,"
                    "disc_number INTEGER NOT NULL,"
                    "UNIQUE (entry_id, save_slot));");

         m_db->exec("CREATE TABLE IF NOT EXISTS variant_groups("
                    "id INTEGER PRIMARY KEY,"
                    "title TEXT NOT NULL,"
                    "title_user_set INTEGER NOT NULL DEFAULT 0, "
                    "primary_entry_id INTEGER, "
                    "primary_user_set INTEGER NOT NULL DEFAULT 0, "
                    "auto_launch_primary INTEGER NOT NULL DEFAULT 0, "
                    "created_at INTEGER NOT NULL);");

         m_db->exec("CREATE TABLE IF NOT EXISTS run_configurations("
                    "id INTEGER PRIMARY KEY,"
                    "type TEXT NOT NULL,"
                    "content_hash TEXT NOT NULL,"
                    "content_file_id INTEGER NOT NULL,"
                    "patch_id INTEGER,"
                    "disc_set_id INTEGER,"
                    "created_at INTEGER NOT NULL,"
                    "UNIQUE (type, content_file_id, patch_id),"
                    "UNIQUE (type, content_file_id));");

         // A set launches through one way in, whichever disc is anchoring it
         m_db->exec("CREATE UNIQUE INDEX IF NOT EXISTS runConfigDiscSetIdx ON "
                    "run_configurations(disc_set_id) WHERE disc_set_id IS NOT NULL;");

         m_db->exec("CREATE TABLE IF NOT EXISTS content_directories("
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
                    "entries(content_hash);");
         m_db->exec("CREATE INDEX IF NOT EXISTS runConfigContentHashIdx ON "
                    "run_configurations(content_hash);");
         m_db->exec("CREATE INDEX IF NOT EXISTS contentFileContentHashIdx ON "
                    "content_files(content_hash);");

         // Partial because most entries belong to no variant group, so the index stays
         // small no matter how large the library gets
         m_db->exec("CREATE INDEX IF NOT EXISTS entriesVariantGroupIdx ON "
                    "entries(variant_group_id) WHERE variant_group_id IS NOT NULL;");

         m_db->exec("CREATE INDEX IF NOT EXISTS entriesDiscSetIdx ON "
                    "entries(disc_set_id) WHERE disc_set_id IS NOT NULL;");

         m_db->exec("CREATE INDEX IF NOT EXISTS contentFileDiscSetIdx ON "
                    "content_files(disc_set_id) WHERE disc_set_id IS NOT NULL;");

         // The UNIQUE(entry_id, tag_id) index serves the per-entry direction; this one
         // is what makes "how many entries use this tag" and deleting a tag everywhere
         // cheap
         m_db->exec("CREATE INDEX IF NOT EXISTS entryTagTagIdx ON entry_tags(tag_id);");

         // A file that got past the extension gate and could not be catalogued. Per file,
         // because the answer to "where did my game go" is a path
         m_db->exec("CREATE TABLE IF NOT EXISTS scan_drops("
                    "id INTEGER PRIMARY KEY,"
                    "file_path TEXT NOT NULL,"
                    "archive_path TEXT NOT NULL DEFAULT '',"
                    "extension TEXT NOT NULL,"
                    "file_size INTEGER NOT NULL DEFAULT 0,"
                    "outcome INTEGER NOT NULL,"
                    "identified_as TEXT NOT NULL DEFAULT '',"
                    "first_seen_at INTEGER NOT NULL,"
                    "last_seen_at INTEGER NOT NULL,"
                    "UNIQUE (file_path, archive_path));");

         // A count and never a path, so a folder of three thousand save files is one row
         m_db->exec("CREATE TABLE IF NOT EXISTS unrecognized_extensions("
                    "extension TEXT PRIMARY KEY,"
                    "count INTEGER NOT NULL DEFAULT 0,"
                    "last_seen_at INTEGER NOT NULL);");
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
  //
  // TODO
  // These run on every startup rather than once per version, so removing a
  // column from the schema above means removing its line here in the same
  // change — otherwise the next startup adds it straight back as NULL
  ensureColumnExists("content_files", "content_directory_id", "INTEGER NOT NULL DEFAULT -1");
  ensureColumnExists("content_files", "disc_number_user_set", "INTEGER NOT NULL DEFAULT 0");
  ensureColumnExists("content_files", "region", "TEXT NOT NULL DEFAULT ''");
  ensureColumnExists("content_files", "game_id", "INTEGER NOT NULL DEFAULT 0");
  ensureColumnExists("run_configurations", "disc_set_id", "INTEGER");
  ensureColumnExists("folders", "type", "INTEGER NOT NULL DEFAULT 0");
  ensureColumnExists("folders", "filter_json", "TEXT");
  ensureColumnExists("folders", "color", "TEXT");
  ensureColumnExists("folders", "sort_role", "TEXT");
  ensureColumnExists("folders", "sort_ascending", "INTEGER NOT NULL DEFAULT 1");
  ensureColumnExists("folders", "parent_id", "INTEGER NOT NULL DEFAULT -1");
  ensureColumnExists("folders", "position", "INTEGER NOT NULL DEFAULT 0");
  ensureColumnExists("entries", "name_user_set", "INTEGER NOT NULL DEFAULT 0");
  ensureColumnExists("entries", "rating", "INTEGER NOT NULL DEFAULT 0");

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
    if (strings::startsWith(onDiskPath, dir.path) && static_cast<int>(dir.path.length()) > bestLen) {
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
    query.bind(":createdAt", nowMs());
    query.exec();

    folder.id = static_cast<int>(m_db->getLastInsertRowid());
    EventDispatcher::instance().publish(FolderChangedEvent{.folderId = folder.id});
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

bool SqliteUserLibraryRepository::create(FolderEntry &folderEntry) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "INSERT INTO folder_entries(folder_id, entry_id, "
                                   "created_at) VALUES(:folderId, :entryId, "
                                   ":createdAt);");
    query.bind(":folderId", folderEntry.folderId);
    query.bind(":entryId", folderEntry.entryId);
    query.bind(":createdAt", nowMs());
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

    EventDispatcher::instance().publish(FolderChangedEvent{.folderId = folderId});
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

    EventDispatcher::instance().publish(FolderChangedEvent{.folderId = folder.id});
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

bool SqliteUserLibraryRepository::deleteFolderEntry(FolderEntry &info) {
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
    SQLite::Statement query(*m_db, "UPDATE entries SET display_name = :displayName, "
                                   "active_save_slot = :activeSaveSlot, "
                                   "favorite = :favorite, rating = :rating, name_user_set = :nameUserSet "
                                   "WHERE id = :id;");
    query.bind(":id", entry.id);
    query.bind(":displayName", entry.displayName);
    query.bind(":activeSaveSlot", entry.activeSaveSlot);
    query.bind(":favorite", entry.favorite ? 1 : 0);
    query.bind(":rating", entry.rating);
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

bool SqliteUserLibraryRepository::setEntryHidden(const int entryId, const bool hidden) {
  {
    std::lock_guard lock(m_mutex);
    try {
      SQLite::Statement query(*m_db, "UPDATE entries SET hidden = :hidden WHERE id = :id;");
      query.bind(":id", entryId);
      query.bind(":hidden", hidden ? 1 : 0);

      if (query.exec() == 0) {
        spdlog::error("Failed to set hidden on entry with ID {}: no rows affected", entryId);
        return false;
      }
    } catch (const std::exception &e) {
      spdlog::error("Failed to set hidden on entry with ID {}: {}", entryId, e.what());
      return false;
    }
  }

  EventDispatcher::instance().publish(EntryUpdatedEvent{.entryId = entryId});
  return true;
}

bool SqliteUserLibraryRepository::updateEntryMetadata(const Entry &entry) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "UPDATE entries SET display_name = :displayName, "
                                   "name_user_set = :nameUserSet, "
                                   "normalized_title = :normalizedTitle, "
                                   "icon_1x1_source_url = :icon1x1, "
                                   "boxart_front_source_url = :boxartFront, "
                                   "boxart_back_source_url = :boxartBack WHERE id = :id;");
    query.bind(":id", entry.id);
    query.bind(":displayName", entry.displayName);
    query.bind(":nameUserSet", entry.nameUserSet ? 1 : 0);
    query.bind(":normalizedTitle", entry.normalizedTitle);
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

bool SqliteUserLibraryRepository::applyEntryMetadata(const int entryId, const GameMetadata &incoming,
                                                     const std::set<std::string> &changedFields,
                                                     const bool isUserEdit) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement readQuery(*m_db, "SELECT metadata_json, metadata_overrides_json FROM entries WHERE id = :id;");
    readQuery.bind(":id", entryId);

    if (!readQuery.executeStep()) {
      return false;
    }

    const auto storedMetadata = readQuery.getColumn(0).getString();
    const auto storedOverrides = readQuery.getColumn(1).getString();

    auto merged = GameMetadata::parse(storedMetadata);
    auto overrides = MetadataOverrides::parse(storedOverrides);

    const auto shouldWrite = [&](const char *field) {
      return changedFields.count(field) > 0 && (isUserEdit || !overrides.isUserSet(field));
    };

    const auto take = [&](const char *field, auto &destination, const auto &source) {
      if (shouldWrite(field)) {
        destination = source;

        if (isUserEdit) {
          overrides.markUserSet(field);
        }
      }
    };

    take(metadata_fields::DESCRIPTION, merged.description, incoming.description);
    take(metadata_fields::DEVELOPER, merged.developer, incoming.developer);
    take(metadata_fields::PUBLISHER, merged.publisher, incoming.publisher);
    take(metadata_fields::RELEASE_YEAR, merged.releaseYear, incoming.releaseYear);
    take(metadata_fields::RELEASE_DATE, merged.releaseDate, incoming.releaseDate);
    take(metadata_fields::PLAYERS, merged.players, incoming.players);
    take(metadata_fields::DISC_COUNT, merged.discCount, incoming.discCount);
    take(metadata_fields::REVISION, merged.revision, incoming.revision);
    take(metadata_fields::GENRES, merged.genres, incoming.genres);
    take(metadata_fields::REGIONS, merged.regions, incoming.regions);
    take(metadata_fields::LANGUAGES, merged.languages, incoming.languages);
    take(metadata_fields::FLAGS, merged.flags, incoming.flags);

    const auto mergedJson = merged.toJson();
    const auto overridesJson = overrides.toJson();

    // TODO
    // Every write announces a change that wakes the groupers, so laying the same values over
    // the same document has to say nothing at all
    if (mergedJson == storedMetadata && overridesJson == storedOverrides) {
      return true;
    }

    SQLite::Statement writeQuery(*m_db, "UPDATE entries SET metadata_json = :metadata, "
                                        "metadata_overrides_json = :overrides WHERE id = :id;");
    writeQuery.bind(":id", entryId);
    writeQuery.bind(":metadata", mergedJson);
    writeQuery.bind(":overrides", overridesJson);

    if (writeQuery.exec() == 0) {
      return false;
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to apply entry metadata for ID {}: {}", entryId, e.what());
    return false;
  }

  EventDispatcher::instance().publish(EntryUpdatedEvent{.entryId = entryId});
  return true;
}

bool SqliteUserLibraryRepository::markArtFetched(const int entryId, const uint64_t whenMillis) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "UPDATE entries SET art_fetched_at = :when WHERE id = :id;");
    query.bind(":id", entryId);
    query.bind(":when", static_cast<int64_t>(whenMillis));

    return query.exec() != 0;
  } catch (const std::exception &e) {
    spdlog::error("Failed to mark art fetched for entry {}: {}", entryId, e.what());
    return false;
  }
}

std::vector<int> SqliteUserLibraryRepository::getEntryIdsMissingArt(const int limit) {
  std::lock_guard lock(m_mutex);
  std::vector<int> ids;
  try {
    // Looking up art for a game whose files are gone spends a request on something nobody can play
    SQLite::Statement query(*m_db, "SELECT id FROM entries e WHERE art_fetched_at IS NULL AND hidden = 0 "
                                   "AND EXISTS (SELECT 1 FROM content_files cf WHERE cf.content_hash = e.content_hash "
                                   "            AND cf.missing_since = 0) "
                                   "ORDER BY id LIMIT :limit;");
    query.bind(":limit", limit);

    while (query.executeStep()) {
      ids.push_back(query.getColumn("id").getInt());
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get entries missing art: {}", e.what());
  }

  return ids;
}

//****************
// variant groups
//****************

namespace {
VariantGroup deserializeVariantGroup(const SQLite::Statement &query) {
  return VariantGroup{
      .id = query.getColumn("id").getInt(),
      .title = query.getColumn("title").getString(),
      .titleUserSet = query.getColumn("title_user_set").getInt() != 0,
      .primaryEntryId = query.getColumn("primary_entry_id").isNull()
                            ? std::nullopt
                            : std::optional(query.getColumn("primary_entry_id").getInt()),
      .primaryUserSet = query.getColumn("primary_user_set").getInt() != 0,
      .autoLaunchPrimary = query.getColumn("auto_launch_primary").getInt() != 0,
      .createdAt = static_cast<uint64_t>(query.getColumn("created_at").getInt64()),
  };
}
} // namespace

namespace {
DiscSet deserializeDiscSet(const SQLite::Statement &query) {
  return DiscSet{
      .id = query.getColumn("id").getInt(),
      .title = query.getColumn("title").getString(),
      .titleUserSet = query.getColumn("title_user_set").getInt() != 0,
      .discCount = query.getColumn("disc_count").getInt(),
      .createdAt = static_cast<uint64_t>(query.getColumn("created_at").getInt64()),
  };
}
} // namespace

bool SqliteUserLibraryRepository::createDiscSet(DiscSet &set) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "INSERT INTO disc_sets(title, title_user_set, disc_count, created_at) "
                                   "VALUES(:title, :titleUserSet, :discCount, :createdAt);");
    query.bind(":title", set.title);
    query.bind(":titleUserSet", set.titleUserSet ? 1 : 0);
    query.bind(":discCount", set.discCount);
    query.bind(":createdAt", nowMs());
    query.exec();
    set.id = static_cast<int>(m_db->getLastInsertRowid());
    set.createdAt = static_cast<uint64_t>(nowMs());
  } catch (const std::exception &e) {
    spdlog::error("Failed to create disc set: {}", e.what());
    return false;
  }

  return true;
}

bool SqliteUserLibraryRepository::updateDiscSet(const DiscSet &set) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "UPDATE disc_sets SET title = :title, "
                                   "title_user_set = :titleUserSet, disc_count = :discCount "
                                   "WHERE id = :id;");
    query.bind(":id", set.id);
    query.bind(":title", set.title);
    query.bind(":titleUserSet", set.titleUserSet ? 1 : 0);
    query.bind(":discCount", set.discCount);

    if (query.exec() == 0) {
      return false;
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to update disc set {}: {}", set.id, e.what());
    return false;
  }

  return true;
}

bool SqliteUserLibraryRepository::recordScanDrop(const ScanDrop &drop) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement existing(*m_db, "SELECT id FROM scan_drops WHERE file_path = :filePath "
                                      "AND archive_path = :archivePath;");
    existing.bind(":filePath", drop.filePath);
    existing.bind(":archivePath", drop.archivePath);
    const auto isNew = !existing.executeStep();

    SQLite::Statement query(*m_db, "INSERT INTO scan_drops "
                                   "(file_path, archive_path, extension, file_size, outcome, identified_as, "
                                   "first_seen_at, last_seen_at) VALUES (:filePath, :archivePath, :extension, "
                                   ":fileSize, :outcome, :identifiedAs, :now, :now) "
                                   "ON CONFLICT(file_path, archive_path) DO UPDATE SET "
                                   "extension = excluded.extension, file_size = excluded.file_size, "
                                   "outcome = excluded.outcome, identified_as = excluded.identified_as, "
                                   "last_seen_at = excluded.last_seen_at;");
    query.bind(":filePath", drop.filePath);
    query.bind(":archivePath", drop.archivePath);
    query.bind(":extension", drop.extension);
    query.bind(":fileSize", static_cast<int64_t>(drop.fileSizeBytes));
    query.bind(":outcome", static_cast<int>(drop.outcome));
    query.bind(":identifiedAs", drop.identifiedAs);
    query.bind(":now", nowMs());
    query.exec();

    return isNew;
  } catch (const std::exception &e) {
    spdlog::error("Failed to record scan drop for {}: {}", drop.filePath, e.what());
    return false;
  }
}

bool SqliteUserLibraryRepository::clearScanDrop(const std::string &filePath, const std::string &archivePath) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement query(*m_db, "DELETE FROM scan_drops WHERE file_path = :filePath "
                                   "AND archive_path = :archivePath;");
    query.bind(":filePath", filePath);
    query.bind(":archivePath", archivePath);
    return query.exec() > 0;
  } catch (const std::exception &e) {
    spdlog::error("Failed to clear scan drop for {}: {}", filePath, e.what());
    return false;
  }
}

std::vector<ScanDrop> SqliteUserLibraryRepository::getScanDrops() {
  std::lock_guard lock(m_mutex);
  std::vector<ScanDrop> drops;

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM scan_drops ORDER BY first_seen_at, id;");

    while (query.executeStep()) {
      drops.push_back(ScanDrop{
          .id = query.getColumn("id").getInt(),
          .filePath = query.getColumn("file_path").getString(),
          .archivePath = query.getColumn("archive_path").getString(),
          .extension = query.getColumn("extension").getString(),
          .fileSizeBytes = static_cast<size_t>(query.getColumn("file_size").getInt64()),
          .outcome = static_cast<IdentifyOutcome>(query.getColumn("outcome").getInt()),
          .identifiedAs = query.getColumn("identified_as").getString(),
          .firstSeenAt = query.getColumn("first_seen_at").getInt64(),
          .lastSeenAt = query.getColumn("last_seen_at").getInt64(),
      });
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get scan drops: {}", e.what());
  }

  return drops;
}

void SqliteUserLibraryRepository::countUnrecognizedExtension(const std::string &extension) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement query(*m_db, "INSERT INTO unrecognized_extensions (extension, count, last_seen_at) "
                                   "VALUES (:extension, 1, :now) "
                                   "ON CONFLICT(extension) DO UPDATE SET "
                                   "count = count + 1, last_seen_at = excluded.last_seen_at;");
    query.bind(":extension", extension);
    query.bind(":now", nowMs());
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to count unrecognized extension {}: {}", extension, e.what());
  }
}

std::vector<UnrecognizedExtension> SqliteUserLibraryRepository::getUnrecognizedExtensions() {
  std::lock_guard lock(m_mutex);
  std::vector<UnrecognizedExtension> extensions;

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM unrecognized_extensions ORDER BY count DESC, extension;");

    while (query.executeStep()) {
      extensions.push_back(UnrecognizedExtension{
          .extension = query.getColumn("extension").getString(),
          .count = query.getColumn("count").getInt(),
          .lastSeenAt = query.getColumn("last_seen_at").getInt64(),
      });
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get unrecognized extensions: {}", e.what());
  }

  return extensions;
}

bool SqliteUserLibraryRepository::deleteDiscSet(const int setId) {
  // TODO
  // Before the set goes, while its way in can still be found by set id. A row left behind keeps
  // getRunConfigurations non-empty, which is what decides whether a game can be hidden
  deleteRunConfigurationsForDiscSet(setId);

  std::lock_guard lock(m_mutex);
  try {
    // The discs and the entry outlive the set; only the grouping goes
    SQLite::Statement clearFiles(*m_db, "UPDATE content_files SET disc_set_id = NULL WHERE disc_set_id = :id;");
    clearFiles.bind(":id", setId);
    clearFiles.exec();

    SQLite::Statement clearEntries(*m_db, "UPDATE entries SET disc_set_id = NULL WHERE disc_set_id = :id;");
    clearEntries.bind(":id", setId);
    clearEntries.exec();

    SQLite::Statement query(*m_db, "DELETE FROM disc_sets WHERE id = :id;");
    query.bind(":id", setId);

    if (query.exec() == 0) {
      return false;
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to delete disc set {}: {}", setId, e.what());
    return false;
  }

  return true;
}

std::optional<DiscSet> SqliteUserLibraryRepository::getDiscSet(const int setId) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM disc_sets WHERE id = :id LIMIT 1;");
    query.bind(":id", setId);

    if (query.executeStep()) {
      return deserializeDiscSet(query);
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get disc set {}: {}", setId, e.what());
  }

  return std::nullopt;
}

std::vector<DiscSet> SqliteUserLibraryRepository::getDiscSets() {
  std::lock_guard lock(m_mutex);
  std::vector<DiscSet> sets;

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM disc_sets ORDER BY id;");

    while (query.executeStep()) {
      sets.emplace_back(deserializeDiscSet(query));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get disc sets: {}", e.what());
  }

  return sets;
}

std::optional<DiscSet> SqliteUserLibraryRepository::getDiscSetForContentFile(const int contentFileId) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "SELECT s.* FROM disc_sets s JOIN content_files f ON "
                                   "f.disc_set_id = s.id WHERE f.id = :id LIMIT 1;");
    query.bind(":id", contentFileId);

    if (query.executeStep()) {
      return deserializeDiscSet(query);
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get disc set for content file {}: {}", contentFileId, e.what());
  }

  return std::nullopt;
}

std::vector<ContentFile> SqliteUserLibraryRepository::getDiscsInSet(const int setId) {
  return discsInSet(setId, false);
}

std::vector<ContentFile> SqliteUserLibraryRepository::getPresentDiscsInSet(const int setId) {
  return discsInSet(setId, true);
}

std::vector<ContentFile> SqliteUserLibraryRepository::discsInSet(const int setId, const bool presentOnly) {
  std::lock_guard lock(m_mutex);
  std::vector<ContentFile> discs;
  try {
    SQLite::Statement query(*m_db, presentOnly ? "SELECT * FROM content_files WHERE disc_set_id = :id "
                                                 "AND missing_since = 0 ORDER BY disc_number, id;"
                                               : "SELECT * FROM content_files WHERE disc_set_id = :id "
                                                 "ORDER BY disc_number, id;");
    query.bind(":id", setId);

    // TODO
    // One disc dumped twice (a cue and a chd of the same bytes) is two rows sharing a hash,
    // and every caller counts these as discs. An unhashed row matches nothing rather than
    // matching every other unhashed one
    std::unordered_set<std::string> seenHashes;

    while (query.executeStep()) {
      auto disc = deserializeContentFile(query);

      if (!disc.m_contentHash.empty() && !seenHashes.insert(disc.m_contentHash).second) {
        continue;
      }

      discs.emplace_back(std::move(disc));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get discs in set {}: {}", setId, e.what());
  }

  return discs;
}

std::vector<Entry> SqliteUserLibraryRepository::getEntriesInDiscSet(const int setId) {
  std::lock_guard lock(m_mutex);
  std::vector<Entry> entries;

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM entries WHERE disc_set_id = :setId;");
    query.bind(":setId", setId);

    while (query.executeStep()) {
      auto entry = deserializeEntry(query);
      populateEntrySource(entry);
      entries.emplace_back(std::move(entry));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get entries in disc set {}: {}", setId, e.what());
  }

  return entries;
}

bool SqliteUserLibraryRepository::setContentFileDiscSet(const int contentFileId, const std::optional<int> setId) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "UPDATE content_files SET disc_set_id = :setId WHERE id = :id;");
    query.bind(":id", contentFileId);

    if (setId.has_value()) {
      query.bind(":setId", *setId);
    } else {
      query.bind(":setId");
    }

    if (query.exec() == 0) {
      return false;
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to set disc set for content file {}: {}", contentFileId, e.what());
    return false;
  }

  return true;
}

bool SqliteUserLibraryRepository::setEntryDiscSet(const int entryId, const std::optional<int> setId,
                                                  const bool isUserChoice) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "UPDATE entries SET disc_set_id = :setId, "
                                   "disc_set_user_set = :userSet WHERE id = :id;");
    query.bind(":id", entryId);
    query.bind(":userSet", isUserChoice ? 1 : 0);

    if (setId.has_value()) {
      query.bind(":setId", *setId);
    } else {
      query.bind(":setId");
    }

    if (query.exec() == 0) {
      return false;
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to set disc set for entry {}: {}", entryId, e.what());
    return false;
  }

  EventDispatcher::instance().publish(EntryUpdatedEvent{.entryId = entryId});
  return true;
}

std::optional<int> SqliteUserLibraryRepository::getLastDisc(const int entryId, const int saveSlot) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "SELECT disc_number FROM entry_disc_state WHERE "
                                   "entry_id = :entryId AND save_slot = :saveSlot LIMIT 1;");
    query.bind(":entryId", entryId);
    query.bind(":saveSlot", saveSlot);

    if (query.executeStep()) {
      return query.getColumn("disc_number").getInt();
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get last disc for entry {}: {}", entryId, e.what());
  }

  return std::nullopt;
}

bool SqliteUserLibraryRepository::setLastDisc(const int entryId, const int saveSlot, const int discNumber) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "INSERT INTO entry_disc_state(entry_id, save_slot, disc_number) "
                                   "VALUES(:entryId, :saveSlot, :discNumber) ON CONFLICT(entry_id, "
                                   "save_slot) DO UPDATE SET disc_number = :discNumber;");
    query.bind(":entryId", entryId);
    query.bind(":saveSlot", saveSlot);
    query.bind(":discNumber", discNumber);
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to set last disc for entry {}: {}", entryId, e.what());
    return false;
  }

  return true;
}

bool SqliteUserLibraryRepository::createVariantGroup(VariantGroup &group) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "INSERT INTO variant_groups(title, title_user_set, primary_entry_id, "
                                   "primary_user_set, auto_launch_primary, created_at) VALUES"
                                   "(:title, :titleUserSet, :primaryEntryId, :primaryUserSet, "
                                   ":autoLaunchPrimary, :createdAt);");
    query.bind(":title", group.title);
    query.bind(":titleUserSet", group.titleUserSet ? 1 : 0);

    if (group.primaryEntryId.has_value()) {
      query.bind(":primaryEntryId", *group.primaryEntryId);
    } else {
      query.bind(":primaryEntryId");
    }

    query.bind(":primaryUserSet", group.primaryUserSet ? 1 : 0);
    query.bind(":autoLaunchPrimary", group.autoLaunchPrimary ? 1 : 0);
    query.bind(":createdAt", nowMs());
    query.exec();

    group.id = static_cast<int>(m_db->getLastInsertRowid());
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to create variant group: {}", e.what());
    return false;
  }
}

bool SqliteUserLibraryRepository::updateVariantGroup(const VariantGroup &group) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "UPDATE variant_groups SET title = :title, "
                                   "title_user_set = :titleUserSet, primary_entry_id = :primaryEntryId, "
                                   "primary_user_set = :primaryUserSet, "
                                   "auto_launch_primary = :autoLaunchPrimary WHERE id = :id;");
    query.bind(":id", group.id);
    query.bind(":title", group.title);
    query.bind(":titleUserSet", group.titleUserSet ? 1 : 0);

    if (group.primaryEntryId.has_value()) {
      query.bind(":primaryEntryId", *group.primaryEntryId);
    } else {
      query.bind(":primaryEntryId");
    }

    query.bind(":primaryUserSet", group.primaryUserSet ? 1 : 0);
    query.bind(":autoLaunchPrimary", group.autoLaunchPrimary ? 1 : 0);

    return query.exec() != 0;
  } catch (const std::exception &e) {
    spdlog::error("Failed to update variant group {}: {}", group.id, e.what());
    return false;
  }
}

bool SqliteUserLibraryRepository::deleteVariantGroup(const int groupId) {
  std::lock_guard lock(m_mutex);
  try {
    // There are no foreign keys, so the members are cleared here rather than by
    // a cascade
    SQLite::Statement clearQuery(*m_db,
                                 "UPDATE entries SET variant_group_id = NULL WHERE variant_group_id = :groupId;");
    clearQuery.bind(":groupId", groupId);
    clearQuery.exec();

    SQLite::Statement query(*m_db, "DELETE FROM variant_groups WHERE id = :id;");
    query.bind(":id", groupId);

    return query.exec() != 0;
  } catch (const std::exception &e) {
    spdlog::error("Failed to delete variant group {}: {}", groupId, e.what());
    return false;
  }
}

std::vector<VariantGroup> SqliteUserLibraryRepository::getVariantGroups() {
  std::lock_guard lock(m_mutex);
  std::vector<VariantGroup> groups;
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM variant_groups ORDER BY title;");

    while (query.executeStep()) {
      groups.emplace_back(deserializeVariantGroup(query));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get variant groups: {}", e.what());
  }

  return groups;
}

std::optional<VariantGroup> SqliteUserLibraryRepository::getVariantGroup(const int groupId) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM variant_groups WHERE id = :id;");
    query.bind(":id", groupId);

    if (query.executeStep()) {
      return deserializeVariantGroup(query);
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get variant group {}: {}", groupId, e.what());
  }

  return std::nullopt;
}

bool SqliteUserLibraryRepository::setEntryVariantGroup(const int entryId, const std::optional<int> groupId,
                                                       const bool isUserChoice) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "UPDATE entries SET variant_group_id = :groupId, "
                                   "variant_group_user_set = :userSet WHERE id = :id;");
    query.bind(":id", entryId);
    query.bind(":userSet", isUserChoice ? 1 : 0);

    if (groupId.has_value()) {
      query.bind(":groupId", *groupId);
    } else {
      query.bind(":groupId");
    }

    if (query.exec() == 0) {
      return false;
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to set variant group for entry {}: {}", entryId, e.what());
    return false;
  }

  EventDispatcher::instance().publish(EntryUpdatedEvent{.entryId = entryId});
  return true;
}

std::vector<int> SqliteUserLibraryRepository::getCandidateEntryIds(const GameIdentity &identity) {
  std::lock_guard lock(m_mutex);
  std::vector<int> ids;

  if (identity.isEmpty()) {
    return ids;
  }

  try {
    // TODO
    // Two statements rather than one OR, because the planner will not reliably split an OR
    // into two index seeks and each of these is obviously indexed on its own
    if (identity.gameId != 0) {
      SQLite::Statement query(*m_db, "SELECT DISTINCT e.id FROM entries e "
                                     "JOIN content_files cf ON cf.content_hash = e.content_hash "
                                     "WHERE e.platform_id = :platformId AND cf.game_id = :gameId "
                                     "AND cf.game_id != 0;");
      query.bind(":platformId", identity.platformId);
      query.bind(":gameId", identity.gameId);

      while (query.executeStep()) {
        ids.push_back(query.getColumn(0).getInt());
      }
    }

    if (!identity.title.empty()) {
      SQLite::Statement query(*m_db, "SELECT id FROM entries WHERE platform_id = :platformId "
                                     "AND normalized_title = :normalizedTitle;");
      query.bind(":platformId", identity.platformId);
      query.bind(":normalizedTitle", identity.title);

      while (query.executeStep()) {
        ids.push_back(query.getColumn("id").getInt());
      }
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get candidate entries for title {}: {}", identity.title, e.what());
  }

  // Ascending and without repeats, so a caller that stops at the first peer stops at the same
  // one every time
  std::ranges::sort(ids);
  ids.erase(std::ranges::unique(ids).begin(), ids.end());

  return ids;
}

std::vector<Entry> SqliteUserLibraryRepository::getEntriesInVariantGroup(const int groupId) {
  std::lock_guard lock(m_mutex);
  std::vector<Entry> entries;
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM entries WHERE variant_group_id = :groupId;");
    query.bind(":groupId", groupId);

    while (query.executeStep()) {
      auto entry = deserializeEntry(query);
      populateEntrySource(entry);
      entries.emplace_back(std::move(entry));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get entries in variant group {}: {}", groupId, e.what());
  }

  return entries;
}

//****************
// tags
//****************

bool SqliteUserLibraryRepository::createTag(Tag &tag) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "INSERT INTO tags(name, color, created_at) VALUES"
                                   "(:name, :color, :createdAt) ON CONFLICT(name) DO NOTHING;");
    query.bind(":name", tag.name);
    query.bind(":color", tag.color);
    query.bind(":createdAt", nowMs());
    query.exec();

    // The name may already have been taken, in which case the caller gets the tag
    // that exists rather than a failure
    SQLite::Statement idQuery(*m_db, "SELECT id, name FROM tags WHERE name = :name;");
    idQuery.bind(":name", tag.name);

    if (!idQuery.executeStep()) {
      return false;
    }

    tag.id = idQuery.getColumn("id").getInt();
    tag.name = idQuery.getColumn("name").getString();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to create tag '{}': {}", tag.name, e.what());
    return false;
  }
}

bool SqliteUserLibraryRepository::renameTag(const int tagId, const std::string &name) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "UPDATE tags SET name = :name WHERE id = :id;");
    query.bind(":id", tagId);
    query.bind(":name", name);

    return query.exec() != 0;
  } catch (const std::exception &e) {
    spdlog::error("Failed to rename tag {}: {}", tagId, e.what());
    return false;
  }
}

bool SqliteUserLibraryRepository::mergeTags(const int sourceTagId, const int targetTagId) {
  if (sourceTagId == targetTagId) {
    return true;
  }

  std::lock_guard lock(m_mutex);
  try {
    SQLite::Transaction transaction(*m_db);

    // OR IGNORE absorbs the entries that carried both tags; without it the move
    // trips UNIQUE(entry_id, tag_id) and the merge fails
    SQLite::Statement moveQuery(*m_db, "UPDATE OR IGNORE entry_tags SET tag_id = :targetId WHERE tag_id = :sourceId;");
    moveQuery.bind(":targetId", targetTagId);
    moveQuery.bind(":sourceId", sourceTagId);
    moveQuery.exec();

    SQLite::Statement clearQuery(*m_db, "DELETE FROM entry_tags WHERE tag_id = :sourceId;");
    clearQuery.bind(":sourceId", sourceTagId);
    clearQuery.exec();

    SQLite::Statement deleteQuery(*m_db, "DELETE FROM tags WHERE id = :sourceId;");
    deleteQuery.bind(":sourceId", sourceTagId);
    deleteQuery.exec();

    transaction.commit();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to merge tag {} into {}: {}", sourceTagId, targetTagId, e.what());
    return false;
  }
}

bool SqliteUserLibraryRepository::deleteTag(const int tagId) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Transaction transaction(*m_db);

    SQLite::Statement clearQuery(*m_db, "DELETE FROM entry_tags WHERE tag_id = :id;");
    clearQuery.bind(":id", tagId);
    clearQuery.exec();

    SQLite::Statement query(*m_db, "DELETE FROM tags WHERE id = :id;");
    query.bind(":id", tagId);
    const auto removed = query.exec();

    transaction.commit();
    return removed != 0;
  } catch (const std::exception &e) {
    spdlog::error("Failed to delete tag {}: {}", tagId, e.what());
    return false;
  }
}

std::vector<Tag> SqliteUserLibraryRepository::getTags() {
  std::lock_guard lock(m_mutex);
  std::vector<Tag> tags;
  try {
    SQLite::Statement query(*m_db, "SELECT t.id, t.name, t.color, t.created_at, "
                                   "COUNT(et.entry_id) AS usage_count "
                                   "FROM tags t LEFT JOIN entry_tags et ON et.tag_id = t.id "
                                   "GROUP BY t.id ORDER BY t.name;");

    while (query.executeStep()) {
      tags.emplace_back(Tag{
          .id = query.getColumn("id").getInt(),
          .name = query.getColumn("name").getString(),
          .color = query.getColumn("color").getString(),
          .createdAt = static_cast<uint64_t>(query.getColumn("created_at").getInt64()),
          .usageCount = query.getColumn("usage_count").getInt(),
      });
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get tags: {}", e.what());
  }

  return tags;
}

bool SqliteUserLibraryRepository::setEntryTags(const int entryId, const std::vector<int> &tagIds) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Transaction transaction(*m_db);

    SQLite::Statement clearQuery(*m_db, "DELETE FROM entry_tags WHERE entry_id = :entryId;");
    clearQuery.bind(":entryId", entryId);
    clearQuery.exec();

    for (const auto tagId : tagIds) {
      SQLite::Statement query(*m_db, "INSERT OR IGNORE INTO entry_tags(entry_id, tag_id, created_at) "
                                     "VALUES(:entryId, :tagId, :createdAt);");
      query.bind(":entryId", entryId);
      query.bind(":tagId", tagId);
      query.bind(":createdAt", nowMs());
      query.exec();
    }

    transaction.commit();
  } catch (const std::exception &e) {
    spdlog::error("Failed to set tags for entry {}: {}", entryId, e.what());
    return false;
  }

  EventDispatcher::instance().publish(EntryUpdatedEvent{.entryId = entryId});
  return true;
}

bool SqliteUserLibraryRepository::deleteContentDirectory(int id) {
  std::lock_guard lock(m_mutex);
  std::string path;
  try {
    SQLite::Statement selectQuery(*m_db, "SELECT path FROM content_directories WHERE id = :id;");
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
    SQLite::Statement query(*m_db, "DELETE FROM content_directories WHERE id = :id;");
    query.bind(":id", id);
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to delete content directory with ID {}: {}", id, e.what());
    return false;
  }

  for (const auto &rom : getContentFiles()) {
    auto romPath = rom.m_inArchive ? rom.m_archivePathName : rom.m_filePath;

    if (strings::startsWith(romPath, path)) {
      auto found = false;
      for (const auto &contentPath : getContentDirectories()) {
        if (strings::startsWith(romPath, contentPath.path)) {
          found = true;
          break;
        }
      }

      if (!found) {
        markContentFileMissing(rom.m_id);
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
                                   "content_hash, content_type, disc_number, disc_number_user_set, "
                                   "region, game_id, content_directory_id, created_at) "
                                   "VALUES (:filePath, :fileSize, :fileMd5, :fileCrc32, :inArchive, "
                                   ":archiveFilePath, :platformId, :contentHash, :contentType, "
                                   ":discNumber, :discNumberUserSet, :region, :gameId, "
                                   ":contentDirectoryId, :createdAt);");
    query.bind(":filePath", romFile.m_filePath);
    query.bind(":discNumber", romFile.m_discNumber);
    query.bind(":discNumberUserSet", romFile.m_discNumberUserSet ? 1 : 0);
    query.bind(":region", strings::join(romFile.m_regions, ","));
    query.bind(":gameId", romFile.m_gameId);
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

    return deserializeContentFile(query);
  } catch (const std::exception &e) {
    spdlog::error("Failed to get rom file with path {}: {}", filePath, e.what());
    return std::nullopt;
  }
}

bool SqliteUserLibraryRepository::markContentFileMissing(const int id) {
  ContentFileMissingEvent event;

  {
    std::lock_guard lock(m_mutex);
    try {
      SQLite::Statement query(*m_db, "SELECT * FROM content_files WHERE id = :id;");
      query.bind(":id", id);

      if (!query.executeStep()) {
        return true;
      }

      const auto file = deserializeContentFile(query);

      if (file.m_missingSince != 0) {
        return true;
      }

      event = ContentFileMissingEvent{.id = id, .contentHash = file.m_contentHash, .discSetId = file.m_discSetId};

      SQLite::Statement mark(*m_db, "UPDATE content_files SET missing_since = :now WHERE id = :id;");
      mark.bind(":id", id);
      mark.bind(":now", nowMs());
      mark.exec();
    } catch (const std::exception &e) {
      spdlog::error("Failed to mark content file {} missing: {}", id, e.what());
      return false;
    }
  }

  EventDispatcher::instance().publish(event);
  return true;
}

bool SqliteUserLibraryRepository::reviveContentFile(const int id) {
  ContentFileRestoredEvent event;

  {
    std::lock_guard lock(m_mutex);
    try {
      SQLite::Statement query(*m_db, "SELECT * FROM content_files WHERE id = :id;");
      query.bind(":id", id);

      if (!query.executeStep()) {
        return true;
      }

      const auto file = deserializeContentFile(query);

      if (file.m_missingSince == 0) {
        return true;
      }

      event = ContentFileRestoredEvent{.id = id, .contentHash = file.m_contentHash, .discSetId = file.m_discSetId};

      SQLite::Statement clear(*m_db, "UPDATE content_files SET missing_since = 0 WHERE id = :id;");
      clear.bind(":id", id);
      clear.exec();
    } catch (const std::exception &e) {
      spdlog::error("Failed to revive content file {}: {}", id, e.what());
      return false;
    }
  }

  EventDispatcher::instance().publish(event);
  return true;
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

    // Every way in, whatever its type: a file that is gone cannot be launched through any of
    // them, and one left behind would keep a deleted entry looking playable
    SQLite::Statement deleteRunConfigsQuery(*m_db,
                                            "DELETE FROM run_configurations WHERE content_file_id = :contentFileId;");
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

std::vector<Entry> SqliteUserLibraryRepository::getEntries() {
  std::lock_guard lock(m_mutex);
  std::vector<Entry> entries;

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM entries ORDER BY display_name ASC;");

    while (query.executeStep()) {
      entries.emplace_back(deserializeEntry(query));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get entries: {}", e.what());
    return {};
  }

  // TODO
  // Three grouped queries rather than one per entry per join. At a few thousand
  // entries the per-entry form re-prepares thousands of statements, and adding tags
  // to it would have made that a third worse
  std::unordered_map<int, size_t> indexById;
  std::unordered_map<std::string, std::vector<size_t>> indexByHash;

  for (size_t i = 0; i < entries.size(); ++i) {
    indexById[entries[i].id] = i;
    indexByHash[entries[i].contentHash].push_back(i);
  }

  try {
    SQLite::Statement folderQuery(*m_db, "SELECT entry_id, folder_id FROM folder_entries;");

    while (folderQuery.executeStep()) {
      const auto found = indexById.find(folderQuery.getColumn("entry_id").getInt());

      if (found != indexById.end()) {
        entries[found->second].folderIds.push_back(folderQuery.getColumn("folder_id").getInt());
      }
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get folder IDs: {}", e.what());
  }

  try {
    SQLite::Statement tagQuery(*m_db, "SELECT entry_id, tag_id FROM entry_tags;");

    while (tagQuery.executeStep()) {
      const auto found = indexById.find(tagQuery.getColumn("entry_id").getInt());

      if (found != indexById.end()) {
        entries[found->second].tagIds.push_back(tagQuery.getColumn("tag_id").getInt());
      }
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get tag IDs: {}", e.what());
  }

  try {
    SQLite::Statement sourceQuery(*m_db, "SELECT content_hash, content_directory_id, file_path, in_archive, "
                                         "archive_file_path, missing_since FROM content_files;");

    while (sourceQuery.executeStep()) {
      const auto found = indexByHash.find(sourceQuery.getColumn("content_hash").getString());

      if (found == indexByHash.end()) {
        continue;
      }

      const auto directoryId = sourceQuery.getColumn("content_directory_id").getInt();
      const auto isReadable = sourceQuery.getColumn("missing_since").getInt64() == 0;
      const auto path = sourceQuery.getColumn("in_archive").getInt() != 0
                            ? sourceQuery.getColumn("archive_file_path").getString()
                            : sourceQuery.getColumn("file_path").getString();

      for (const auto index : found->second) {
        auto &entry = entries[index];

        if (directoryId >= 0 &&
            std::ranges::find(entry.contentDirectoryIds, directoryId) == entry.contentDirectoryIds.end()) {
          entry.contentDirectoryIds.push_back(directoryId);
        }

        entry.contentPaths.push_back(path);

        if (isReadable) {
          entry.readableContentPaths.push_back(path);
          entry.isContentAvailable = true;
        }
      }
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get content locations: {}", e.what());
  }

  return entries;
}

std::optional<Entry> SqliteUserLibraryRepository::getEntry(const int entryId) {
  std::lock_guard lock(m_mutex);
  Entry entry;

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM entries WHERE id = :entryId LIMIT 1;");
    query.bind(":entryId", entryId);

    if (!query.executeStep()) {
      spdlog::debug("No entry with ID {}", entryId);
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

  try {
    SQLite::Statement tagQuery(*m_db, "SELECT tag_id FROM entry_tags WHERE entry_id = :entryId;");
    tagQuery.bind(":entryId", entry.id);

    while (tagQuery.executeStep()) {
      entry.tagIds.push_back(tagQuery.getColumn("tag_id").getInt());
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get tag IDs for entry {}: {}", entry.id, e.what());
  }

  populateEntrySource(entry);
  return entry;
}

std::optional<Entry> SqliteUserLibraryRepository::getEntryWithContentHash(const std::string &contentHash) {
  std::lock_guard lock(m_mutex);
  Entry entry;

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM entries WHERE content_hash = :contentHash LIMIT 1;");
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

  try {
    SQLite::Statement tagQuery(*m_db, "SELECT tag_id FROM entry_tags WHERE entry_id = :entryId;");
    tagQuery.bind(":entryId", entry.id);

    while (tagQuery.executeStep()) {
      entry.tagIds.push_back(tagQuery.getColumn("tag_id").getInt());
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get tag IDs for entry {}: {}", entry.id, e.what());
  }

  populateEntrySource(entry);
  return entry;
}

void SqliteUserLibraryRepository::populateEntrySource(Entry &entry) {
  try {
    // TODO
    // A playlist carries its first disc's hash with no disc number of its own, so ordering
    // numbered files first keeps it out of the front of contentPaths
    SQLite::Statement query(*m_db, "SELECT content_directory_id, file_path, in_archive, "
                                   "archive_file_path, disc_number, game_id, missing_since FROM content_files WHERE "
                                   "content_hash = :contentHash "
                                   "ORDER BY (disc_number = 0), disc_number, id;");
    query.bind(":contentHash", entry.contentHash);

    while (query.executeStep()) {
      const auto isReadable = query.getColumn("missing_since").getInt64() == 0;

      // Which disc this is belongs to the file; the entry reads it rather than storing it again
      if (entry.discNumber == 0) {
        entry.discNumber = query.getColumn("disc_number").getInt();
      }

      const int dirId = query.getColumn("content_directory_id").getInt();
      if (dirId >= 0 && std::ranges::find(entry.contentDirectoryIds, dirId) == entry.contentDirectoryIds.end()) {
        entry.contentDirectoryIds.push_back(dirId);
      }

      const auto path = query.getColumn("in_archive").getInt() != 0 ? query.getColumn("archive_file_path").getString()
                                                                    : query.getColumn("file_path").getString();
      entry.contentPaths.push_back(path);

      if (isReadable) {
        entry.readableContentPaths.push_back(path);
        entry.isContentAvailable = true;
      }
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
          .discSetId = query.getColumn("disc_set_id").isNull() ? std::nullopt
                                                               : std::optional(query.getColumn("disc_set_id").getInt()),
          .createdAt = static_cast<uint32_t>(query.getColumn("created_at").getInt64()),
      });
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get run configurations: {}", e.what());
  }
  return runConfigurations;
}

std::vector<ContentFile> SqliteUserLibraryRepository::getContentFiles() { return contentFiles(false); }

std::vector<ContentFile> SqliteUserLibraryRepository::getPresentContentFiles() { return contentFiles(true); }

std::vector<ContentFile> SqliteUserLibraryRepository::contentFiles(const bool presentOnly) {
  std::lock_guard lock(m_mutex);
  std::vector<ContentFile> romFiles;

  try {
    SQLite::Statement query(*m_db, presentOnly ? "SELECT * FROM content_files WHERE missing_since = 0;"
                                               : "SELECT * FROM content_files;");
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

std::vector<ContentFile> SqliteUserLibraryRepository::getContentFilesWithContentHash(const std::string &contentHash) {
  std::lock_guard lock(m_mutex);
  std::vector<ContentFile> contentFiles;

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM content_files WHERE content_hash = :contentHash "
                                   "ORDER BY disc_number, id;");
    query.bind(":contentHash", contentHash);

    while (query.executeStep()) {
      contentFiles.push_back(deserializeContentFile(query));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get content files for hash {}: {}", contentHash, e.what());
  }

  return contentFiles;
}

std::optional<ContentFile> SqliteUserLibraryRepository::getContentFileWithPath(const std::string &filePath) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement query(*m_db, "SELECT * FROM content_files WHERE file_path = :filePath LIMIT 1;");
    query.bind(":filePath", filePath);

    if (!query.executeStep()) {
      return std::nullopt;
    }

    return deserializeContentFile(query);
  } catch (const std::exception &e) {
    spdlog::error("Failed to get content file at {}: {}", filePath, e.what());
    return std::nullopt;
  }
}

bool SqliteUserLibraryRepository::setContentFileIdentity(const int contentFileId, const std::string &contentHash,
                                                         const size_t fileSizeBytes) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement query(*m_db, "UPDATE content_files SET content_hash = :contentHash, "
                                   "file_size = :fileSize WHERE id = :id;");
    query.bind(":id", contentFileId);
    query.bind(":contentHash", contentHash);
    query.bind(":fileSize", static_cast<int64_t>(fileSizeBytes));

    return query.exec() != 0;
  } catch (const std::exception &e) {
    spdlog::error("Failed to re-stamp content file {}: {}", contentFileId, e.what());
    return false;
  }
}

bool SqliteUserLibraryRepository::deleteRunConfigurationsForContentFile(const int contentFileId) {
  std::string contentHash;

  {
    std::lock_guard lock(m_mutex);

    try {
      SQLite::Statement query(*m_db, "SELECT content_hash FROM run_configurations WHERE "
                                     "content_file_id = :contentFileId AND disc_set_id IS NULL LIMIT 1;");
      query.bind(":contentFileId", contentFileId);

      if (!query.executeStep()) {
        return false;
      }

      contentHash = query.getColumn("content_hash").getString();

      SQLite::Statement deleteQuery(*m_db, "DELETE FROM run_configurations WHERE "
                                           "content_file_id = :contentFileId AND disc_set_id IS NULL;");
      deleteQuery.bind(":contentFileId", contentFileId);
      deleteQuery.exec();
    } catch (const std::exception &e) {
      spdlog::error("Failed to delete run configurations for content file {}: {}", contentFileId, e.what());
      return false;
    }
  }

  EventDispatcher::instance().publish(RunConfigurationDeletedEvent{.contentHash = contentHash});
  return true;
}

bool SqliteUserLibraryRepository::deleteEntry(const int entryId) {
  {
    std::lock_guard lock(m_mutex);

    try {
      SQLite::Statement query(*m_db, "DELETE FROM entries WHERE id = :id;");
      query.bind(":id", entryId);

      if (query.exec() == 0) {
        return false;
      }

      for (const auto *statement :
           {"DELETE FROM folder_entries WHERE entry_id = :id;", "DELETE FROM entry_tags WHERE entry_id = :id;",
            "DELETE FROM entry_disc_state WHERE entry_id = :id;",
            "UPDATE variant_groups SET primary_entry_id = NULL WHERE primary_entry_id = :id;"}) {
        SQLite::Statement cleanup(*m_db, statement);
        cleanup.bind(":id", entryId);
        cleanup.exec();
      }
    } catch (const std::exception &e) {
      spdlog::error("Failed to delete entry {}: {}", entryId, e.what());
      return false;
    }
  }

  EventDispatcher::instance().publish(EntryDeletedEvent{.entryId = entryId});
  return true;
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
    SQLite::Statement query(*m_db, "SELECT * FROM content_directories;");
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
    SQLite::Statement query(*m_db, "INSERT OR IGNORE INTO content_directories (path, created_at) "
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
    SQLite::Statement selectQuery(*m_db, "SELECT path FROM content_directories WHERE id = :id;");
    selectQuery.bind(":id", directory.id);

    if (!selectQuery.executeStep()) {
      return true; // nothing to update
    }

    oldPath = selectQuery.getColumn("path").getString();

    SQLite::Statement query(*m_db, "UPDATE content_directories SET path = :path WHERE id = :id;");
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
                                                         const int platformId, const std::string &contentHash,
                                                         const std::string_view type) {
  std::lock_guard lock(m_mutex);
  int64_t rowId = 0;

  try {
    SQLite::Statement query(*m_db, "INSERT OR IGNORE INTO run_configurations "
                                   "(type, content_hash, content_file_id, created_at) "
                                   "VALUES (:type, :contentHash, :contentFileId, :createdAt);");
    query.bind(":type", std::string(type));
    query.bind(":contentHash", contentHash);
    query.bind(":contentFileId", contentFileId);
    query.bind(":createdAt", nowMs());
    query.exec();

    rowId = m_db->getLastInsertRowid();
  } catch (const std::exception &e) {
    spdlog::error("Failed to create run configuration: {}", e.what());
  }

  EventDispatcher::instance().publish(RunConfigurationCreatedEvent{
      .id = static_cast<int>(rowId), .filePath = path, .platformId = platformId, .contentHash = contentHash});
}

void SqliteUserLibraryRepository::createRunConfigurationForSet(const int setId, const int anchorContentFileId,
                                                               const std::string &contentHash) {
  std::lock_guard lock(m_mutex);

  try {
    // The unique index on disc_set_id is what makes this replace the set's way in rather than
    // add a second one when the anchor moves to a lower-numbered disc
    SQLite::Statement query(*m_db, "INSERT INTO run_configurations "
                                   "(type, content_hash, content_file_id, disc_set_id, created_at) "
                                   "VALUES (:type, :contentHash, :contentFileId, :discSetId, :createdAt) "
                                   "ON CONFLICT(disc_set_id) WHERE disc_set_id IS NOT NULL DO UPDATE SET "
                                   "content_hash = excluded.content_hash, content_file_id = excluded.content_file_id;");
    query.bind(":type", std::string(RunConfiguration::TYPE_PLAYLIST));
    query.bind(":contentHash", contentHash);
    query.bind(":contentFileId", anchorContentFileId);
    query.bind(":discSetId", setId);
    query.bind(":createdAt", nowMs());
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to create run configuration for disc set {}: {}", setId, e.what());
  }
}

bool SqliteUserLibraryRepository::deleteRunConfigurationsForDiscSet(const int setId) {
  std::string contentHash;

  {
    std::lock_guard lock(m_mutex);

    try {
      SQLite::Statement query(*m_db, "SELECT content_hash FROM run_configurations WHERE "
                                     "disc_set_id = :setId LIMIT 1;");
      query.bind(":setId", setId);

      if (!query.executeStep()) {
        return false;
      }

      contentHash = query.getColumn("content_hash").getString();

      SQLite::Statement deleteQuery(*m_db, "DELETE FROM run_configurations WHERE disc_set_id = :setId;");
      deleteQuery.bind(":setId", setId);
      deleteQuery.exec();
    } catch (const std::exception &e) {
      spdlog::error("Failed to delete run configurations for disc set {}: {}", setId, e.what());
      return false;
    }
  }

  EventDispatcher::instance().publish(RunConfigurationDeletedEvent{.contentHash = contentHash});
  return true;
}

bool SqliteUserLibraryRepository::createEntry(Entry &entry) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement selectQuery(*m_db, "SELECT id FROM entries WHERE content_hash = :contentHash;");
    selectQuery.bind(":contentHash", entry.contentHash);

    if (selectQuery.executeStep()) {
      spdlog::debug("Entry with content hash {} already exists, skipping creation", entry.contentHash);
      return false;
    }

    SQLite::Statement entryQuery(*m_db, "INSERT INTO entries (display_name, content_hash, platform_id, "
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
