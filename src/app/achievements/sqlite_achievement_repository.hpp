#pragma once
#include "SQLiteCpp/Database.h"
#include "achievement_repository.hpp"

namespace firelight::achievements {

/**
 * @brief SQLite-based implementation of the achievement repository interface
 *
 * Provides persistent storage for achievement data using SQLite database
 * backend. Supports both file-based and in-memory database configurations for
 * production use and testing respectively.
 *
 * Database Schema:
 * - achievement_sets: Game achievement collections with metadata
 * - achievements: Individual achievements with display and completion data
 * - achievement_progress: User progress tracking for incremental achievements
 * - hashes: Content hash to achievement set mappings for automatic loading
 * - user_unlocks: User achievement completion status and sync state
 * - patch_response_cache: Cached RetroAchievements API responses for offline
 * use
 *
 * All write operations use upsert semantics (INSERT ... ON CONFLICT DO UPDATE)
 * to handle both creation and modification scenarios gracefully.
 */
class SqliteAchievementRepository final : public IAchievementRepository {
public:
  /**
   * @brief Constructs the repository and initializes the database
   *
   * Creates or opens the SQLite database at the specified path and sets up
   * all required tables with proper schema and constraints. Use ":memory:"
   * for in-memory database suitable for testing.
   *
   * @param dbPath Path to the SQLite database file, or ":memory:" for in-memory
   * DB
   * @throws std::runtime_error if database initialization fails
   */
  explicit SqliteAchievementRepository(std::string dbPath);

  /**
   * @brief Virtual destructor for proper cleanup
   */
  virtual ~SqliteAchievementRepository() = default;

  // Achievement Set Operations

  /**
   * @brief Creates or updates an achievement set in the database
   *
   * Stores achievement set metadata including name, icon URL, point totals,
   * and achievement counts. Uses upsert semantics.
   *
   * @param achievementSet The achievement set data to store
   * @return true if the operation succeeded, false on database error
   */
  bool create(AchievementSet achievementSet) override;

  /**
   * @brief Updates an existing achievement set's metadata
   *
   * Modifies the stored data for an achievement set that must already exist.
   * Does not affect associated achievements.
   *
   * @param achievementSet The achievement set with updated data
   * @return true if update succeeded, false if set doesn't exist or database
   * error
   */
  bool update(AchievementSet achievementSet) override;

  /**
   * @brief Retrieves a complete achievement set by ID
   *
   * Fetches the achievement set metadata and all associated achievements,
   * ordered by display order. Populates the achievements vector with
   * complete achievement data including all metadata fields.
   *
   * @param setId The unique identifier of the achievement set
   * @return The complete achievement set if found, std::nullopt otherwise
   */
  std::optional<AchievementSet>
  getAchievementSet(unsigned setId) const override;

  /**
   * @brief Retrieves achievement set by game content hash
   *
   * Looks up an achievement set using the game's content hash mapping.
   * Only includes achievements with active flags (flags == 3) and calculates
   * total points from included achievements. Used for automatic achievement
   * loading when games are launched.
   *
   * @param contentHash The game's content fingerprint hash
   * @return The achievement set if mapping exists, std::nullopt otherwise
   */
  [[nodiscard]] std::optional<AchievementSet>
  getAchievementSetByContentHash(const std::string &contentHash) const override;

  // Individual Achievement Operations

  /**
   * @brief Creates or updates an individual achievement
   *
   * Stores complete achievement data including name, description, points,
   * icon URL, type, display order, flags, and set association. Uses upsert
   * semantics for both creation and updates.
   *
   * @param achievement The achievement data to store
   * @return true if the operation succeeded, false on database error
   */
  bool create(Achievement achievement) override;

  // Achievement Progress Operations

  /**
   * @brief Creates or updates user progress for an achievement
   *
   * Tracks incremental progress (numerator/denominator) for achievements
   * that require multiple steps or accumulated actions. Uses composite
   * primary key of username and achievement ID for upsert behavior.
   *
   * @param progress The progress data to store or update
   * @return true if the operation succeeded, false on database error
   */
  bool create(AchievementProgress progress) override;

  // Content Hash Mapping Operations

  /**
   * @brief Associates a content hash with an achievement set
   *
   * Creates or updates a mapping between a game's content hash and its
   * achievement set. This enables automatic achievement loading based on
   * game content fingerprinting. Uses upsert semantics to allow remapping.
   *
   * @param contentHash The game's content hash
   * @param setId The achievement set ID to associate
   * @return true if the mapping was stored successfully, false on database
   * error
   */
  bool setGameId(const std::string &contentHash, int setId) override;

  // User Unlock Operations

  /**
   * @brief Retrieves user unlock status for a specific achievement
   *
   * Fetches complete unlock data including normal and hardcore mode status,
   * unlock timestamps, and synchronization status with remote services.
   *
   * @param username The username to query unlock status for
   * @param achievementId The achievement ID to check
   * @return The unlock data if found, std::nullopt if no unlock record exists
   */
  [[nodiscard]] std::optional<UserUnlock>
  getUserUnlock(const std::string &username,
                unsigned achievementId) const override;

  /**
   * @brief Creates or updates user unlock status for an achievement
   *
   * Records achievement completion status for both normal and hardcore modes,
   * including unlock timestamps and sync status with remote services. Uses
   * upsert semantics based on the combination of username and achievement ID.
   *
   * @param unlock The unlock data to store or update
   * @return true if the operation succeeded, false on database error
   */
  bool createOrUpdate(UserUnlock unlock) override;

  /**
   * @brief Retrieves all user unlocks for a specific achievement set
   *
   * Fetches all achievement unlock records for a given user within a specific
   * achievement set, ordered by the achievement's display order. This is useful
   * for displaying a user's progress through all achievements in a game.
   *
   * @param username The username to query unlock records for
   * @param setId The achievement set ID to filter unlocks by
   * @return Vector of UserUnlock records, empty if none found or on error
   */
  std::vector<UserUnlock> getAllUserUnlocks(const std::string &username,
                                            unsigned setId) const override;

  // Patch Response Caching Operations

  /**
   * @brief Retrieves cached patch response data for offline use
   *
   * Fetches previously stored RetroAchievements API patch response data,
   * enabling achievement functionality without network connectivity.
   *
   * @param gameId The game ID to retrieve cached data for
   * @return The cached patch response if available, std::nullopt otherwise
   */
  std::optional<PatchResponse> getPatchResponse(int gameId) const override;

  /**
   * @brief Caches RetroAchievements patch response data
   *
   * Stores complete patch response JSON data in binary format for later
   * offline retrieval. Enables achievement functionality without requiring
   * constant network connectivity.
   *
   * @param patchResponse The patch response data to cache
   * @return true if caching succeeded, false on database error
   */
  bool create(PatchResponse patchResponse) override;

private:
  /** @brief Path to the SQLite database file */
  std::string m_databaseFile;

  /** @brief SQLite database connection handle */
  std::unique_ptr<SQLite::Database> m_database;
};

} // namespace firelight::achievements