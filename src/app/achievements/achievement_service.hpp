#pragma once

#include "achievement_repository.hpp"
#include "models/achievement_progress.hpp"
#include "models/user_unlock.hpp"

#include <optional>
#include <rcheevos/patch_response.hpp>
#include <rcheevos/startsession_response.hpp>
#include <rcheevos/user.h>

namespace firelight::achievements {

class AchievementService {
public:
  explicit AchievementService(IAchievementRepository &m_repository);

  /**
   * @brief Retrieves user data by username
   *
   * Fetches complete user information including authentication token and
   * point totals for both normal and hardcore modes. Delegates to the
   * underlying repository implementation.
   *
   * @param username The username to retrieve data for
   * @return User data if found, std::nullopt otherwise
   */
  std::optional<User> getUser(const std::string &username) const;

  /**
   * @brief Creates or updates user data
   *
   * Stores or updates user information including authentication token and
   * point totals. Uses upsert semantics - if the user already exists, their
   * data will be updated with the new values. Delegates to the underlying
   * repository implementation.
   *
   * @param user The user data to store or update
   * @return true if the operation succeeded, false otherwise
   */
  bool createOrUpdateUser(const User &user);

  [[nodiscard]] std::optional<AchievementSet>
  getAchievementSetByContentHash(const std::string &contentHash) const;

  bool setGameId(const std::string &contentHash, int setId);

  std::optional<Achievement> getAchievement(unsigned achievementId) const;

  /**
   * @brief Retrieves the achievement set ID associated with a content hash
   *
   * Looks up the achievement set ID that has been mapped to a specific game
   * content hash. This is the reverse operation of setGameId and enables
   * querying which achievement set is associated with a given game. Delegates
   * to the underlying repository implementation.
   *
   * @param contentHash The content hash to look up
   * @return The achievement set ID if found, std::nullopt otherwise
   */
  std::optional<int> getGameId(const std::string &contentHash) const;

  bool updateAchievementProgress(const AchievementProgress &progress);

  [[nodiscard]] std::optional<UserUnlock>
  getUserUnlock(const std::string &username, unsigned achievementId) const;

  bool createOrUpdate(const UserUnlock &unlock);

  std::vector<UserUnlock> getAllUserUnlocks(const std::string &username,
                                            unsigned setId) const;

  std::optional<PatchResponse> getPatchResponse(int gameId) const;
  bool processPatchResponse(const std::string &username,
                            const PatchResponse &response);

  bool
  processStartSessionResponse(const std::string &username, unsigned setId,
                              const StartSessionResponse &startSessionResponse);

private:
  IAchievementRepository &m_repository;
};

} // namespace firelight::achievements
