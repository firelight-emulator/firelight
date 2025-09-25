/**
 * @file sqlite_achievement_repository.cpp
 * @brief Implementation of SQLite-based achievement repository
 */

#include "sqlite_achievement_repository.hpp"

#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::achievements {

/**
 * @brief Constructs the SQLite achievement repository
 *
 * Initializes the database connection and creates all required tables with
 * proper schema and constraints. The database is created if it doesn't exist.
 *
 * Database Schema Overview:
 * - hashes: Maps game content hashes to achievement set IDs for automatic
 * loading
 * - achievement_sets: Stores achievement collection metadata (name, points,
 * etc.)
 * - achievements: Individual achievement data with display order and flags
 * - achievement_progress: User progress tracking (numerator/denominator)
 * - user_unlocks: Achievement completion status with normal/hardcore modes
 * - patch_response_cache: Cached RetroAchievements API responses for offline
 * use
 * - users: User account information and point totals
 *
 * @param dbPath Path to SQLite database file, or ":memory:" for in-memory
 * database
 * @throws std::runtime_error If database initialization fails
 */
SqliteAchievementRepository::SqliteAchievementRepository(std::string dbPath)
    : m_databaseFile(std::move(dbPath)) {
  try {
    // Open database with read/write and createOrUpdate flags
    m_database = std::make_unique<SQLite::Database>(
        m_databaseFile, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

    // Create all required tables with proper schema
    const std::string setupQueryString = R"(
            -- Content hash to achievement set mapping for automatic loading
            CREATE TABLE IF NOT EXISTS hashes (
                hash TEXT NOT NULL,                    -- Game content hash
                achievement_set_id INTEGER NOT NULL,   -- Associated achievement set
                PRIMARY KEY (hash)
            );

            -- Achievement set metadata
            CREATE TABLE IF NOT EXISTS achievement_sets (
                id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,                    -- Achievement set name
                num_achievements INTEGER NOT NULL,     -- Total achievement count
                num_points INTEGER NOT NULL DEFAULT 0, -- Total point value
                icon_url TEXT NOT NULL DEFAULT ''      -- Set icon URL
            );

            -- User account information
            CREATE TABLE IF NOT EXISTS users (
                username TEXT PRIMARY KEY NOT NULL,
                token TEXT NOT NULL,                   -- Authentication token
                points INTEGER NOT NULL,               -- Total points earned
                hardcore_points INTEGER NOT NULL       -- Hardcore mode points
            );

            -- Individual achievement data
            CREATE TABLE IF NOT EXISTS achievements (
                id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,                    -- Achievement name
                description TEXT NOT NULL,             -- Achievement description
                points INTEGER NOT NULL,               -- Point value
                achievement_set_id INTEGER NOT NULL,   -- Parent set ID
                flags INTEGER NOT NULL,                -- Status flags (3 = active)
                type TEXT NOT NULL DEFAULT '',         -- Achievement type/category
                icon_url TEXT NOT NULL DEFAULT '',     -- Achievement icon URL
                display_order INTEGER NOT NULL DEFAULT 0, -- Display sort order
                FOREIGN KEY (achievement_set_id) REFERENCES achievement_sets(id)
            );

            -- User progress tracking for incremental achievements
            CREATE TABLE IF NOT EXISTS achievement_progress (
                achievement_id INTEGER NOT NULL,       -- Achievement being tracked
                username TEXT NOT NULL,                -- User making progress
                numerator INTEGER NOT NULL DEFAULT 0,  -- Current progress value
                denominator INTEGER NOT NULL DEFAULT 0, -- Target/maximum value
                PRIMARY KEY (username, achievement_id),
                FOREIGN KEY (achievement_id) REFERENCES achievements(id)
            );

            -- User achievement completion status
            CREATE TABLE IF NOT EXISTS user_unlocks (
                username TEXT NOT NULL,
                achievement_id INTEGER NOT NULL,
                earned BOOLEAN NOT NULL,               -- Normal mode completion
                earned_hardcore BOOLEAN NOT NULL,      -- Hardcore mode completion
                "when" TIMESTAMP,                      -- Normal unlock timestamp
                when_hardcore TIMESTAMP,               -- Hardcore unlock timestamp
                synced BOOLEAN DEFAULT 0 NOT NULL,     -- Normal sync status
                synced_hardcore BOOLEAN DEFAULT 0 NOT NULL, -- Hardcore sync status
                PRIMARY KEY (username, achievement_id),
                FOREIGN KEY (achievement_id) REFERENCES achievements(id)
            );

            -- Cached API responses for offline functionality
            CREATE TABLE IF NOT EXISTS patch_response_cache (
                game_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
                cached_value BLOB NOT NULL             -- JSON response as binary data
            );
        )";

    m_database->exec(setupQueryString);
  } catch (const std::exception &e) {
    spdlog::error("Database initialization failed: {}", e.what());
    throw std::runtime_error("Failed to initialize achievement database: " +
                             std::string(e.what()));
  }
}

