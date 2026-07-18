#include <firelight/metadata/sqlite_game_metadata_source.hpp>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::metadata {
SqliteGameMetadataSource::SqliteGameMetadataSource(std::string databaseFile) : m_databaseFile(std::move(databaseFile)) {
  try {
    m_db = std::make_unique<SQLite::Database>(m_databaseFile, SQLite::OPEN_READONLY);
  } catch (const std::exception &e) {
    // It's fine if we don't find it, we just won't have metadata available
    spdlog::warn("Game metadata database unavailable ({}): {}", m_databaseFile, e.what());
    m_db.reset();
  }
}

SqliteGameMetadataSource::~SqliteGameMetadataSource() = default;

std::optional<GameMetadata> SqliteGameMetadataSource::lookup(const std::string &contentHash) {
  if (!m_db) {
    return std::nullopt;
  }
  try {
    SQLite::Statement query(*m_db, "SELECT g.id, g.name, g.description, g.developer, g.publisher, g.genre, "
                                   "g.release_year, g.release_date, g.region, g.players, g.ra_game_id, "
                                   "g.platform_id FROM games g JOIN game_hashes h ON h.game_id = g.id "
                                   "WHERE h.content_hash = :hash LIMIT 1");

    query.bind(":hash", contentHash);
    if (!query.executeStep()) {
      return std::nullopt;
    }

    const int gameId = query.getColumn(0).getInt();
    GameMetadata metadata;
    metadata.name = query.getColumn(1).getString();
    metadata.description = query.getColumn(2).getString();
    metadata.developer = query.getColumn(3).getString();
    metadata.publisher = query.getColumn(4).getString();
    metadata.genre = query.getColumn(5).getString();
    metadata.releaseYear = query.getColumn(6).getUInt();
    metadata.releaseDate = query.getColumn(7).getString();
    metadata.region = query.getColumn(8).getString();
    metadata.players = query.getColumn(9).getString();
    metadata.retroAchievementsId = query.getColumn(10).getUInt();
    metadata.platformId = query.getColumn(11).getInt();

    SQLite::Statement mediaQuery(*m_db, "SELECT media_type, url FROM game_media WHERE game_id = :gid");
    mediaQuery.bind(":gid", gameId);

    while (mediaQuery.executeStep()) {
      metadata.media.push_back(MediaDefault{.type = static_cast<MediaType>(mediaQuery.getColumn(0).getInt()),
                                            .url = mediaQuery.getColumn(1).getString()});
    }

    return metadata;
  } catch (const std::exception &e) {
    spdlog::error("Metadata lookup failed for {}: {}", contentHash, e.what());
    return std::nullopt;
  }
}
} // namespace firelight::metadata
