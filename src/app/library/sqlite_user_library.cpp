#include "sqlite_user_library.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::library {
SqliteUserLibrary::SqliteUserLibrary(QString path, QString mainGameDirectory)
    : m_mainGameDirectory(std::move(mainGameDirectory)),
      m_databasePath(std::move(path)) {
  m_settings.beginGroup("Library");
  m_mainGameDirectory =
      m_settings.value("MainGameDirectory", m_mainGameDirectory).toString();

  QSqlQuery createRomFilesTable(getDatabase());
  createRomFilesTable.prepare("CREATE TABLE IF NOT EXISTS rom_files("
                              "id INTEGER PRIMARY KEY,"
                              "file_path TEXT NOT NULL,"
                              "file_size INTEGER NOT NULL,"
                              "file_md5 TEXT NOT NULL,"
                              "file_crc32 TEXT NOT NULL,"
                              "in_archive INTEGER NOT NULL DEFAULT 0,"
                              "archive_file_path TEXT,"
                              "platform_id INTEGER NOT NULL,"
                              "content_hash TEXT NOT NULL,"
                              "firelight_rom_id INTEGER,"
                              "created_at INTEGER NOT NULL);");

  if (!createRomFilesTable.exec()) {
    spdlog::error("Table creation failed: {}",
                  createRomFilesTable.lastError().text().toStdString());
  }

  QSqlQuery createPatchFilesTable(getDatabase());
  createPatchFilesTable.prepare("CREATE TABLE IF NOT EXISTS patch_files("
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

  if (!createPatchFilesTable.exec()) {
    spdlog::error("Table creation failed: {}",
                  createPatchFilesTable.lastError().text().toStdString());
  }

  QSqlQuery createEntriesTable(getDatabase());
  createEntriesTable.prepare("CREATE TABLE IF NOT EXISTS entriesv1("
                             "id INTEGER PRIMARY KEY,"
                             "display_name TEXT NOT NULL,"
                             "content_hash TEXT NOT NULL,"
                             "platform_id INTEGER NOT NULL,"
                             "active_save_slot INTEGER NOT NULL DEFAULT 1, "
                             "hidden INTEGER NOT NULL DEFAULT 0, "
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
                             "created_at INTEGER NOT NULL);");

  if (!createEntriesTable.exec()) {
    spdlog::error("Table creation failed: {}",
                  createEntriesTable.lastError().text().toStdString());
  }

  QSqlQuery createRunConfigurationsTable(getDatabase());
  createRunConfigurationsTable.prepare(
      "CREATE TABLE IF NOT EXISTS run_configurations("
      "id INTEGER PRIMARY KEY,"
      "type TEXT NOT NULL,"
      "content_hash TEXT NOT NULL,"
      "rom_id INTEGER NOT NULL,"
      "patch_id INTEGER,"
      "created_at INTEGER NOT NULL,"
      "UNIQUE (type, rom_id, patch_id),"
      "UNIQUE (type, rom_id));");

  if (!createRunConfigurationsTable.exec()) {
    spdlog::error("Table creation failed: {}",
                  createEntriesTable.lastError().text().toStdString());
  }

  QSqlQuery createDirectoriesTable(getDatabase());
  createDirectoriesTable.prepare(
      "CREATE TABLE IF NOT EXISTS watched_directoriesv1("
      "id INTEGER PRIMARY KEY,"
      "path TEXT NOT NULL,"
      "num_files INTEGER NOT NULL DEFAULT 0,"
      "num_content_files INTEGER NOT NULL DEFAULT 0,"
      "last_modified INTEGER DEFAULT 0,"
      "recursive INTEGER NOT NULL DEFAULT 1,"
      "created_at INTEGER NOT NULL);");

  if (!createDirectoriesTable.exec()) {
    spdlog::error("Table creation failed: {}",
                  createDirectoriesTable.lastError().text().toStdString());
  }

  QSqlQuery createSubdirectoriesTable(getDatabase());
  createSubdirectoriesTable.prepare(
      "CREATE TABLE IF NOT EXISTS subdirectories("
      "id INTEGER PRIMARY KEY,"
      "path TEXT NOT NULL,"
      "num_files INTEGER NOT NULL DEFAULT 0,"
      "num_content_files INTEGER NOT NULL DEFAULT 0,"
      "last_modified INTEGER DEFAULT 0,"
      "recursive INTEGER NOT NULL DEFAULT 1,"
      "created_at INTEGER NOT NULL);");

  if (!createSubdirectoriesTable.exec()) {
    spdlog::error("Table creation failed: {}",
                  createSubdirectoriesTable.lastError().text().toStdString());
  }

  QSqlQuery createFoldersTable(getDatabase());
  createFoldersTable.prepare("CREATE TABLE IF NOT EXISTS folders("
                             "id INTEGER PRIMARY KEY,"
                             "display_name TEXT UNIQUE NOT NULL,"
                             "description TEXT,"
                             "icon_source_url TEXT,"
                             "created_at INTEGER NOT NULL);");

  if (!createFoldersTable.exec()) {
    spdlog::error("Table creation failed: {}",
                  createFoldersTable.lastError().text().toStdString());
  }

  QSqlQuery createFolderEntriesTable(getDatabase());
  createFolderEntriesTable.prepare("CREATE TABLE IF NOT EXISTS folder_entries("
                                   "folder_id INTEGER NOT NULL,"
                                   "entry_id INTEGER NOT NULL,"
                                   "created_at INTEGER NOT NULL,"
                                   "UNIQUE (folder_id, entry_id));");

  if (!createFolderEntriesTable.exec()) {
    spdlog::error("Table creation failed: {}",
                  createFolderEntriesTable.lastError().text().toStdString());
  }
}

SqliteUserLibrary::~SqliteUserLibrary() {
  m_settings.setValue("MainGameDirectory", m_mainGameDirectory);
  for (const auto &name : QSqlDatabase::connectionNames()) {
    if (name.startsWith(DATABASE_PREFIX)) {
      QSqlDatabase::removeDatabase(name);
    }
  }
}

bool SqliteUserLibrary::create(FolderInfo &folder) {
  QSqlQuery query(getDatabase());
  query.prepare("INSERT INTO folders("
                "display_name, "
                "description, "
                "icon_source_url, "
                "created_at) VALUES"
                "(:displayName, :description, :iconSourceUrl, :createdAt);");

  query.bindValue(":displayName", QString::fromStdString(folder.displayName));
  query.bindValue(":description", QString::fromStdString(folder.description));
  query.bindValue(":iconSourceUrl",
                  QString::fromStdString(folder.iconSourceUrl));
  query.bindValue(":createdAt", QDateTime::currentSecsSinceEpoch());

  if (!query.exec()) {
    spdlog::error("Failed to create folder: {}",
                  query.lastError().text().toStdString());
    return false;
  }

  folder.id = query.lastInsertId().toInt();

  return true;
}

bool SqliteUserLibrary::create(FolderEntryInfo &folderEntry) {
  QSqlQuery query(getDatabase());
  query.prepare("INSERT INTO folder_entries("
                "folder_id, "
                "entry_id, "
                "created_at) VALUES"
                "(:folderId, :entryId, :createdAt);");

  query.bindValue(":folderId", folderEntry.folderId);
  query.bindValue(":entryId", folderEntry.entryId);
  query.bindValue(":createdAt", QDateTime::currentSecsSinceEpoch());

  if (!query.exec()) {
    spdlog::error("Failed to create folder entry: {}",
                  query.lastError().text().toStdString());
    return false;
  }

  emit entryAddedToFolder(folderEntry.folderId, folderEntry.entryId);
  return true;
}

std::vector<FolderInfo>
SqliteUserLibrary::listFolders(const FolderInfo filter) {
  // TODO: Implement filtering

  QSqlQuery query(getDatabase());
  query.prepare("SELECT * FROM folders");

  if (!query.exec()) {
    spdlog::error("Failed to get folders: {}",
                  query.lastError().text().toStdString());
    return {};
  }

  std::vector<FolderInfo> folders;
  while (query.next()) {
    folders.emplace_back(FolderInfo{
        .id = query.value("id").toInt(),
        .displayName = query.value("display_name").toString().toStdString(),
        .description = query.value("description").toString().toStdString(),
        .iconSourceUrl =
            query.value("icon_source_url").toString().toStdString(),
        .createdAt = query.value("created_at").toULongLong()});
  }

  return folders;
}

bool SqliteUserLibrary::deleteFolder(int folderId) {
  QSqlQuery query(getDatabase());
  query.prepare("DELETE FROM folders WHERE id = :folderId;");
  query.bindValue(":folderId", folderId);

  if (!query.exec()) {
    spdlog::error("Failed to delete folder with ID {}: {}", folderId,
                  query.lastError().text().toStdString());
    return false;
  }

  query.finish();

  // Also delete all entries in the folder_entries table
  QSqlQuery deleteEntriesQuery(getDatabase());
  deleteEntriesQuery.prepare(
      "DELETE FROM folder_entries WHERE folder_id = :folderId;");
  deleteEntriesQuery.bindValue(":folderId", folderId);

  if (!deleteEntriesQuery.exec()) {
    spdlog::error("Failed to delete entries for folder ID {}: {}", folderId,
                  deleteEntriesQuery.lastError().text().toStdString());
    return false;
  }

  return true;
}

bool SqliteUserLibrary::update(FolderInfo &folder) {
  if (folder.id <= 0) {
    spdlog::error("Cannot update folder with invalid ID: {}", folder.id);
    return false;
  }

  QSqlQuery query(getDatabase());
  query.prepare("UPDATE folders SET "
                "display_name = :displayName, "
                "description = :description, "
                "icon_source_url = :iconSourceUrl "
                "WHERE id = :folderId;");

  query.bindValue(":folderId", folder.id);
  query.bindValue(":displayName", QString::fromStdString(folder.displayName));
  query.bindValue(":description", QString::fromStdString(folder.description));
  query.bindValue(":iconSourceUrl",
                  QString::fromStdString(folder.iconSourceUrl));

  if (!query.exec() || query.numRowsAffected() == 0) {
    spdlog::error("Failed to update folder with ID {}: {}", folder.id,
                  query.lastError().text().toStdString());
    return false;
  }

  return true;
}

bool SqliteUserLibrary::deleteFolderEntry(FolderEntryInfo &info) {
  QSqlQuery query(getDatabase());
  query.prepare("DELETE FROM folder_entries WHERE folder_id = :folderId AND "
                "entry_id = :entryId;");
  query.bindValue(":folderId", info.folderId);
  query.bindValue(":entryId", info.entryId);

  if (!query.exec()) {
    spdlog::error("Failed to delete folder entry: {}",
                  query.lastError().text().toStdString());
    return false;
  }

  return true;
}
bool SqliteUserLibrary::update(Entry &entry) {
  QSqlQuery query(getDatabase());
  query.prepare("UPDATE entriesv1 SET "
                "display_name = :displayName, "
                "active_save_slot = :activeSaveSlot, "
                "hidden = :hidden WHERE id = :id;");

  query.bindValue(":id", entry.id);
  query.bindValue(":displayName", entry.displayName);
  query.bindValue(":activeSaveSlot", entry.activeSaveSlot);
  query.bindValue(":hidden", entry.hidden ? 1 : 0);
  if (!query.exec() || query.numRowsAffected() == 0) {
    spdlog::error("Failed to update entry with ID {}: {}", entry.id,
                  query.lastError().text().toStdString());
    return false;
  }

  return true;
}

void SqliteUserLibrary::setMainGameDirectory(const QString &directory) {
  auto temp = directory;

  if (temp.startsWith("file://")) {
    temp = temp.remove(0, 7);
  }
  if (temp.startsWith("/")) {
    temp = temp.remove(0, 1);
  }

  if (temp == m_mainGameDirectory) {
    return;
  }

  m_mainGameDirectory = temp;
  m_settings.setValue("MainGameDirectory", m_mainGameDirectory);
  emit mainGameDirectoryChanged(m_mainGameDirectory);
}

QString SqliteUserLibrary::getMainGameDirectory() {
  return m_mainGameDirectory;
}

void SqliteUserLibrary::create(RomFile &romFile) {
  const QString queryString = "INSERT INTO rom_files ("
                              "file_path, "
                              "file_size, "
                              "file_md5, "
                              "file_crc32,"
                              "in_archive, "
                              "archive_file_path, "
                              "platform_id, "
                              "content_hash, "
                              "created_at) VALUES"
                              "(:filePath, :fileSize, :fileMd5, :fileCrc32, "
                              ":inArchive, :archiveFilePath, :platformId, "
                              ":contentHash, :createdAt);";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);
  query.bindValue(":filePath", romFile.getFilePath());
  query.bindValue(":fileSize", romFile.getFileSizeBytes());
  query.bindValue(":fileMd5", romFile.getFileMd5());
  query.bindValue(":fileCrc32", romFile.getFileCrc32());
  query.bindValue(":inArchive", romFile.inArchive());
  query.bindValue(":archiveFilePath", romFile.getArchivePathName());
  query.bindValue(":platformId", romFile.getPlatformId());
  query.bindValue(":contentHash", romFile.getContentHash());
  query.bindValue(":createdAt",
                  std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count());

  if (!query.exec()) {
    spdlog::error("Failed to add rom file with path {}: {}",
                  romFile.getFilePath().toStdString(),
                  query.lastError().text().toStdString());
    return;
  }

  emit romFileAdded(query.lastInsertId().toInt(), romFile.getContentHash());

  const QString getEntryQueryString =
      "SELECT id FROM entriesv1 WHERE content_hash = :contentHash;";
  QSqlQuery getEntryQuery(getDatabase());
  getEntryQuery.prepare(getEntryQueryString);
  getEntryQuery.bindValue(":contentHash", romFile.getContentHash());

  if (!getEntryQuery.exec()) {
    spdlog::error("Failed to get entry with content hash {}: {}",
                  romFile.getContentHash().toStdString(),
                  getEntryQuery.lastError().text().toStdString());
    return;
  }

  if (!getEntryQuery.next()) {
    const auto filename = romFile.getFilePath().split("/").last();

    const QString entryQueryString =
        "INSERT INTO entriesv1 ("
        "display_name, "
        "content_hash, "
        "platform_id, "
        "created_at) VALUES"
        "(:displayName, :contentHash, :platformId, :createdAt);";
    QSqlQuery entryQuery(getDatabase());
    entryQuery.prepare(entryQueryString);
    entryQuery.bindValue(":displayName", filename);
    entryQuery.bindValue(":contentHash", romFile.getContentHash());
    entryQuery.bindValue(":platformId", romFile.getPlatformId());
    entryQuery.bindValue(
        ":createdAt", std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count());

    if (!entryQuery.exec()) {
      spdlog::error("Failed to add entry with content hash {}: {}",
                    romFile.getContentHash().toStdString(),
                    query.lastError().text().toStdString());
      return;
    }

    emit entryCreated(entryQuery.lastInsertId().toInt(),
                      romFile.getContentHash());
  }
}

