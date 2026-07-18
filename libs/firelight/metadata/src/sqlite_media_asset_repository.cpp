#include <firelight/metadata/sqlite_media_asset_repository.hpp>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>
#include <chrono>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::metadata {
namespace {
int64_t nowMs() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

MediaAsset readAsset(const SQLite::Statement &query) {
  MediaAsset asset;
  asset.id = query.getColumn("id").getInt();
  asset.contentHash = query.getColumn("content_hash").getString();
  asset.type = static_cast<MediaType>(query.getColumn("media_type").getInt());
  asset.source = mediaSourceFromString(query.getColumn("source").getString());
  asset.remoteUrl = query.getColumn("remote_url").getString();
  asset.thumbUrl = query.getColumn("thumb_url").getString();
  asset.localPath = query.getColumn("local_path").getString();
  asset.width = query.getColumn("width").getInt();
  asset.height = query.getColumn("height").getInt();
  asset.externalId = query.getColumn("external_id").getString();
  asset.selected = query.getColumn("selected").getInt() != 0;
  asset.createdAt = static_cast<uint64_t>(query.getColumn("created_at").getInt64());
  return asset;
}
} // namespace

SqliteMediaAssetRepository::SqliteMediaAssetRepository(std::string databaseFile)
    : m_databaseFile(std::move(databaseFile)) {
  m_db = std::make_unique<SQLite::Database>(m_databaseFile, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
  try {
    // Nullable text is stored as '' (never NULL) so the uniqueness index below
    // actually dedupes
    m_db->exec("CREATE TABLE IF NOT EXISTS media_assets("
               "id INTEGER PRIMARY KEY,"
               "content_hash TEXT NOT NULL,"
               "media_type INTEGER NOT NULL,"
               "source TEXT NOT NULL,"
               "remote_url TEXT NOT NULL DEFAULT '',"
               "thumb_url TEXT NOT NULL DEFAULT '',"
               "local_path TEXT NOT NULL DEFAULT '',"
               "width INTEGER NOT NULL DEFAULT 0,"
               "height INTEGER NOT NULL DEFAULT 0,"
               "external_id TEXT NOT NULL DEFAULT '',"
               "selected INTEGER NOT NULL DEFAULT 0,"
               "created_at INTEGER NOT NULL);");
    m_db->exec("CREATE INDEX IF NOT EXISTS mediaContentHashIdx ON "
               "media_assets(content_hash);");
    m_db->exec("CREATE UNIQUE INDEX IF NOT EXISTS mediaUniqueIdx ON "
               "media_assets(content_hash, media_type, source, external_id, "
               "remote_url);");
  } catch (const std::exception &e) {
    spdlog::error("Failed to initialize media store: {}", e.what());
  }
}

SqliteMediaAssetRepository::~SqliteMediaAssetRepository() = default;

std::vector<MediaAsset> SqliteMediaAssetRepository::listForContent(const std::string &contentHash) {
  std::lock_guard lock(m_mutex);
  std::vector<MediaAsset> assets;
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM media_assets WHERE "
                                   "content_hash = :contentHash ORDER BY media_type, id");
    query.bind(":contentHash", contentHash);
    while (query.executeStep()) {
      assets.push_back(readAsset(query));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to list media for {}: {}", contentHash, e.what());
  }
  return assets;
}

std::vector<MediaAsset> SqliteMediaAssetRepository::listForContentAndType(const std::string &contentHash,
                                                                          MediaType type) {
  std::lock_guard lock(m_mutex);
  std::vector<MediaAsset> assets;
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM media_assets WHERE content_hash = :contentHash AND "
                                   "media_type = :mediaType ORDER BY selected DESC, id");
    query.bind(":contentHash", contentHash);
    query.bind(":mediaType", static_cast<int>(type));
    while (query.executeStep()) {
      assets.push_back(readAsset(query));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to list media for {} type {}: {}", contentHash, static_cast<int>(type), e.what());
  }
  return assets;
}

std::optional<MediaAsset> SqliteMediaAssetRepository::selectedFor(const std::string &contentHash, MediaType type) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM media_assets WHERE content_hash = :contentHash AND "
                                   "media_type = :mediaType AND selected = 1 LIMIT 1");
    query.bind(":contentHash", contentHash);
    query.bind(":mediaType", static_cast<int>(type));

    if (query.executeStep()) {
      return readAsset(query);
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get selected media for {}: {}", contentHash, e.what());
  }
  return std::nullopt;
}