bool SqliteAchievementRepository::create(const AchievementSet achievementSet) {
  try {
    SQLite::Statement query(
        *m_database,
        "INSERT INTO achievement_sets "
        "(id, name, num_achievements, num_points, icon_url) "
        "VALUES (:id, :name, :numAchievements, :numPoints, :iconUrl) "
        "ON CONFLICT(id) DO UPDATE SET "
        "name = excluded.name, "
        "num_achievements = excluded.num_achievements, "
        "num_points = excluded.num_points, "
        "icon_url = excluded.icon_url");
    query.bind(":id", achievementSet.id);
    query.bind(":name", achievementSet.name);
    query.bind(":numAchievements", achievementSet.numAchievements);
    query.bind(":numPoints", achievementSet.totalPoints);
    query.bind(":iconUrl", achievementSet.iconUrl);

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to createOrUpdate achievement set: {}", e.what());
    return false;
  }
}
bool SqliteAchievementRepository::update(const AchievementSet achievementSet) {
  try {
    SQLite::Statement query(*m_database, "UPDATE achievement_sets "
                                         "SET name = :name, "
                                         "num_achievements = :numAchievements, "
                                         "num_points = :numPoints, "
                                         "icon_url = :iconUrl "
                                         "WHERE id = :id");
    query.bind(":id", achievementSet.id);
    query.bind(":name", achievementSet.name);
    query.bind(":numAchievements", achievementSet.numAchievements);
    query.bind(":numPoints", achievementSet.totalPoints);
    query.bind(":iconUrl", achievementSet.iconUrl);

    return query.exec() > 0;
  } catch (const std::exception &e) {
    spdlog::error("Failed to update achievement set: {}", e.what());
    return false;
  }
}

std::optional<AchievementSet>
SqliteAchievementRepository::getAchievementSet(unsigned setId) const {
  try {
    SQLite::Statement query(
        *m_database, "SELECT id, name, num_achievements, num_points, icon_url "
                     "FROM achievement_sets "
                     "WHERE id = :setId");
    query.bind(":setId", setId);

    if (query.executeStep()) {
      AchievementSet achievementSet;
      achievementSet.id = query.getColumn(0);
      achievementSet.name = query.getColumn(1).getString();
      achievementSet.numAchievements = query.getColumn(2);
      achievementSet.totalPoints = query.getColumn(3);
      achievementSet.iconUrl = query.getColumn(4).getString();

      // Load achievements for this set
      SQLite::Statement achievementQuery(
          *m_database,
          "SELECT id, name, description, points, achievement_set_id, flags, "
          "type, icon_url, display_order "
          "FROM achievements "
          "WHERE achievement_set_id = :setId "
          "ORDER BY display_order");
      achievementQuery.bind(":setId", achievementSet.id);

      while (achievementQuery.executeStep()) {
        Achievement achievement;
        achievement.id = achievementQuery.getColumn(0);
        achievement.name = achievementQuery.getColumn(1).getString();
        achievement.description = achievementQuery.getColumn(2).getString();
        achievement.points = achievementQuery.getColumn(3);
        achievement.setId = achievementQuery.getColumn(4);
        achievement.flags = achievementQuery.getColumn(5);
        achievement.type = achievementQuery.getColumn(6).getString();
        achievement.imageUrl = achievementQuery.getColumn(7).getString();
        achievement.displayOrder = achievementQuery.getColumn(8);

        achievementSet.achievements.push_back(achievement);
      }

      return achievementSet;
    }

    return std::nullopt;
  } catch (const std::exception &e) {
    spdlog::error("Failed to get achievement set: {}", e.what());
    return std::nullopt;
  }
}