std::optional<RomFile> SqliteUserLibrary::getRomFileWithPathAndSize(
    const QString &filePath, const size_t fileSizeBytes, const bool inArchive) {
  const QString queryString =
      "SELECT * FROM rom_files WHERE file_path = :filePath AND file_size = "
      ":fileSize AND in_archive = :inArchive;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);
  query.bindValue(":filePath", filePath);
  query.bindValue(":fileSize", fileSizeBytes);
  query.bindValue(":inArchive", inArchive);

  if (!query.exec()) {
    spdlog::error("Failed to get rom file with path {}: {}",
                  filePath.toStdString(),
                  query.lastError().text().toStdString());
  }

  if (!query.next()) {
    return std::nullopt;
  }

  return {RomFile(query)};
}

std::vector<Entry> SqliteUserLibrary::getEntries(int offset, int limit) {
  const QString queryString = R"(
            SELECT e.*,
                   CASE
                       WHEN EXISTS (SELECT 1 FROM rom_files rf WHERE rf.content_hash = e.content_hash)
                       THEN 1
                       ELSE 0
                   END AS has_rom
            FROM entriesv1 e
            ORDER BY e.display_name ASC;
        )";

  QSqlQuery query(getDatabase());
  query.prepare(queryString);

  if (!query.exec()) {
    spdlog::error("Failed to get entries: {}",
                  query.lastError().text().toStdString());
    return {};
  }

  std::vector<Entry> entries;

  while (query.next()) {
    // Get the entry ID
    int entryId = query.value("id").toInt();
    // Query for folder IDs for this entry
    // QSqlQuery folderQuery(getDatabase());
    // folderQuery.prepare("SELECT folder_id FROM folder_entries WHERE entry_id
    // = :entryId;"); folderQuery.bindValue(":entryId", entryId);
    // std::vector<int> folderIds;
    // if (folderQuery.exec()) {
    //   while (folderQuery.next()) {
    //     folderIds.push_back(folderQuery.value("folder_id").toInt());
    //   }
    // } else {
    //   spdlog::error("Failed to get folder IDs for entry {}: {}", entryId,
    //   folderQuery.lastError().text().toStdString());
    // }

    entries.emplace_back(Entry{
        .id = entryId,
        .displayName = query.value("display_name").toString(),
        .contentHash = query.value("content_hash").toString(),
        .platformId = query.value("platform_id").toUInt(),
        .activeSaveSlot = query.value("active_save_slot").toUInt(),
        // TODO
        // .hidden = query.value("hidden").toBool(),
        .hidden = !query.value("has_rom").toBool(),
        .icon1x1SourceUrl = query.value("icon_1x1_source_url").toString(),
        .boxartFrontSourceUrl =
            query.value("boxart_front_source_url").toString(),
        .boxartBackSourceUrl = query.value("boxart_back_source_url").toString(),
        .description = query.value("description").toString(),
        .releaseYear = query.value("release_year").toUInt(),
        .developer = query.value("developer").toString(),
        .publisher = query.value("publisher").toString(),
        .genres = query.value("genres").toString(),
        .regionIds = query.value("region_ids").toString(),
        .retroachievementsSetId =
            query.value("retroachievements_set_id").toUInt(),
        .createdAt = query.value("created_at").toUInt()});
  }

  query.finish();

  for (auto &entry : entries) {
    // Get folder IDs for each entry
    QSqlQuery folderQuery(getDatabase());
    folderQuery.prepare(
        "SELECT folder_id FROM folder_entries WHERE entry_id = :entryId;");
    folderQuery.bindValue(":entryId", entry.id);

    if (folderQuery.exec()) {
      while (folderQuery.next()) {
        entry.folderIds.push_back(folderQuery.value("folder_id").toInt());
      }
    } else {
      spdlog::error("Failed to get folder IDs for entry {}: {}", entry.id,
                    folderQuery.lastError().text().toStdString());
    }
  }

  return entries;
}

