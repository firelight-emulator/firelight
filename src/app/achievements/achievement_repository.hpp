#pragma once

#include "models/achievement_progress.hpp"
#include "models/achievement_set.hpp"
#include "models/user_unlock.hpp"

#include <optional>
#include <rcheevos/patch_response.hpp>
#include <rcheevos/user.h>

namespace firelight::achievements {

/**
 * @brief Abstract interface for achievement repository operations
 *
 * Defines the contract for persistent storage and retrieval of
 * achievement-related data including achievement sets, individual achievements,
 * user progress, and content hash mappings. This interface supports both local
 * offline storage and synchronization with remote services.
 */
class IAchievementRepository {
protected:
  /**
   * @brief Protected destructor to enforce interface-only usage
   */
  ~IAchievementRepository() = default;

public:
  // User Operations

  /**
   * @brief Retrieves user data by username
   *
   * Fetches complete user information including authentication token and
   * point totals for both normal and hardcore modes. Used for session
   * validation and progress tracking.
   *
   * @param username The username to retrieve data for
   * @return User data if found, std::nullopt otherwise
   */
  [[nodiscard]] virtual std::optional<User> getUser(const std::string &username) const = 0;

  /**
   * @brief Retrieves all users from the repository
   *
   * Fetches a complete list of all users stored in the repository, including
   * their usernames, authentication tokens, and point totals for both normal
   * and hardcore modes. The results are ordered alphabetically by username
   * for consistent presentation.
   *
   * This method is useful for:
   * - User management interfaces
   * - Administrative operations
   * - User selection and switching
   * - Statistical analysis across users
   * - Debugging and system monitoring
   *
   * @return Vector of User objects containing all users in the repository,
   *         ordered alphabetically by username. Returns empty vector if no
   *         users exist or on database error.
   *
   * @note Database errors are handled gracefully - an empty vector is returned
   *       instead of throwing exceptions
   * @note All user data including tokens are included in the results, so handle
   *       with appropriate security considerations
   * @note The ordering by username is case-sensitive according to the database
   *       collation
   *
   * @see getUser() to retrieve a specific user by username
   * @see createOrUpdateUser() to add or modify user data
   */
  virtual std::vector<User> listUsers() const = 0;

  /**
   * @brief Creates or updates user data
   *
   * Stores or updates user information including authentication token and
   * point totals. Uses upsert semantics - if the user already exists, their
   * data will be updated with the new values.
   *
   * @param user The user data to store or update
   * @return true if the operation succeeded, false otherwise
   */
  virtual bool createOrUpdateUser(const User &user) = 0;

  // Achievement Set Operations

  /**
   * @brief Creates or updates an achievement set
   *
   * Stores an achievement set with its metadata. Uses upsert semantics - if an
   * achievement set with the same ID already exists, it will be updated with
   * the new data.
   *
   * @param achievementSet The achievement set to create or update
   * @return true if the operation succeeded, false otherwise
   */
  virtual bool create(AchievementSet achievementSet) = 0;

  /**
   * @brief Updates an existing achievement set
   *
   * Modifies the metadata of an existing achievement set. The achievement set
   * must already exist in the repository.
   *
   * @param achievementSet The achievement set with updated data
   * @return true if the update succeeded, false if the set doesn't exist or
   * update failed
   */
  virtual bool update(AchievementSet achievementSet) = 0;

  /**
   * @brief Retrieves an achievement set by its unique identifier
   *
   * Fetches a complete achievement set including all associated achievements
   * ordered by their display order. Only achievements with active flags are
   * included.
   *
   * @param setId The unique identifier of the achievement set
   * @return The achievement set if found, std::nullopt otherwise
   */
  [[nodiscard]] virtual std::optional<AchievementSet>
  getAchievementSet(unsigned setId) const = 0;

  /**
   * @brief Retrieves an achievement set by content hash
   *
   * Looks up an achievement set associated with a specific game content hash.
   * This enables automatic achievement loading when a game is launched based on
   * its content fingerprint. Only achievements with active flags (flags == 3)
   * are included.
   *
   * @param contentHash The content hash of the game
   * @return The achievement set if found, std::nullopt otherwise
   */
  [[nodiscard]] virtual std::optional<AchievementSet>
  getAchievementSetByContentHash(const std::string &contentHash) const = 0;

  /**
   * @brief Retrieves the achievement set ID associated with a content hash
   *
   * Looks up the achievement set ID that has been mapped to a specific game
   * content hash. This is the reverse operation of setGameId and enables
   * querying which achievement set is associated with a given game.
   *
   * @param contentHash The content hash to look up
   * @return The achievement set ID if found, std::nullopt otherwise
   */
  [[nodiscard]] virtual std::optional<int>
  getGameId(const std::string &contentHash) const = 0;

  /**
   * @brief Retrieves the content hash associated with an achievement set ID
   *
   * Performs the reverse lookup of getGameId - given an achievement set ID,
   * returns the content hash that has been mapped to it. This enables
   * bidirectional navigation between achievement sets and their associated
   * game content hashes.
   *
   * This method is useful for:
   * - Verifying which content hash is associated with a loaded achievement set
   * - Debugging hash mapping configurations
   * - Content verification and validation workflows
   * - Implementing cache invalidation based on content changes
   * - Supporting multi-hash scenarios where games may have multiple content variants
   *
   * @param setId The achievement set ID to look up the associated content hash for
   * @return The content hash string if a mapping exists, std::nullopt if no
   *         mapping is found or on database error
   *
   * @note This method performs the inverse operation of getGameId() - they form
   *       a bidirectional lookup pair
   * @note If an achievement set has no content hash mapping, std::nullopt is returned
   * @note Database errors are handled gracefully with std::nullopt return values
   * @note The returned hash string may be empty if an empty hash was explicitly mapped
   *
   * @see getGameId() for the forward lookup (hash -> set ID)
   * @see setGameId() to create or update hash mappings
   * @see getAchievementSetByContentHash() to retrieve achievement sets by hash
   */
  [[nodiscard]] virtual std::optional<std::string> getGameHash(unsigned setId) const = 0;

