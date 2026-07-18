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
 * proper schema and constraints. The database is created if it doesn't exist
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
            -- Content hash to game mapping for automatic loading
            CREATE TABLE IF NOT EXISTS game_hashes (
                hash TEXT NOT NULL PRIMARY KEY,                    -- Game content hash
                game_id INTEGER NOT NULL
            );

            -- Content hash to achievement set mapping for automatic loading
            CREATE TABLE IF NOT EXISTS achievement_set_hashes (
                hash TEXT NOT NULL,                    -- Game content hash
                achievement_set_id INTEGER NOT NULL,   -- Associated set
                PRIMARY KEY (hash, achievement_set_id)
            );

            -- User account information
            CREATE TABLE IF NOT EXISTS users (
                username TEXT PRIMARY KEY NOT NULL,
                display_name TEXT NOT NULL DEFAULT '',    -- User display name
                avatar_url TEXT NOT NULL DEFAULT '',      -- User avatar URL
                token TEXT NOT NULL,                   -- Authentication token
                score INTEGER NOT NULL,               -- Total score earned
                softcore_score INTEGER NOT NULL,       -- Hardcore mode score
                messages INTEGER NOT NULL DEFAULT 0,          -- Unread message count
                permissions INTEGER NOT NULL DEFAULT 0,        -- User permission flags
                account_type TEXT NOT NULL DEFAULT ''
            );

            -- Game metadata
            CREATE TABLE IF NOT EXISTS games (
                id INTEGER NOT NULL PRIMARY KEY,
                title TEXT NOT NULL,                            -- Game name
                image_icon_url TEXT NOT NULL DEFAULT '',        -- Set icon URL
                rich_presence_game_id INTEGER NOT NULL DEFAULT 0,  -- Rich Presence ID
                rich_presence_patch TEXT NOT NULL DEFAULT '',     -- Rich Presence Patch
                console_id INTEGER NOT NULL DEFAULT 0            -- Console ID
            );

            -- Achievement set metadata
            CREATE TABLE IF NOT EXISTS achievement_sets (
                id INTEGER NOT NULL PRIMARY KEY,
                title TEXT NOT NULL,                      -- Achievement set name
                type TEXT NOT NULL,                       -- Achievement set type
                game_id INTEGER NOT NULL,                 -- Associated game ID
                image_icon_url TEXT NOT NULL DEFAULT '',   -- Set icon URL,
                num_achievements INTEGER NOT NULL,        -- Total achievement count
                num_points INTEGER NOT NULL DEFAULT 0,    -- Total point value
                FOREIGN KEY (game_id) REFERENCES games(id)
            );

            -- Individual achievement data
            CREATE TABLE IF NOT EXISTS achievements (
                id INTEGER NOT NULL PRIMARY KEY,
                achievement_set_id INTEGER NOT NULL,   -- Parent set ID
                mem_address TEXT NOT NULL DEFAULT '',     -- Memory address
                title TEXT NOT NULL,                    -- Achievement name
                description TEXT NOT NULL,             -- Achievement description
                points INTEGER NOT NULL,               -- Point value
                author TEXT NOT NULL DEFAULT '',          -- Achievement author
                modified INTEGER NOT NULL DEFAULT 0,        -- Last modified timestamp
                created INTEGER NOT NULL DEFAULT 0,         -- Creation timestamp
                badge_name TEXT NOT NULL DEFAULT '',       -- Badge internal name
                flags INTEGER NOT NULL,                -- Status flags (3 = active)
                type TEXT NOT NULL DEFAULT '',         -- Achievement type/category
                rarity REAL NOT NULL DEFAULT 0.0,        -- Normal mode rarity %
                rarity_hardcore REAL NOT NULL DEFAULT 0.0, -- Hardcore mode rarity %
                badge_url TEXT NOT NULL DEFAULT '',        -- Achievement icon URL
                badge_locked_url TEXT NOT NULL DEFAULT '', -- Locked icon URL
                display_order INTEGER NOT NULL DEFAULT 0, -- Display sort order
                FOREIGN KEY (achievement_set_id) REFERENCES achievement_sets(id)
            );

            -- Achievement set metadata
            CREATE TABLE IF NOT EXISTS leaderboards (
                id INTEGER NOT NULL PRIMARY KEY,
                achievement_set_id INTEGER NOT NULL,                 -- Associated game ID
                title TEXT NOT NULL,                      -- Achievement set name
                description TEXT NOT NULL,                       -- Achievement set type
                mem_address TEXT NOT NULL,                       -- Achievement set type
                lower_is_better INTEGER NOT NULL,                       -- Achievement set type
                format TEXT NOT NULL,                       -- Achievement set type
                hidden INTEGER NOT NULL DEFAULT 0,                       -- Achievement set type
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
                PRIMARY KEY (username, achievement_id),
                FOREIGN KEY (achievement_id) REFERENCES achievements(id)
            );
        )";

    m_database->exec(setupQueryString);
  } catch (const std::exception &e) {
    spdlog::error("Database initialization failed: {}", e.what());
    throw std::runtime_error("Failed to initialize achievement database: " +
                             std::string(e.what()));
  }
}
/**
 * @brief Retrieves user data by username from the SQLite database
 *
 * Queries the users table to fetch complete user information including
 * authentication token and point totals for both normal and hardcore modes
 *
 * @param username The username to query for
 * @return User data if found, std::nullopt if not found or on database error
 */