std::optional<Entry> SqliteUserLibrary::getEntry(const int entryId) {
  const QString queryString =
      "SELECT * FROM entriesv1 WHERE id = :entryId LIMIT 1;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);
  query.bindValue(":entryId", entryId);

  if (!query.exec()) {
    spdlog::error("Failed to get entry: {}",
                  query.lastError().text().toStdString());
    return {};
  }

  if (!query.next()) {
    spdlog::error("Failed to get entry: {}",
                  query.lastError().text().toStdString());
    return {};
  }

  return {Entry{
      .id = query.value("id").toInt(),
      .displayName = query.value("display_name").toString(),
      .contentHash = query.value("content_hash").toString(),
      .platformId = query.value("platform_id").toUInt(),
      .activeSaveSlot = query.value("active_save_slot").toUInt(),
      .hidden = query.value("hidden").toBool(),
      .icon1x1SourceUrl = query.value("icon_1x1_source_url").toString(),
      .boxartFrontSourceUrl = query.value("boxart_front_source_url").toString(),
      .boxartBackSourceUrl = query.value("boxart_back_source_url").toString(),
      .description = query.value("description").toString(),
      .releaseYear = query.value("release_year").toUInt(),
      .developer = query.value("developer").toString(),
      .publisher = query.value("publisher").toString(),
      .genres = query.value("genres").toString(),
      .regionIds = query.value("region_ids").toString(),
      .retroachievementsSetId =
          query.value("retroachievements_set_id").toUInt(),
      .createdAt = query.value("created_at").toUInt()}};
}