bool SqliteMediaAssetRepository::add(MediaAsset &asset) {
  std::lock_guard lock(m_mutex);
  try {
    {
      SQLite::Statement insert(*m_db, "INSERT OR IGNORE INTO media_assets(content_hash, media_type, source, "
                                      "remote_url, thumb_url, local_path, width, height, external_id, "
                                      "selected, created_at) VALUES(:contentHash, :mediaType, :source, :remoteUrl, "
                                      ":thumbUrl, :localPath, :width, :height, "
                                      ":externalId, 0, :createdAt)");
      insert.bind(":contentHash", asset.contentHash);
      insert.bind(":mediaType", static_cast<int>(asset.type));
      insert.bind(":source", toString(asset.source));
      insert.bind(":remoteUrl", asset.remoteUrl);
      insert.bind(":thumbUrl", asset.thumbUrl);
      insert.bind(":localPath", asset.localPath);
      insert.bind(":width", asset.width);
      insert.bind(":height", asset.height);
      insert.bind(":externalId", asset.externalId);
      insert.bind(":createdAt", static_cast<int64_t>(asset.createdAt ? asset.createdAt : nowMs()));
      insert.exec();
    }
    // Resolve the id (existing or newly inserted) via the unique key, then
    // refresh the mutable fields (a re-add can update a cached local path/size)
    {
      SQLite::Statement select(*m_db, "SELECT id FROM media_assets WHERE content_hash = :contentHash AND "
                                      "media_type = :mediaType AND source = :source AND external_id = :externalId AND "
                                      "remote_url = :remoteUrl");
      select.bind(":contentHash", asset.contentHash);
      select.bind(":mediaType", static_cast<int>(asset.type));
      select.bind(":source", toString(asset.source));
      select.bind(":externalId", asset.externalId);
      select.bind(":remoteUrl", asset.remoteUrl);
      if (select.executeStep()) {
        asset.id = select.getColumn(0).getInt();
      }
    }
    if (asset.id <= 0) {
      return false;
    }
    {
      SQLite::Statement update(*m_db, "UPDATE media_assets SET thumb_url = :thumbUrl, local_path = :localPath, "
                                      "width = :width, height = :height WHERE id = :id");
      update.bind(":thumbUrl", asset.thumbUrl);
      update.bind(":localPath", asset.localPath);
      update.bind(":width", asset.width);
      update.bind(":height", asset.height);
      update.bind(":id", asset.id);
      update.exec();
    }
    if (asset.selected) {
      setSelected(asset.id);
    }
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to add media asset for {}: {}", asset.contentHash, e.what());
    return false;
  }
}

bool SqliteMediaAssetRepository::setSelected(int assetId) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Transaction transaction(*m_db);
    std::string contentHash;

    // CLion keeps formatting the scopes like this. TODO: Fix
    int type;
    {
      SQLite::Statement query(*m_db, "SELECT content_hash, media_type FROM "
                                     "media_assets WHERE id = :id");
      query.bind(":id", assetId);

      if (!query.executeStep()) {
        return false;
      }

      contentHash = query.getColumn(0).getString();
      type = query.getColumn(1).getInt();
    }
    {
      SQLite::Statement clear(*m_db, "UPDATE media_assets SET selected = 0 WHERE "
                                     "content_hash = :contentHash AND media_type = :mediaType");
      clear.bind(":contentHash", contentHash);
      clear.bind(":mediaType", type);
      clear.exec();
    }
    {
      SQLite::Statement set(*m_db, "UPDATE media_assets SET selected = 1 WHERE id = :id");
      set.bind(":id", assetId);
      set.exec();
    }

    transaction.commit();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to select media asset {}: {}", assetId, e.what());
    return false;
  }
}

bool SqliteMediaAssetRepository::remove(int assetId) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "DELETE FROM media_assets WHERE id = :id");
    query.bind(":id", assetId);
    query.exec();

    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to remove media asset {}: {}", assetId, e.what());
    return false;
  }
}
} // namespace firelight::metadata
