#pragma once

#include "achievement_progress.hpp"
#include "achievement_repository.hpp"
#include "game.hpp"
#include "rcheevos/startsession_response.hpp"
#include "user_unlock.hpp"
#include <optional>

namespace firelight::achievements {

struct AchievementSessionStartedEvent {
  std::string username;
  unsigned gameId;
  bool hardcore;
};

struct AchievementSessionEndedEvent {
  std::string username;
  unsigned gameId;
  bool hardcore;
};

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
  [[nodiscard]] std::optional<User> getUser(const std::string &username) const;

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
  bool create(const User &user);

  [[nodiscard]] std::optional<AchievementSet>
  getAchievementSetByContentHash(const std::string &contentHash) const;

  bool setGameId(const std::string &contentHash, int gameId);

  bool setAchievementSetHash(unsigned achievementSetId,
                             const std::string &contentHash);

  [[nodiscard]] std::optional<Achievement>
  getAchievement(unsigned achievementId) const;

  bool create(const Game &game);
  bool create(const AchievementSet &achievementSet);
  bool create(const Achievement &achievement);
  bool create(const Leaderboard &leaderboard);
  bool create(const AchievementProgress &progress);
  bool create(const UserUnlock &unlock);

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
  [[nodiscard]] std::optional<int>
  getGameId(const std::string &contentHash) const;


  std::optional<Game> getGameForHash(const std::string &contentHash) const;

  [[nodiscard]] std::optional<UserUnlock>
  getUserUnlock(const std::string &username, unsigned achievementId) const;

  [[nodiscard]] std::vector<UserUnlock>
  getAllUserUnlocks(const std::string &username, unsigned gameId) const;

  bool
  processStartSessionResponse(const std::string &username, unsigned gameId,
                              const StartSessionResponse &startSessionResponse);

  /**
   * Attempts to send an unlock request for every unsynced achievement unlock.
   */
  void syncOfflineAchievements();

  void startSession(const std::string &username, unsigned gameId,
                    bool hardcore);
  void endSession();

  /**
   * @return True if there's an active hardcore session, otherwise false.
   */
  [[nodiscard]] bool inHardcoreSession() const;

  /**
   *
   * @return The number of achievements the user has unlocked in hardcore mode
   *   during the current session.
   */
  [[nodiscard]] unsigned getNumCurrentSessionHardcoreUnlocks() const;

  void setLoggedInUsername(const std::string &username);
  [[nodiscard]] std::string getLoggedInUsername() const;

private:
  IAchievementRepository &m_repository;

  std::string m_loggedInUsername;

  bool m_inActiveSession = false;
  std::string m_currentSessionUsername;
  unsigned m_currentSessionGameId = 0;
  bool m_currentSessionHardcore = false;

  std::vector<unsigned> m_currentSessionHardcoreUnlocks;
};

} // namespace firelight::achievements