std::vector<RunConfiguration>
SqliteUserLibrary::getRunConfigurations(const QString &contentHash) {
  const auto queryString =
      "SELECT * FROM run_configurations WHERE content_hash = :contentHash;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);

  query.bindValue(":contentHash", contentHash);
  if (!query.exec()) {
    spdlog::error("Failed to get run configurations: {}",
                  query.lastError().text().toStdString());
  }

  std::vector<RunConfiguration> runConfigurations;
  while (query.next()) {
    RunConfiguration runConfiguration{
        .id = query.value("id").toInt(),
        .type = query.value("type").toString().toStdString(),
        .contentHash = contentHash.toStdString(),
        .romId = query.value("rom_id").toInt(),
        .patchId = query.value("patch_id").toInt(),
        .createdAt = query.value("created_at").toUInt()};

    runConfigurations.push_back(runConfiguration);
  }

  return runConfigurations;
}

void SqliteUserLibrary::doStuffWithRunConfigurations() {
  // Get all roms, make sure a run configuration exists
  // Get all patches, see if we have a match, if so, create a run configuration
  // Get all run configurations, make sure that the roms and patches exist

  auto romFiles = getRomFiles();
  for (auto &romFile : romFiles) {
    const QString insertQueryString =
        "INSERT OR IGNORE INTO run_configurations "
        "(type, content_hash, rom_id, created_at) "
        "VALUES (:type, :contentHash, :romId, :createdAt);";

    QSqlQuery query(getDatabase());
    query.prepare(insertQueryString);

    query.bindValue(":type", QString::fromStdString("rom"));
    query.bindValue(":contentHash", romFile.getContentHash());
    query.bindValue(":romId", romFile.getId());
    query.bindValue(":createdAt",
                    QDateTime::currentSecsSinceEpoch()); // Current timestamp

    if (!query.exec()) {
      spdlog::error("Failed to create run configuration: {}",
                    query.lastError().text().toStdString());
    }
  }
}

