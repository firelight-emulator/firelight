#include "../include/firelight/achievement_service.hpp"

#include "../include/firelight/achievement.hpp"

#include <firelight/event_dispatcher.hpp>

#include <cpr/api.h>
#include <cpr/cprtypes.h>
#include <qcryptographichash.h>
#include <rcheevos/ra_constants.h>
#include <spdlog/spdlog.h>

namespace firelight::achievements {

AchievementService::AchievementService(IAchievementRepository &m_repository) : m_repository(m_repository) {}

std::optional<User> AchievementService::getUser(const std::string &username) const {
  return m_repository.getUser(username);
}

bool AchievementService::create(const User &user) { return m_repository.createOrUpdateUser(user); }

std::optional<AchievementSet> AchievementService::getAchievementSetByContentHash(const std::string &contentHash) const {
  return m_repository.getAchievementSetByContentHash(contentHash);
}

bool AchievementService::setGameId(const std::string &contentHash, const int gameId) {
  return m_repository.setGameId(contentHash, gameId);
}

bool AchievementService::setAchievementSetHash(const unsigned achievementSetId, const std::string &contentHash) {
  return m_repository.setAchievementSetHash(achievementSetId, contentHash);
}

std::optional<Achievement> AchievementService::getAchievement(const unsigned achievementId) const {
  return m_repository.getAchievement(achievementId);
}

// TODO: Need a pass to make the interface smaller here
bool AchievementService::create(const Game &game) { return m_repository.create(game); }

bool AchievementService::create(const AchievementSet &achievementSet) { return m_repository.create(achievementSet); }

bool AchievementService::create(const Achievement &achievement) { return m_repository.create(achievement); }

bool AchievementService::create(const Leaderboard &leaderboard) { return m_repository.create(leaderboard); }

std::optional<int> AchievementService::getGameId(const std::string &contentHash) const {
  return m_repository.getGameId(contentHash);
}

std::optional<Game> AchievementService::getGameForHash(const std::string &contentHash) const {
  const auto id = m_repository.getGameId(contentHash);
  if (!id.has_value()) {
    return std::nullopt;
  }

  return m_repository.getGameById(*id);
}

bool AchievementService::create(const AchievementProgress &progress) { return m_repository.create(progress); }

std::optional<UserUnlock> AchievementService::getUserUnlock(const std::string &username,
                                                            const unsigned achievementId) const {
  return m_repository.getUserUnlock(username, achievementId);
}

bool AchievementService::create(const UserUnlock &unlock) {
  if (m_currentSessionHardcore) {
    m_currentSessionHardcoreUnlocks.emplace_back(unlock.achievementId);
  }
  return m_repository.createOrUpdate(unlock);
}

std::vector<UserUnlock> AchievementService::getAllUserUnlocks(const std::string &username, unsigned gameId) const {
  return m_repository.getAllUserUnlocks(username, gameId);
}

std::pair<int, int> AchievementService::getAchievementCounts(const std::string &contentHash,
                                                             const std::string &username) const {
  const auto set = m_repository.getAchievementSetByContentHash(contentHash);
  if (!set.has_value()) {
    return {0, 0};
  }
  int total = static_cast<int>(set->numAchievements);
  if (total == 0) {
    total = static_cast<int>(set->achievements.size());
  }
  if (username.empty() || total == 0) {
    return {0, total};
  }
  // getAllUserUnlocks filters by achievement_sets.game_id (the RA game id),
  // not the set's primary key — so pass gameId, not id
  int earned = 0;
  for (const auto &unlock : m_repository.getAllUserUnlocks(username, set->gameId)) {
    if (unlock.earned || unlock.earnedHardcore) {
      earned++;
    }
  }
  return {earned, total};
}

std::vector<AchievementSet> AchievementService::getAchievementSetsForContentHash(const std::string &contentHash) const {
  const auto set = m_repository.getAchievementSetByContentHash(contentHash);
  if (!set.has_value()) {
    return {};
  }
  return m_repository.getAchievementSetsByGameId(set->gameId);
}

bool AchievementService::processStartSessionResponse(const std::string &username, const unsigned gameId,
                                                     const StartSessionResponse &startSessionResponse) {
  auto foundUnsupportedEmu = false;

  // Non-hardcore unlocks
  for (const auto &a : startSessionResponse.Unlocks) {
    if (a.ID == UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID) {
      foundUnsupportedEmu = true;
      break;
    }

    auto unlock = m_repository.getUserUnlock(username, a.ID);
    if (!unlock.has_value()) {
      auto newUnlock = UserUnlock{.username = username,
                                  .achievementId = a.ID,
                                  .earned = true,
                                  .earnedHardcore = false,
                                  .unlockTimestamp = a.When,
                                  .unlockTimestampHardcore = 0,
                                  .synced = true};
      if (!m_repository.createOrUpdate(newUnlock)) {
        spdlog::error("Failed to create user unlock: {} for user {}", a.ID, username);
        return false;
      }
    }
  }

  // Hardcore unlocks
  for (const auto &a : startSessionResponse.HardcoreUnlocks) {
    if (a.ID == UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID) {
      foundUnsupportedEmu = true;
      break;
    }

    auto unlock = m_repository.getUserUnlock(username, a.ID);
    if (!unlock.has_value()) {
      auto newUnlock = UserUnlock{.username = username,
                                  .achievementId = a.ID,
                                  .earned = true,
                                  .earnedHardcore = true,
                                  .unlockTimestamp = a.When,
                                  .unlockTimestampHardcore = a.When,
                                  .synced = true};
      if (!m_repository.createOrUpdate(newUnlock)) {
        spdlog::error("Failed to create user unlock: {} for user {}", a.ID, username);
        return false;
      }
    }
  }

  // TODO: Update user score
  for (auto &unlock : m_repository.getAllUserUnlocks(username, gameId)) {
    auto foundInUnlocks = std::ranges::find_if(startSessionResponse.Unlocks,
                                               [&unlock](const Unlock &u) { return u.ID == unlock.achievementId; });

    auto foundInHardcoreUnlocks = std::ranges::find_if(
        startSessionResponse.HardcoreUnlocks, [&unlock](const Unlock &u) { return u.ID == unlock.achievementId; });

    if (foundInHardcoreUnlocks != startSessionResponse.HardcoreUnlocks.end()) {
      // Hardcore unlock means both earned and earnedHardcore are true
      unlock.earned = true;
      unlock.earnedHardcore = true;
      unlock.unlockTimestampHardcore = foundInHardcoreUnlocks->When;

      // Set non-hardcore timestamp if not already set, or use existing
      // non-hardcore unlock time
      if (foundInUnlocks != startSessionResponse.Unlocks.end()) {
        unlock.unlockTimestamp = foundInUnlocks->When;
      } else if (unlock.unlockTimestamp == 0) {
        unlock.unlockTimestamp = foundInHardcoreUnlocks->When;
      }
    } else if (foundInUnlocks != startSessionResponse.Unlocks.end()) {
      // Non-hardcore only unlock
      unlock.earned = true;
      unlock.earnedHardcore = false;
      unlock.unlockTimestamp = foundInUnlocks->When;
      unlock.unlockTimestampHardcore = 0;
    } else {
      unlock.earned = false;
      unlock.earnedHardcore = false;
      unlock.unlockTimestamp = 0;
      unlock.unlockTimestampHardcore = 0;
    }

    unlock.synced = true;
    if (!m_repository.createOrUpdate(unlock)) {
      spdlog::error("Failed to update user unlock: {} for user {}", unlock.achievementId, username);
    }
  }

  auto newUnlock = UserUnlock{.username = username,
                              .achievementId = UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID,
                              .earned = foundUnsupportedEmu,
                              .earnedHardcore = foundUnsupportedEmu,
                              .unlockTimestamp = foundUnsupportedEmu ? static_cast<uint64_t>(time(nullptr)) : 0,
                              .unlockTimestampHardcore = foundUnsupportedEmu ? static_cast<uint64_t>(time(nullptr)) : 0,
                              .synced = true};

  if (!m_repository.createOrUpdate(newUnlock)) {
    spdlog::error("Failed to create unsupported achievement user "
                  "unlock: {} for user {}",
                  UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID, username);
  }

  return true;
}

void AchievementService::syncOfflineAchievements() {
  // TODO:
  // Need to probably add rate limiting
  // Need to store the hash with the user unlocks to get accurate one when
  // sending offline award request

  const auto headers =
      cpr::Header{{"User-Agent", OFFLINE_USER_AGENT}, {"Content-Type", "application/x-www-form-urlencoded"}};

  for (auto &user : m_repository.listUsers()) {
    auto unsyncedUnlocks = m_repository.getAllUnsyncedUserUnlocks(user.username);
    if (unsyncedUnlocks.empty()) {
      continue;
    }

    // Try logging in for current user
    auto postBody = "r=login2&u=" + user.username + "&t=" + user.token;

    const auto response = Post(cpr::Url{RA_DOREQUEST_URL}, headers, cpr::Body{postBody});

    if (response.error) {
      spdlog::warn("Failed to log in user: {} ({})", user.username, response.error.message);
      continue;
    }

    auto json = nlohmann::json::parse(response.text);
    if (json.contains("Success") && json["Success"].is_boolean() && json["Success"].get<bool>()) {
      spdlog::info("Logged in for user {}", user.username);
    } else {
      spdlog::error("Login was not successful for user {}", user.username);
      continue;
    }

    spdlog::info("[AchievementService] Syncing {} achievements for user {}", unsyncedUnlocks.size(), user.username);
    // Go through their unlocks
    for (auto &unlock : unsyncedUnlocks) {
      if (unlock.earnedHardcore) {
        auto shouldDemote = true;
        for (const auto sessionUnlockIds : m_currentSessionHardcoreUnlocks) {
          if (sessionUnlockIds == unlock.achievementId) {
            shouldDemote = false;
            break;
          }
        }

        if (shouldDemote) {
          spdlog::info("User {}'s achievement {} was demoted from hardcore to "
                       "non-hardcore",
                       user.username, unlock.achievementId);
          unlock.earnedHardcore = false;
          unlock.unlockTimestampHardcore = 0;
        }
      }

      auto achieveTimestamp = unlock.earnedHardcore ? unlock.unlockTimestampHardcore : unlock.unlockTimestamp;
      auto now =
          std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

      // Calculate hash to send on payload
      auto secondsSinceUnlock = now - achieveTimestamp;
      auto hashContent = std::to_string(unlock.achievementId) + user.username +
                         std::to_string(unlock.earnedHardcore ? 1 : 0) + std::to_string(unlock.achievementId) +
                         std::to_string(secondsSinceUnlock);

      // TODO: Remove qt dependency
      hashContent = QCryptographicHash::hash(hashContent, QCryptographicHash::Md5).toHex().toStdString();

      auto achievement = m_repository.getAchievement(unlock.achievementId);
      if (!achievement.has_value()) {
        spdlog::warn("Could not find achievement with ID: {}", unlock.achievementId);
        continue;
      }

      auto gameHash = m_repository.getGameHash(achievement->achievementSetId);
      if (!gameHash.has_value()) {
        spdlog::warn("Could not find game hash for achievement set ID: {}", achievement->achievementSetId);
        continue;
      }

      auto unlockPostBody = "r=awardachievement&u=" + user.username + "&t=" + user.token +
                            "&a=" + std::to_string(unlock.achievementId) +
                            "&h=" + std::to_string(unlock.earnedHardcore ? 1 : 0) + "&m=" + gameHash.value() +
                            "&o=" + std::to_string(secondsSinceUnlock) + "&v=" + hashContent;

      const auto unlockResponse = Post(cpr::Url{RA_DOREQUEST_URL}, headers, cpr::Body{unlockPostBody});

      if (unlockResponse.error) {
        spdlog::warn("Failed to award achievement; will try later: {}", unlockResponse.error.message);
        continue;
      }

      auto unlockJson = nlohmann::json::parse(unlockResponse.text);

      // If not successful, check error
      if (!unlockJson.contains("Success") || !unlockJson["Success"].is_boolean() ||
          !unlockJson["Success"].get<bool>()) {
        if (unlockJson.contains("Error") && unlockJson["Error"].is_string()) {
          auto errorString = unlockJson["Error"].get<std::string>();
          if (errorString.find("already has") == std::string::npos) {
            spdlog::warn("Got error: {}", errorString);
            unlock.earned = false;
            unlock.earnedHardcore = false;
            unlock.unlockTimestamp = 0;
            unlock.unlockTimestampHardcore = 0;
          } else {
            spdlog::info("Server already knows user {} has achievement {}", user.username, unlock.achievementId);
          }

        } else {
          spdlog::error("Unsuccessful response did not contain Error flag...");
          continue;
        }
      }

      unlock.synced = true;

      if (unlockJson.contains("Score") && unlockJson["Score"].is_number()) {
        user.score = unlockJson["Score"];
      }

      if (unlockJson.contains("SoftcoreScore") && unlockJson["SoftcoreScore"].is_number()) {
        user.softcoreScore = unlockJson["SoftcoreScore"];
      }

      m_repository.createOrUpdate(unlock);
      spdlog::info("[AchievementService] Synced achievement {} for user {}", unlock.achievementId, user.username);
    }

    m_repository.createOrUpdateUser(user);
  }

  m_currentSessionHardcoreUnlocks.clear();
}

void AchievementService::startSession(const std::string &username, const unsigned gameId, const bool hardcore) {
  m_inActiveSession = true;
  m_currentSessionUsername = username;
  m_currentSessionGameId = gameId;
  m_currentSessionHardcore = hardcore;

  EventDispatcher::instance().publish(AchievementSessionStartedEvent{
      .username = m_currentSessionUsername,
      .gameId = m_currentSessionGameId,
      .hardcore = m_currentSessionHardcore,
  });
}

void AchievementService::endSession() {
  m_inActiveSession = false;
  m_currentSessionHardcoreUnlocks.clear();

  const auto event = AchievementSessionEndedEvent{
      .username = m_currentSessionUsername,
      .gameId = m_currentSessionGameId,
      .hardcore = m_currentSessionHardcore,
  };

  m_currentSessionUsername.clear();
  m_currentSessionGameId = 0;
  m_currentSessionHardcore = false;

  EventDispatcher::instance().publish(event);
}

bool AchievementService::inHardcoreSession() const { return m_inActiveSession && m_currentSessionHardcore; }

unsigned AchievementService::getNumCurrentSessionHardcoreUnlocks() const {
  return static_cast<unsigned>(m_currentSessionHardcoreUnlocks.size());
}

void AchievementService::setLoggedInUsername(const std::string &username) {
  if (m_loggedInUsername == username) {
    return;
  }
  m_loggedInUsername = username;
  EventDispatcher::instance().publish(UserLoggedInEvent{.username = username});
}

std::string AchievementService::getLoggedInUsername() const { return m_loggedInUsername; }
} // namespace firelight::achievements