bool SqliteAchievementRepository::create(const Achievement achievement) {
  try {
    SQLite::Statement query(
        *m_database, "INSERT INTO achievements "
                     "(id, name, description, points, achievement_set_id, "
                     "flags, type, icon_url, display_order) "
                     "VALUES (:id, :name, :description, :points, :setId, "
                     ":flags, :type, :iconUrl, :displayOrder) "
                     "ON CONFLICT(id) DO UPDATE SET "
                     "name = excluded.name, "
                     "description = excluded.description, "
                     "points = excluded.points, "
                     "achievement_set_id = excluded.achievement_set_id, "
                     "flags = excluded.flags, "
                     "type = excluded.type, "
                     "icon_url = excluded.icon_url, "
                     "display_order = excluded.display_order");
    query.bind(":id", achievement.id);
    query.bind(":name", achievement.name);
    query.bind(":description", achievement.description);
    query.bind(":points", achievement.points);
    query.bind(":setId", achievement.setId);
    query.bind(":flags", achievement.flags);
    query.bind(":type", achievement.type);
    query.bind(":iconUrl", achievement.imageUrl);
    query.bind(":displayOrder", achievement.displayOrder);

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to createOrUpdate achievement: {}", e.what());
    return false;
  }
}

bool SqliteAchievementRepository::create(AchievementProgress progress) {
  try {
    SQLite::Statement query(
        *m_database,
        "INSERT INTO achievement_progress "
        "(achievement_id, username, numerator, denominator) "
        "VALUES (:achievementId, :username, :numerator, :denominator) "
        "ON CONFLICT(achievement_id, username) DO UPDATE SET "
        "numerator = excluded.numerator, "
        "denominator = excluded.denominator");
    query.bind(":achievementId", progress.achievementId);
    query.bind(":username", progress.username);
    query.bind(":numerator", progress.numerator);
    query.bind(":denominator", progress.denominator);

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to createOrUpdate achievement progress: {}",
                  e.what());
    return false;
  }
}
bool SqliteAchievementRepository::setGameId(const std::string &contentHash,
                                            int setId) {
  try {
    SQLite::Statement query(*m_database,
                            "INSERT INTO hashes "
                            "(hash, achievement_set_id) "
                            "VALUES (:contentHash, :setId) "
                            "ON CONFLICT(hash) DO UPDATE SET "
                            "achievement_set_id = excluded.achievement_set_id");
    query.bind(":contentHash", contentHash);
    query.bind(":setId", setId);

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to set game hash: {}", e.what());
    return false;
  }
}

