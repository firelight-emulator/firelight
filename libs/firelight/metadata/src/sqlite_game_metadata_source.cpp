#include <firelight/metadata/sqlite_game_metadata_source.hpp>
#include <firelight/util/region_codes.hpp>
#include <firelight/util/strings.hpp>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::metadata {

namespace {
// TODO
// The column holds however many genres the row's author felt like writing, on one line
std::vector<std::string> splitGenres(const std::string &value) {
  auto genres = std::vector<std::string>{};

  for (const auto &piece : strings::split(strings::replaceAll(value, ";", ","), ',')) {
    genres.push_back(piece);
  }

  return genres;
}

// TODO
// Mapped to the codes the rest of the app compares against, so a region read here and a region
// read off a filename can be ranked against the same preference
std::vector<std::string> splitRegions(const std::string &value) {
  auto regions = std::vector<std::string>{};

  for (const auto &piece : strings::split(strings::replaceAll(value, ";", ","), ',')) {
    const auto code = regionForTag(piece);
    regions.push_back(code.empty() ? piece : code);
  }

  return regions;
}
} // namespace

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

std::optional<MetadataLookup> SqliteGameMetadataSource::lookup(const std::string &contentHash) {
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
    MetadataLookup lookup;
    lookup.name = query.getColumn(1).getString();
    lookup.metadata.description = query.getColumn(2).getString();
    lookup.metadata.developer = query.getColumn(3).getString();
    lookup.metadata.publisher = query.getColumn(4).getString();
    lookup.metadata.genres = splitGenres(query.getColumn(5).getString());
    lookup.metadata.releaseYear = query.getColumn(6).getUInt();
    lookup.metadata.releaseDate = query.getColumn(7).getString();
    lookup.metadata.regions = splitRegions(query.getColumn(8).getString());
    lookup.metadata.players = query.getColumn(9).getString();
    lookup.retroAchievementsId = query.getColumn(10).getUInt();
    lookup.platformId = query.getColumn(11).getInt();

    SQLite::Statement mediaQuery(*m_db, "SELECT media_type, url FROM game_media WHERE game_id = :gid");
    mediaQuery.bind(":gid", gameId);

    while (mediaQuery.executeStep()) {
      lookup.media.push_back(MediaDefault{.type = static_cast<MediaType>(mediaQuery.getColumn(0).getInt()),
                                          .url = mediaQuery.getColumn(1).getString()});
    }

    return lookup;
  } catch (const std::exception &e) {
    spdlog::error("Metadata lookup failed for {}: {}", contentHash, e.what());
    return std::nullopt;
  }
}
} // namespace firelight::metadata