std::vector<RomFile>
SqliteUserLibrary::getRomFilesWithContentHash(const QString &contentHash) {
  const QString queryString =
      "SELECT * FROM rom_files WHERE content_hash = :contentHash;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);
  query.bindValue(":contentHash", contentHash);

  if (!query.exec()) {
    spdlog::error("Failed to get rom files with content hash {}: {}",
                  contentHash.toStdString(),
                  query.lastError().text().toStdString());
  }

  std::vector<RomFile> romFiles;

  while (query.next()) {
    romFiles.emplace_back(query);
  }

  return romFiles;
}

std::vector<RomFile> SqliteUserLibrary::getRomFiles() {
  const QString queryString = "SELECT * FROM rom_files;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);

  if (!query.exec()) {
    spdlog::error("Failed to get rom files: {}",
                  query.lastError().text().toStdString());
  }

  std::vector<RomFile> directories;

  while (query.next()) {
    directories.emplace_back(query);
  }

  return directories;
}

std::optional<RomFile> SqliteUserLibrary::getRomFile(int id) {
  auto queryString = "SELECT * FROM rom_files WHERE id = :id LIMIT 1;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);
  query.bindValue(":id", id);

  if (!query.exec()) {
    spdlog::error("Failed to get rom file with id {}: {}", id,
                  query.lastError().text().toStdString());
  }

  if (!query.next()) {
    return std::nullopt;
  }

  return {RomFile(query)};
}
std::optional<PatchFile> SqliteUserLibrary::getPatchFile(int id) {
  auto queryString = "SELECT * FROM patch_files WHERE id = :id LIMIT 1;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);
  query.bindValue(":id", id);

  if (!query.exec()) {
    spdlog::error("Failed to get patch file with id {}: {}", id,
                  query.lastError().text().toStdString());
  }

  if (!query.next()) {
    return std::nullopt;
  }

  PatchFile patchFile;
  patchFile.m_filePath = query.value("file_path").toString().toStdString();
  patchFile.m_fileSize = query.value("file_size").toInt();
  patchFile.m_fileMd5 = query.value("file_md5").toString().toStdString();

  return {patchFile};
}