std::optional<AchievementSet>
SqliteAchievementRepository::getAchievementSetByContentHash(
    const std::string &contentHash) const {
  try {
    SQLite::Statement query(
        *m_database,
        "SELECT s.id, s.name, s.num_achievements, s.num_points, s.icon_url  "
        "FROM achievement_sets s "
        "JOIN hashes h ON s.id = h.achievement_set_id "
        "WHERE h.hash = :contentHash");
    query.bind(":contentHash", contentHash);

    if (query.executeStep()) {
      AchievementSet achievementSet;
      achievementSet.id = query.getColumn(0);
      achievementSet.name = query.getColumn(1).getString();
      achievementSet.numAchievements = query.getColumn(2);
      achievementSet.totalPoints = query.getColumn(3);
      achievementSet.iconUrl = query.getColumn(4).getString();

      // Load achievements for this set
      SQLite::Statement achievementQuery(
          *m_database,
          "SELECT id, name, description, points, achievement_set_id, flags, "
          "type, icon_url, display_order "
          "FROM achievements "
          "WHERE achievement_set_id = :setId "
          "ORDER BY display_order");
      achievementQuery.bind(":setId", achievementSet.id);

      unsigned totalPoints = 0;
      while (achievementQuery.executeStep()) {
        Achievement achievement;
        achievement.id = achievementQuery.getColumn(0);
        achievement.name = achievementQuery.getColumn(1).getString();
        achievement.description = achievementQuery.getColumn(2).getString();
        achievement.points = achievementQuery.getColumn(3);
        achievement.setId = achievementQuery.getColumn(4);
        achievement.flags = achievementQuery.getColumn(5);
        achievement.type = achievementQuery.getColumn(6).getString();
        achievement.imageUrl = achievementQuery.getColumn(7).getString();
        achievement.displayOrder = achievementQuery.getColumn(8);

        if (achievement.flags == 3) {
          totalPoints += achievement.points;
          achievementSet.achievements.push_back(achievement);
        }
      }

      achievementSet.totalPoints = totalPoints;
      return achievementSet;
    }

    return std::nullopt;
  } catch (const std::exception &e) {
    return std::nullopt;
  }
}

std::optional<UserUnlock>
SqliteAchievementRepository::getUserUnlock(const std::string &username,
                                           const unsigned achievementId) const {
  try {
    SQLite::Statement query(
        *m_database,
        "SELECT username, achievement_id, earned, earned_hardcore, "
        "\"when\", when_hardcore, synced, synced_hardcore "
        "FROM user_unlocks "
        "WHERE username = :username AND achievement_id = :achievementId");
    query.bind(":username", username);
    query.bind(":achievementId", achievementId);

    if (query.executeStep()) {
      UserUnlock unlock;
      unlock.username = query.getColumn(0).getString();
      unlock.achievementId = query.getColumn(1);
      unlock.earned = query.getColumn(2).getInt() != 0;
      unlock.earnedHardcore = query.getColumn(3).getInt() != 0;
      unlock.unlockTimestamp =
          query.getColumn(4).isNull() ? 0 : query.getColumn(4);
      unlock.unlockTimestampHardcore =
          query.getColumn(5).isNull() ? 0 : query.getColumn(5);
      unlock.synced = query.getColumn(6).getInt() != 0;
      unlock.syncedHardcore = query.getColumn(7).getInt() != 0;
      return unlock;
    }

    return std::nullopt;
  } catch (const std::exception &e) {
    return std::nullopt;
  }
}

bool SqliteAchievementRepository::createOrUpdate(const UserUnlock unlock) {
  try {
    SQLite::Statement query(
        *m_database,
        "INSERT INTO user_unlocks "
        "(username, achievement_id, earned, earned_hardcore, \"when\", "
        "when_hardcore, synced, synced_hardcore) "
        "VALUES (:username, :achievementId, :earned, :earnedHardcore, "
        ":unlockTimestamp, :unlockTimestampHardcore, :synced, :syncedHardcore) "
        "ON CONFLICT(username, achievement_id) DO UPDATE SET "
        "earned = excluded.earned, "
        "earned_hardcore = excluded.earned_hardcore, "
        "\"when\" = excluded.\"when\", "
        "when_hardcore = excluded.when_hardcore, "
        "synced = excluded.synced, "
        "synced_hardcore = excluded.synced_hardcore");

    query.bind(":username", unlock.username);
    query.bind(":achievementId", unlock.achievementId);
    query.bind(":earned", unlock.earned);
    query.bind(":earnedHardcore", unlock.earnedHardcore);

    if (unlock.unlockTimestamp != 0) {
      query.bind(":unlockTimestamp",
                 static_cast<int64_t>(unlock.unlockTimestamp));
    } else {
      query.bind(":unlockTimestamp");
    }

    if (unlock.unlockTimestampHardcore != 0) {
      query.bind(":unlockTimestampHardcore",
                 static_cast<int64_t>(unlock.unlockTimestampHardcore));
    } else {
      query.bind(":unlockTimestampHardcore");
    }

    query.bind(":synced", unlock.synced);
    query.bind(":syncedHardcore", unlock.syncedHardcore);

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to createOrUpdate user unlock: {}", e.what());
    return false;
  }
}