std::optional<User>
SqliteAchievementRepository::getUser(const std::string &username) const {
  try {
    SQLite::Statement query(
        *m_database,
        "SELECT username, display_name, token, softcore_score, score, "
        "avatar_url, messages, permissions, account_type "
        "FROM users "
        "WHERE username = :username");
    query.bind(":username", username);

    if (query.executeStep()) {
      User user;
      user.username = query.getColumn(0).getString();
      user.displayName = query.getColumn(1).getString();
      user.token = query.getColumn(2).getString();
      user.softcoreScore = query.getColumn(3);
      user.score = query.getColumn(4);
      user.avatarUrl = query.getColumn(5).getString();
      user.messages = query.getColumn(6);
      user.permissions = query.getColumn(7);
      user.accountType = query.getColumn(8).getString();
      return user;
    }

    return std::nullopt;
  } catch (const std::exception &e) {
    spdlog::error("Failed to get user: {}", e.what());
    return std::nullopt;
  }
}
std::vector<User> SqliteAchievementRepository::listUsers() const {
  std::vector<User> users;

  try {
    SQLite::Statement query(
        *m_database,
        "SELECT username, display_name, token, softcore_score, score, "
        "avatar_url, messages, permissions, account_type "
        "FROM users "
        "ORDER BY username");

    while (query.executeStep()) {
      User user;
      user.username = query.getColumn(0).getString();
      user.displayName = query.getColumn(1).getString();
      user.token = query.getColumn(2).getString();
      user.softcoreScore = query.getColumn(3);
      user.score = query.getColumn(4);
      user.avatarUrl = query.getColumn(5).getString();
      user.messages = query.getColumn(6);
      user.permissions = query.getColumn(7);
      user.accountType = query.getColumn(8).getString();
      users.emplace_back(user);
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to list users: {}", e.what());
    // Return empty vector on error
  }

  return users;
}
/**
 * @brief Creates or updates user data in the SQLite database
 *
 * Inserts new user data or updates existing user information including
 * authentication token and point totals. Uses SQLite's ON CONFLICT DO UPDATE
 * for upsert semantics based on the username primary key
 *
 * @param user The user data to store or update
 * @return true if the operation succeeded, false on database error
 */
bool SqliteAchievementRepository::createOrUpdateUser(const User &user) {
  try {
    SQLite::Statement query(*m_database,
                            "INSERT INTO users "
                            "(username, display_name, token, softcore_score, "
                            "score, avatar_url, messages, "
                            "permissions, account_type) "
                            "VALUES (:username, :displayName, :token, "
                            ":softcoreScore, :score, :avatarUrl, "
                            ":messages, :permissions, :accountType) "
                            "ON CONFLICT(username) DO UPDATE SET "
                            "display_name = excluded.display_name, "
                            "token = excluded.token, "
                            "softcore_score = excluded.softcore_score, "
                            "score = excluded.score, "
                            "avatar_url = excluded.avatar_url, "
                            "messages = excluded.messages, "
                            "permissions = excluded.permissions, "
                            "account_type = excluded.account_type");

    query.bind(":username", user.username);
    // Intentionally set display_name to username on creation/update
    query.bind(":displayName", user.username);
    query.bind(":token", user.token);
    query.bind(":softcoreScore", user.softcoreScore);
    query.bind(":score", user.score);
    query.bind(":avatarUrl", user.avatarUrl);
    query.bind(":messages", user.messages);
    query.bind(":permissions", user.permissions);
    query.bind(":accountType", user.accountType);

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to createOrUpdate user: {}", e.what());
    return false;
  }
}

bool SqliteAchievementRepository::create(const AchievementSet achievementSet) {
  try {
    SQLite::Statement query(*m_database,
                            "INSERT INTO achievement_sets "
                            "(id, title, type, game_id, image_icon_url, "
                            "num_achievements, num_points) "
                            "VALUES (:id, :title, :type, :gameId, "
                            ":imageIconUrl, :numAchievements, :numPoints) "
                            "ON CONFLICT(id) DO UPDATE SET "
                            "title = excluded.title, "
                            "type = excluded.type, "
                            "game_id = excluded.game_id, "
                            "image_icon_url = excluded.image_icon_url, "
                            "num_achievements = excluded.num_achievements, "
                            "num_points = excluded.num_points");
    query.bind(":id", achievementSet.id);
    query.bind(":title", achievementSet.title);
    query.bind(":type", achievementSet.type);
    query.bind(":gameId", achievementSet.gameId);
    query.bind(":imageIconUrl", achievementSet.imageIconUrl);
    query.bind(":numAchievements", achievementSet.numAchievements);
    query.bind(":numPoints", achievementSet.totalPoints);

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to createOrUpdate achievement set: {}", e.what());
    return false;
  }
}

std::vector<AchievementSet>
SqliteAchievementRepository::getAchievementSetsByGameId(unsigned gameId) const {
  try {
    SQLite::Statement query(*m_database,
                            "SELECT id, title, type, game_id, image_icon_url "
                            "FROM achievement_sets "
                            "WHERE game_id = :gameId");
    query.bind(":gameId", gameId);

    std::vector<AchievementSet> sets;
    while (query.executeStep()) {
      AchievementSet achievementSet;
      achievementSet.id = query.getColumn(0);
      achievementSet.title = query.getColumn(1).getString();
      achievementSet.type = query.getColumn(2).getString();
      achievementSet.gameId = query.getColumn(3);
      achievementSet.imageIconUrl = query.getColumn(4).getString();

      // Load achievements for this set
      SQLite::Statement achievementQuery(
          *m_database, "SELECT id, achievement_set_id, mem_address, title, "
                       "description, points, "
                       "author, modified, created, badge_name, flags, type, "
                       "rarity, rarity_hardcore, "
                       "badge_url, badge_locked_url, display_order "
                       "FROM achievements "
                       "WHERE achievement_set_id = :setId "
                       "ORDER BY display_order");
      achievementQuery.bind(":setId", achievementSet.id);

      auto totalPoints = 0;
      auto numAchievements = 0;
      while (achievementQuery.executeStep()) {
        Achievement achievement;
        achievement.id = achievementQuery.getColumn(0);
        achievement.achievementSetId = achievementQuery.getColumn(1);
        achievement.memAddr = achievementQuery.getColumn(2).getString();
        achievement.title = achievementQuery.getColumn(3).getString();
        achievement.description = achievementQuery.getColumn(4).getString();
        achievement.points = achievementQuery.getColumn(5);
        achievement.author = achievementQuery.getColumn(6).getString();
        achievement.modified = achievementQuery.getColumn(7);
        achievement.created = achievementQuery.getColumn(8);
        achievement.badgeName = achievementQuery.getColumn(9).getString();
        achievement.flags = achievementQuery.getColumn(10);
        achievement.type = achievementQuery.getColumn(11).getString();
        achievement.rarity = achievementQuery.getColumn(12).getDouble();
        achievement.rarityHardcore = achievementQuery.getColumn(13).getDouble();
        achievement.badgeUrl = achievementQuery.getColumn(14).getString();
        achievement.badgeLockedUrl = achievementQuery.getColumn(15).getString();
        achievement.displayOrder = achievementQuery.getColumn(16);

        if (achievement.flags != 3) {
          continue; // Only include active achievements
        }

        numAchievements++;
        totalPoints += achievement.points;
        achievementSet.achievements.emplace_back(achievement);
      }

      achievementSet.totalPoints = totalPoints;
      achievementSet.numAchievements = numAchievements;
      sets.emplace_back(achievementSet);
    }

    return sets;

  } catch (const std::exception &e) {
    spdlog::error("Failed to get achievement set: {}", e.what());
    return {};
  }
}

bool SqliteAchievementRepository::create(const Achievement achievement) {
  try {
    SQLite::Statement query(
        *m_database,
        "INSERT INTO achievements "
        "(id, achievement_set_id, mem_address, title, description, points, "
        "author, modified, created, badge_name, flags, type, rarity, "
        "rarity_hardcore, badge_url, badge_locked_url, display_order) "
        "VALUES (:id, :achievementSetId, :memAddr, :title, :description, "
        ":points, "
        ":author, :modified, :created, :badgeName, :flags, :type, :rarity, "
        ":rarityHardcore, :badgeUrl, :badgeLockedUrl, :displayOrder) "
        "ON CONFLICT(id) DO UPDATE SET "
        "achievement_set_id = excluded.achievement_set_id, "
        "mem_address = excluded.mem_address, "
        "title = excluded.title, "
        "description = excluded.description, "
        "points = excluded.points, "
        "author = excluded.author, "
        "modified = excluded.modified, "
        "created = excluded.created, "
        "badge_name = excluded.badge_name, "
        "flags = excluded.flags, "
        "type = excluded.type, "
        "rarity = excluded.rarity, "
        "rarity_hardcore = excluded.rarity_hardcore, "
        "badge_url = excluded.badge_url, "
        "badge_locked_url = excluded.badge_locked_url, "
        "display_order = excluded.display_order");
    query.bind(":id", achievement.id);
    query.bind(":achievementSetId", achievement.achievementSetId);
    query.bind(":memAddr", achievement.memAddr);
    query.bind(":title", achievement.title);
    query.bind(":description", achievement.description);
    query.bind(":points", achievement.points);
    query.bind(":author", achievement.author);
    query.bind(":modified", achievement.modified);
    query.bind(":created", achievement.created);
    query.bind(":badgeName", achievement.badgeName);
    query.bind(":flags", achievement.flags);
    query.bind(":type", achievement.type);
    query.bind(":rarity", achievement.rarity);
    query.bind(":rarityHardcore", achievement.rarityHardcore);
    query.bind(":badgeUrl", achievement.badgeUrl);
    query.bind(":badgeLockedUrl", achievement.badgeLockedUrl);
    query.bind(":displayOrder", achievement.displayOrder);

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to createOrUpdate achievement: {}", e.what());
    return false;
  }
}
/**
 * @brief Retrieves an individual achievement by its ID from the SQLite database
 *
 * Queries the achievements table to fetch complete achievement information
 * including title, description, points, type, display order, flags, and parent
 * achievement set ID. Used for displaying achievement details or validating
 * individual achievement data
 *
 * @param achievementId The unique ID of the achievement to retrieve
 * @return Achievement data if found, std::nullopt if not found or on database
 * error
 */
std::optional<Achievement>
SqliteAchievementRepository::getAchievement(unsigned achievementId) const {
  try {
    SQLite::Statement query(
        *m_database,
        "SELECT id, achievement_set_id, mem_address, title, description, "
        "points, "
        "author, modified, created, badge_name, flags, type, rarity, "
        "rarity_hardcore, badge_url, badge_locked_url, display_order "
        "FROM achievements "
        "WHERE id = :achievementId");
    query.bind(":achievementId", achievementId);

    if (query.executeStep()) {
      Achievement achievement;
      achievement.id = query.getColumn(0);
      achievement.achievementSetId = query.getColumn(1);
      achievement.memAddr = query.getColumn(2).getString();
      achievement.title = query.getColumn(3).getString();
      achievement.description = query.getColumn(4).getString();
      achievement.points = query.getColumn(5);
      achievement.author = query.getColumn(6).getString();
      achievement.modified = query.getColumn(7);
      achievement.created = query.getColumn(8);
      achievement.badgeName = query.getColumn(9).getString();
      achievement.flags = query.getColumn(10);
      achievement.type = query.getColumn(11).getString();
      achievement.rarity = query.getColumn(12).getDouble();
      ;
      achievement.rarityHardcore = query.getColumn(13).getDouble();
      ;
      achievement.badgeUrl = query.getColumn(14).getString();
      achievement.badgeLockedUrl = query.getColumn(15).getString();
      achievement.displayOrder = query.getColumn(16);
      return achievement;
    }

    return std::nullopt;
  } catch (const std::exception &e) {
    spdlog::error("Failed to get achievement: {}", e.what());
    return std::nullopt;
  }
}

bool SqliteAchievementRepository::create(const AchievementProgress progress) {
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

std::optional<Game>
SqliteAchievementRepository::getGameById(const int gameId) const {
  try {
    SQLite::Statement query(
        *m_database, "SELECT id, title, image_icon_url, rich_presence_game_id, "
                     "rich_presence_patch, console_id "
                     "FROM games "
                     "WHERE id = :gameId");
    query.bind(":gameId", gameId);

    if (query.executeStep()) {
      Game game;
      game.id = query.getColumn(0);
      game.title = query.getColumn(1).getString();
      game.imageIconUrl = query.getColumn(2).getString();
      game.richPresenceGameId = query.getColumn(3);
      game.richPresencePatch = query.getColumn(4).getString();
      game.consoleId = query.getColumn(5);
      game.achievementSets = getAchievementSetsByGameId(game.id);

      return game;
    }

    return std::nullopt;
  } catch (const std::exception &e) {
    spdlog::error("Failed to get game by id: {}", e.what());
    return std::nullopt;
  }
}

bool SqliteAchievementRepository::create(const Game game) {
  try {
    SQLite::Statement query(
        *m_database, "INSERT INTO games "
                     "(id, title, image_icon_url, rich_presence_game_id, "
                     "rich_presence_patch, console_id) "
                     "VALUES (:id, :title, :imageIconUrl, :richPresenceGameId, "
                     ":richPresencePatch, :consoleId) "
                     "ON CONFLICT(id) DO UPDATE SET "
                     "title = excluded.title, "
                     "image_icon_url = excluded.image_icon_url, "
                     "rich_presence_game_id = excluded.rich_presence_game_id, "
                     "rich_presence_patch = excluded.rich_presence_patch, "
                     "console_id = excluded.console_id");
    query.bind(":id", game.id);
    query.bind(":title", game.title);
    query.bind(":imageIconUrl", game.imageIconUrl);
    query.bind(":richPresenceGameId", game.richPresenceGameId);
    query.bind(":richPresencePatch", game.richPresencePatch);
    query.bind(":consoleId", game.consoleId);

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to createOrUpdate game: {}", e.what());
    return false;
  }
}

bool SqliteAchievementRepository::setGameId(const std::string &contentHash,
                                            const int gameId) {
  try {
    SQLite::Statement query(*m_database, "INSERT INTO game_hashes "
                                         "(hash, game_id) "
                                         "VALUES (:contentHash, :gameId) "
                                         "ON CONFLICT(hash) DO UPDATE SET "
                                         "game_id = excluded.game_id");
    query.bind(":contentHash", contentHash);
    query.bind(":gameId", gameId);

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to set game hash: {}", e.what());
    return false;
  }
}

bool SqliteAchievementRepository::setAchievementSetHash(
    const unsigned achievementSetId, const std::string &contentHash) {
  try {
    SQLite::Statement query(*m_database,
                            "INSERT INTO achievement_set_hashes "
                            "(hash, achievement_set_id) "
                            "VALUES (:contentHash, :achievementSetId) "
                            "ON CONFLICT(hash, achievement_set_id) DO NOTHING");
    query.bind(":contentHash", contentHash);
    query.bind(":achievementSetId", achievementSetId);

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to set achievement set hash: {}", e.what());
    return false;
  }
}

std::optional<AchievementSet>
SqliteAchievementRepository::getAchievementSetByContentHash(
    const std::string &contentHash) const {
  try {
    SQLite::Statement query(
        *m_database,
        "SELECT s.id, s.title, s.type, s.game_id, "
        "s.image_icon_url, s.num_achievements, s.num_points "
        "FROM achievement_sets s "
        "JOIN achievement_set_hashes h ON s.id = h.achievement_set_id "
        "WHERE h.hash = :contentHash");
    query.bind(":contentHash", contentHash);

    if (query.executeStep()) {
      AchievementSet achievementSet;
      achievementSet.id = query.getColumn(0);
      achievementSet.title = query.getColumn(1).getString();
      achievementSet.type = query.getColumn(2).getString();
      achievementSet.gameId = query.getColumn(3);
      achievementSet.imageIconUrl = query.getColumn(4).getString();
      achievementSet.numAchievements = query.getColumn(5);
      achievementSet.totalPoints = query.getColumn(6);

      // Load achievements for this set
      SQLite::Statement achievementQuery(
          *m_database, "SELECT id, achievement_set_id, mem_address, title, "
                       "description, points, "
                       "author, modified, created, badge_name, flags, type, "
                       "rarity, rarity_hardcore, "
                       "badge_url, badge_locked_url, display_order "
                       "FROM achievements "
                       "WHERE achievement_set_id = :gameId "
                       "ORDER BY display_order");
      achievementQuery.bind(":gameId", achievementSet.id);

      unsigned numAchievements = 0;
      unsigned totalPoints = 0;
      while (achievementQuery.executeStep()) {
        numAchievements++;

        Achievement achievement;
        achievement.id = achievementQuery.getColumn(0);
        achievement.achievementSetId = achievementQuery.getColumn(1);
        achievement.memAddr = achievementQuery.getColumn(2).getString();
        achievement.title = achievementQuery.getColumn(3).getString();
        achievement.description = achievementQuery.getColumn(4).getString();
        achievement.points = achievementQuery.getColumn(5);
        achievement.author = achievementQuery.getColumn(6).getString();
        achievement.modified = achievementQuery.getColumn(7);
        achievement.created = achievementQuery.getColumn(8);
        achievement.badgeName = achievementQuery.getColumn(9).getString();
        achievement.flags = achievementQuery.getColumn(10);
        achievement.type = achievementQuery.getColumn(11).getString();
        achievement.rarity = achievementQuery.getColumn(12).getDouble();
        achievement.rarityHardcore = achievementQuery.getColumn(13).getDouble();
        achievement.badgeUrl = achievementQuery.getColumn(14).getString();
        achievement.badgeLockedUrl = achievementQuery.getColumn(15).getString();
        achievement.displayOrder = achievementQuery.getColumn(16);

        if (achievement.flags == 3) {
          totalPoints += achievement.points;
          achievementSet.achievements.emplace_back(achievement);
        }
      }

      achievementSet.numAchievements = numAchievements;
      achievementSet.totalPoints = totalPoints;
      return achievementSet;
    }

    return std::nullopt;
  } catch (const std::exception &e) {
    return std::nullopt;
  }
}
/**
 * @brief Retrieves the achievement set ID associated with a content hash from
 * the SQLite database
 *
 * Queries the hashes table to find the achievement set ID that has been mapped
 * to the specified content hash. This enables reverse lookups from content hash
 * to achievement set ID
 *
 * @param contentHash The content hash to look up in the database
 * @return The achievement set ID if found, std::nullopt if not found or on
 * database error
 */
std::optional<int>
SqliteAchievementRepository::getGameId(const std::string &contentHash) const {
  try {
    SQLite::Statement query(*m_database,
                            "SELECT game_id FROM game_hashes "
                            "WHERE hash = :contentHash AND game_id != 0");
    query.bind(":contentHash", contentHash);

    if (query.executeStep()) {
      return query.getColumn(0).getInt();
    }

    return std::nullopt;
  } catch (const std::exception &e) {
    spdlog::error("Failed to get game ID: {}", e.what());
    return std::nullopt;
  }
}
std::optional<std::string>
SqliteAchievementRepository::getGameHash(const unsigned gameId) const {
  try {
    SQLite::Statement query(*m_database, "SELECT hash FROM game_hashes "
                                         "WHERE game_id = :gameId");
    query.bind(":gameId", gameId);

    if (query.executeStep()) {
      return query.getColumn(0).getString();
    }

    return std::nullopt;
  } catch (const std::exception &e) {
    spdlog::error("Failed to get game hash: {}", e.what());
    return std::nullopt;
  }
}
bool SqliteAchievementRepository::create(const Leaderboard &leaderboard) {
  try {
    SQLite::Statement query(
        *m_database,
        "INSERT INTO leaderboards "
        "(id, achievement_set_id, title, description, mem_address, "
        "lower_is_better, format, hidden) "
        "VALUES (:id, :achievementSetId, :title, :description, :memAddress, "
        ":lowerIsBetter, :format, :hidden) "
        "ON CONFLICT(id) DO UPDATE SET "
        "achievement_set_id = excluded.achievement_set_id, "
        "title = excluded.title, "
        "description = excluded.description, "
        "mem_address = excluded.mem_address, "
        "lower_is_better = excluded.lower_is_better, "
        "format = excluded.format, "
        "hidden = excluded.hidden");
    query.bind(":id", leaderboard.id);
    query.bind(":achievementSetId", leaderboard.achievementSetId);
    query.bind(":title", leaderboard.title);
    query.bind(":description", leaderboard.description);
    query.bind(":memAddress", leaderboard.memAddr);
    query.bind(":lowerIsBetter", leaderboard.lowerIsBetter);
    query.bind(":format", leaderboard.format);
    query.bind(":hidden", leaderboard.hidden);

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to create leaderboard: {}", e.what());
    return false;
  }
}

std::optional<UserUnlock>
SqliteAchievementRepository::getUserUnlock(const std::string &username,
                                           const unsigned achievementId) {
  try {
    SQLite::Statement query(
        *m_database,
        "SELECT username, achievement_id, earned, earned_hardcore, "
        "\"when\", when_hardcore, synced "
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
      return unlock;
    }

    UserUnlock newUnlock{
        .username = username,
        .achievementId = achievementId,
        .earned = false,
        .earnedHardcore = false,
        .unlockTimestamp = 0,
        .unlockTimestampHardcore = 0,
        .synced = true,
    };
    createOrUpdate(newUnlock);

    return {newUnlock};
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
        "when_hardcore, synced) "
        "VALUES (:username, :achievementId, :earned, :earnedHardcore, "
        ":unlockTimestamp, :unlockTimestampHardcore, :synced ) "
        "ON CONFLICT(username, achievement_id) DO UPDATE SET "
        "earned = excluded.earned, "
        "earned_hardcore = excluded.earned_hardcore, "
        "\"when\" = excluded.\"when\", "
        "when_hardcore = excluded.when_hardcore, "
        "synced = excluded.synced");

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

    query.exec();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to createOrUpdate user unlock: {}", e.what());
    return false;
  }
}

std::vector<UserUnlock>
SqliteAchievementRepository::getAllUserUnlocks(const std::string &username,
                                               const unsigned gameId) const {
  std::vector<UserUnlock> unlocks;

  try {
    SQLite::Statement query(
        *m_database,
        "SELECT u.username, u.achievement_id, u.earned, u.earned_hardcore, "
        "u.\"when\", u.when_hardcore, u.synced "
        "FROM user_unlocks u "
        "JOIN achievements a ON u.achievement_id = a.id "
        "JOIN achievement_sets s ON a.achievement_set_id = s.id "
        "WHERE u.username = :username AND s.game_id = :gameId "
        "ORDER BY a.display_order");

    query.bind(":username", username);
    query.bind(":gameId", gameId);

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

      unlocks.emplace_back(unlock);
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get all user unlocks: {}", e.what());
    // Return empty vector on error
  }

  return unlocks;
}
std::vector<UserUnlock> SqliteAchievementRepository::getAllUnsyncedUserUnlocks(
    const std::string &username) const {
  std::vector<UserUnlock> unlocks;

  try {
    SQLite::Statement query(
        *m_database,
        "SELECT username, achievement_id, earned, earned_hardcore, "
        "\"when\", when_hardcore, synced FROM user_unlocks WHERE username = "
        ":username AND synced = 0");

    query.bind(":username", username);

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

      unlocks.emplace_back(unlock);
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to get all user unlocks: {}", e.what());
    // Return empty vector on error
  }

  return unlocks;
}
} // namespace firelight::achievements