void SqliteUserLibrary::create(PatchFile &file) {
  const QString queryString =
      "INSERT OR IGNORE INTO patch_files ("
      "file_path, "
      "file_size, "
      "file_md5, "
      "file_crc32, "
      "target_md5, "
      "patched_md5, "
      "patched_content_hash, "
      "in_archive, "
      "archive_file_path, "
      "created_at) VALUES"
      "(:filePath, :fileSize, :fileMd5, :fileCrc32, :targetMd5, "
      ":patchedMd5, "
      ":patchedContentHash, :inArchive, :archiveFilePath, :createdAt);";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);
  query.bindValue(":filePath", QString::fromStdString(file.m_filePath));
  query.bindValue(":fileSize", file.m_fileSize);
  query.bindValue(":fileMd5", QString::fromStdString(file.m_fileMd5));
  query.bindValue(":fileCrc32", QString::fromStdString(file.m_fileCrc32));
  query.bindValue(":targetMd5", QString::fromStdString(file.m_targetFileMd5));
  query.bindValue(":patchedMd5", QString::fromStdString(file.m_patchedMd5));
  query.bindValue(":patchedContentHash",
                  QString::fromStdString(file.m_patchedContentHash));
  query.bindValue(":inArchive", file.m_inArchive);
  query.bindValue(":archiveFilePath",
                  QString::fromStdString(file.m_archiveFilePath));
  query.bindValue(":createdAt",
                  std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count());

  if (!query.exec()) {
    spdlog::error("Failed to add patch file with path {}: {}", file.m_filePath,
                  query.lastError().text().toStdString());
  }
}