std::vector<UserUnlock>
SqliteAchievementRepository::getAllUserUnlocks(const std::string &username,
                                               unsigned setId) const {
  std::vector<UserUnlock> unlocks;

  try {
    SQLite::Statement query(
        *m_database,
        "SELECT u.username, u.achievement_id, u.earned, u.earned_hardcore, "
        "u.\"when\", u.when_hardcore, u.synced, u.synced_hardcore "
        "FROM user_unlocks u "
        "JOIN achievements a ON u.achievement_id = a.id "
        "WHERE u.username = :username AND a.achievement_set_id = :setId "
        "ORDER BY a.display_order");

    query.bind(":username", username);
    query.bind(":setId", setId);

    while (query.executeStep()) {
      UserUnlock unlock;
      unlock.username = query.getColumn(0).getString();
      unlock.achievementId = query.getColumn(1);
      unlock.earned = query.getColumn(2).getInt() != 0;
      unlock.earnedHardcore = query.getColumn(3).getInt() != 0;
      unlock.unlockTimestamp =
          query.getColumn(4).isNull() ? 0 : query.getColumn(4);
      unlock.unlockTimestampHardcore =
          query.getColumn(5).isNull() ? 0 : query.getColumn(5);
      unlock.synced = query.getColumn(6).getInt() != 0;
      unlock.syncedHardcore = query.getColumn(7).getInt() != 0;

      unlocks.push_back(unlock);
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get all user unlocks: {}", e.what());
    // Return empty vector on error
  }

  return unlocks;
}

std::optional<PatchResponse>
SqliteAchievementRepository::getPatchResponse(const int gameId) const {
  try {
    SQLite::Statement query(*m_database,
                            "SELECT cached_value FROM patch_response_cache "
                            "WHERE game_id = :gameId");
    query.bind(":gameId", gameId);

    if (query.executeStep()) {
      const void *blobData = query.getColumn(0).getBlob();
      int blobSize = query.getColumn(0).getBytes();

      std::string jsonString(static_cast<const char *>(blobData), blobSize);
      auto jsonObj = nlohmann::json::parse(jsonString);

      PatchResponse response;
      from_json(jsonObj, response);
      return response;
    }

    return std::nullopt;
  } catch (const std::exception &e) {
    spdlog::error("Failed to get patch response: {}", e.what());
    return std::nullopt;
  }
}
bool SqliteAchievementRepository::create(const PatchResponse patchResponse) {
  try {
    nlohmann::json jsonObj;
    to_json(jsonObj, patchResponse);
    const std::string jsonString = jsonObj.dump();

    SQLite::Statement query(*m_database,
                            "INSERT OR REPLACE INTO patch_response_cache "
                            "(game_id, cached_value) "
                            "VALUES (:gameId, :cachedValue)");
    query.bind(":gameId", patchResponse.PatchData.ID);
    query.bind(":cachedValue", jsonString.c_str(), jsonString.size());

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to createOrUpdate patch response: {}", e.what());
    return false;
  }
}
} // namespace firelight::achievements