  // Individual Achievement Operations

  /**
   * @brief Creates or updates an individual achievement
   *
   * Stores an achievement with all its metadata including name, description,
   * points, icon URL, type, display order, and flags. Uses upsert semantics.
   *
   * @param achievement The achievement to create or update
   * @return true if the operation succeeded, false otherwise
   */
  virtual bool create(Achievement achievement) = 0;

  /**
   * @brief Retrieves an individual achievement by its unique ID
   *
   * Fetches complete achievement information including name, description,
   * points, type, display order, flags, and parent achievement set ID.
   * Used for displaying individual achievement details or validating
   * achievement data independently of the parent set.
   *
   * @param achievementId The unique ID of the achievement to retrieve
   * @return Achievement data if found, std::nullopt otherwise
   */
  [[nodiscard]] virtual std::optional<Achievement>
  getAchievement(unsigned achievementId) const = 0;

  // Achievement Progress Operations

  /**
   * @brief Creates or updates user progress for an achievement
   *
   * Tracks incremental progress toward achievement completion for a specific
   * user. Uses upsert semantics based on the combination of username and
   * achievement ID.
   *
   * @param progress The progress data to store or update
   * @return true if the operation succeeded, false otherwise
   */
  virtual bool create(AchievementProgress progress) = 0;

  // Content Hash Mapping Operations

  /**
   * @brief Associates a content hash with an achievement set
   *
   * Creates a mapping between a game's content hash and its achievement set ID.
   * This enables automatic achievement loading when games are launched. Uses
   * upsert semantics - if the hash already exists, it will be remapped to the
   * new set ID.
   *
   * @param contentHash The game's content hash
   * @param setId The achievement set ID to associate with the hash
   * @return true if the mapping was created successfully, false otherwise
   */
  virtual bool setGameId(const std::string &contentHash, int setId) = 0;

  // User Unlock Operations

  /**
   * @brief Retrieves user unlock data for a specific achievement
   *
   * Gets the unlock status and metadata for a user's achievement, including
   * both normal and hardcore mode completion status, timestamps, and sync
   * status.
   *
   * @param username The username to query
   * @param achievementId The achievement ID to query
   * @return The unlock data if found, std::nullopt otherwise
   */
  [[nodiscard]] virtual std::optional<UserUnlock>
  getUserUnlock(const std::string &username, unsigned achievementId) const = 0;

  /**
   * @brief Creates or updates user unlock status for an achievement
   *
   * Records achievement completion status for both normal and hardcore modes,
   * including unlock timestamps and sync status with remote services. Uses
   * upsert semantics based on the combination of username and achievement ID.
   *
   * @param unlock The unlock data to store or update
   * @return true if the operation succeeded, false otherwise
   */
  virtual bool createOrUpdate(UserUnlock unlock) = 0;

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
  [[nodiscard]] virtual std::vector<UserUnlock> getAllUserUnlocks(const std::string &username,
                                                    unsigned setId) const = 0;

  /**
   * @brief Retrieves all unsynced user unlock records for a specific user
   *
   * Fetches all achievement unlock records for a given user that have not yet
   * been synchronized with remote services (synced = false). This is essential
   * for offline achievement tracking, allowing the system to identify which
   * unlocks need to be uploaded when connectivity is restored.
   *
   * The method returns unlocks across all achievement sets for the specified
   * user, regardless of the achievement set they belong to. This provides a
   * comprehensive view of all pending synchronization work for a user.
   *
   * @param username The username to query unsynced unlock records for
   * @return Vector of UserUnlock records with synced=false, empty if none found
   *         or on error
   *
   * @note This method does not filter by achievement set - it returns all
   *       unsynced unlocks for the user across all games/sets
   * @note The returned unlocks include both normal and hardcore mode completion
   *       data with their respective timestamps
   */
  [[nodiscard]] virtual std::vector<UserUnlock>
  getAllUnsyncedUserUnlocks(const std::string &username) const = 0;

  // Patch Response Caching Operations

  /**
   * @brief Caches a patch response from RetroAchievements API
   *
   * Stores the complete patch response data for offline use, enabling
   * achievement functionality when not connected to the RetroAchievements
   * service.
   *
   * @param patchResponse The patch response data to cache
   * @return true if caching succeeded, false otherwise
   */
  virtual bool create(PatchResponse patchResponse) = 0;

  /**
   * @brief Retrieves cached patch response data for a game
   *
   * Fetches previously cached patch response data, allowing offline achievement
   * functionality without requiring a network connection.
   *
   * @param gameId The game ID to retrieve patch data for
   * @return The cached patch response if found, std::nullopt otherwise
   */
  [[nodiscard]] virtual std::optional<PatchResponse> getPatchResponse(int gameId) const = 0;
};

} // namespace firelight::achievements