std::optional<PatchFile> SqliteUserLibrary::getPatchFileWithPathAndSize(
    const QString &filePath, size_t fileSizeBytes, bool inArchive) {}

std::vector<PatchFile> SqliteUserLibrary::getPatchFiles() {}

std::vector<WatchedDirectory> SqliteUserLibrary::getWatchedDirectories() {
  const QString queryString = "SELECT * FROM watched_directoriesv1;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);

  if (!query.exec()) {
    spdlog::error("Failed to get directories: {}",
                  query.lastError().text().toStdString());
  }

  std::vector<WatchedDirectory> directories;
  directories.emplace_back(WatchedDirectory{.path = m_mainGameDirectory});

  while (query.next()) {
    directories.emplace_back(WatchedDirectory{
        .id = query.value("id").toInt(),
        .path = query.value("path").toString(),
        .numFiles = query.value("num_files").toInt(),
        .numContentFiles = query.value("num_content_files").toInt(),
        .lastModified = QDateTime::fromMSecsSinceEpoch(
            query.value("last_modified").toUInt()),
        .recursive = query.value("recursive").toBool(),
        .createdAt = query.value("created_at").toUInt()});
  }

  return directories;
}

bool SqliteUserLibrary::addWatchedDirectory(const WatchedDirectory &directory) {
  const QString queryString = "INSERT INTO watched_directoriesv1 ("
                              "path, "
                              "created_at) VALUES"
                              "(:path, :createdAt);";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);
  query.bindValue(":path", directory.path);
  query.bindValue(":createdAt",
                  std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count());

  if (!query.exec()) {
    spdlog::error("Failed to add directory with path {}: {}",
                  directory.path.toStdString(),
                  query.lastError().text().toStdString());
    return false;
  }

  emit watchedDirectoryAdded(query.lastInsertId().toInt(), directory.path);
  return true;
}

bool SqliteUserLibrary::updateWatchedDirectory(
    const WatchedDirectory &directory) {
  QSqlQuery q(getDatabase());
  q.prepare("UPDATE watched_directoriesv1 SET path = :path WHERE id = :id");
  q.bindValue(":path", directory.path);
  q.bindValue(":id", directory.id);

  if (!q.exec()) {
    spdlog::error("Failed to update watched directory: {}",
                  q.lastError().text().toStdString());
    return false;
  }

  return true;
}

QSqlDatabase SqliteUserLibrary::getDatabase() const {
  const auto name =
      DATABASE_PREFIX +
      QString::number(reinterpret_cast<quint64>(QThread::currentThread()), 16);
  if (QSqlDatabase::contains(name)) {
    return QSqlDatabase::database(name);
  }

  spdlog::debug("Database connection with name {} does not exist; creating",
                name.toStdString());
  auto db = QSqlDatabase::addDatabase("QSQLITE", name);
  db.setDatabaseName(m_databasePath);
  db.open();
  return db;
}
} // namespace firelight::library
