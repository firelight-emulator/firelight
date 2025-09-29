#include "achievement_service.hpp"

#include "models/achievement.hpp"

#include <cpr/api.h>
#include <cpr/cprtypes.h>
#include <event_dispatcher.hpp>
#include <qcryptographichash.h>
#include <rcheevos/ra_constants.h>
#include <spdlog/spdlog.h>

namespace firelight::achievements {

AchievementService::AchievementService(IAchievementRepository &m_repository)
    : m_repository(m_repository) {}
std::optional<User>
AchievementService::getUser(const std::string &username) const {
  return m_repository.getUser(username);
}
bool AchievementService::createOrUpdateUser(const User &user) {
  return m_repository.createOrUpdateUser(user);
}

std::optional<AchievementSet>
AchievementService::getAchievementSetByContentHash(
    const std::string &contentHash) const {
  return m_repository.getAchievementSetByContentHash(contentHash);
}

bool AchievementService::setGameId(const std::string &contentHash,
                                   const int setId) {
  return m_repository.setGameId(contentHash, setId);
}

std::optional<Achievement>
AchievementService::getAchievement(unsigned achievementId) const {
  return m_repository.getAchievement(achievementId);
}

std::optional<int>
AchievementService::getGameId(const std::string &contentHash) const {
  return m_repository.getGameId(contentHash);
}

bool AchievementService::updateAchievementProgress(
    const AchievementProgress &progress) {
  return m_repository.create(progress);
}

std::optional<UserUnlock>
AchievementService::getUserUnlock(const std::string &username,
                                  const unsigned achievementId) const {
  return m_repository.getUserUnlock(username, achievementId);
}
bool AchievementService::createOrUpdate(const UserUnlock &unlock) {
  if (m_currentSessionHardcore) {
    m_currentSessionHardcoreUnlocks.emplace_back(unlock.achievementId);
  }
  return m_repository.createOrUpdate(unlock);
}

std::vector<UserUnlock>
AchievementService::getAllUserUnlocks(const std::string &username,
                                      unsigned setId) const {
  return m_repository.getAllUserUnlocks(username, setId);
}

std::optional<PatchResponse>
AchievementService::getPatchResponse(const int gameId) const {
  return m_repository.getPatchResponse(gameId);
}

bool AchievementService::processPatchResponse(const std::string &username,
                                              const PatchResponse &response) {
  m_repository.create(response);

  std::vector<Achievement> achievements;
  unsigned totalPoints = 0;
  auto i = 0;
  for (const auto &a : response.PatchData.Achievements) {
    auto achievement = Achievement{.id = a.ID,
                                   .name = a.Title,
                                   .description = a.Description,
                                   .imageUrl = a.BadgeURL,
                                   .points = a.Points,
                                   .type = a.Type,
                                   .displayOrder = i++,
                                   .setId = response.PatchData.ID,
                                   .flags = a.Flags};

    if (a.Flags == 5) {
      continue;
    }

    achievements.emplace_back(achievement);
    totalPoints += a.Points;
  }

  auto set = AchievementSet{.id = response.PatchData.ID,
                            .name = response.PatchData.Title,
                            .iconUrl = response.PatchData.ImageIconURL,
                            .numAchievements =
                                static_cast<unsigned>(achievements.size()),
                            .totalPoints = totalPoints};

  if (!m_repository.create(set)) {
    spdlog::error("Failed to createOrUpdate achievement set: {}", set.id);
    return false;
  }

  for (const auto &achievement : achievements) {
    if (!m_repository.create(achievement)) {
      spdlog::error("Failed to createOrUpdate achievement: {}", achievement.id);
      return false;
    }

    auto unlock = m_repository.getUserUnlock(username, achievement.id);
    if (!unlock.has_value()) {
      auto newUnlock = UserUnlock{.username = username,
                                  .achievementId = achievement.id,
                                  .earned = false,
                                  .earnedHardcore = false,
                                  .unlockTimestamp = 0,
                                  .unlockTimestampHardcore = 0,
                                  .synced = true};

      if (!m_repository.createOrUpdate(newUnlock)) {
        spdlog::error("Failed to createOrUpdate user unlock: {} for user {}",
                      achievement.id, username);
        return false;
      }
    }
  }
  return true;
}
bool AchievementService::processStartSessionResponse(
    const std::string &username, const unsigned setId,
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
        spdlog::error("Failed to createOrUpdate user unlock: {} for user {}",
                      a.ID, username);
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
        spdlog::error("Failed to createOrUpdate user unlock: {} for user {}",
                      a.ID, username);
        return false;
      }
    }
  }

  // TODO: This could cause issues if achievements aren't synced beforehand
  // TODO: Could check to only update those that are synced

  // TODO: Update user score
  for (auto &unlock : m_repository.getAllUserUnlocks(username, setId)) {
    auto foundInUnlocks = std::ranges::find_if(
        startSessionResponse.Unlocks,
        [&unlock](const Unlock &u) { return u.ID == unlock.achievementId; });

    auto foundInHardcoreUnlocks = std::ranges::find_if(
        startSessionResponse.HardcoreUnlocks,
        [&unlock](const Unlock &u) { return u.ID == unlock.achievementId; });

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
      spdlog::error("Failed to update user unlock: {} for user {}",
                    unlock.achievementId, username);
    }
  }

  auto newUnlock = UserUnlock{
      .username = username,
      .achievementId = UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID,
      .earned = foundUnsupportedEmu,
      .earnedHardcore = foundUnsupportedEmu,
      .unlockTimestamp =
          foundUnsupportedEmu ? static_cast<uint64_t>(time(nullptr)) : 0,
      .unlockTimestampHardcore =
          foundUnsupportedEmu ? static_cast<uint64_t>(time(nullptr)) : 0,
      .synced = true};

  if (!m_repository.createOrUpdate(newUnlock)) {
    spdlog::error("Failed to createOrUpdate unsupported achievement user "
                  "unlock: {} for user {}",
                  UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID, username);
  }

  return true;
}

void AchievementService::syncOfflineAchievements() {
  const auto headers =
      cpr::Header{{"User-Agent", OFFLINE_USER_AGENT},
                  {"Content-Type", "application/x-www-form-urlencoded"}};

  for (auto &user : m_repository.listUsers()) {
    auto unsyncedUnlocks =
        m_repository.getAllUnsyncedUserUnlocks(user.username);
    if (unsyncedUnlocks.empty()) {
      continue;
    }

    // Try logging in for current user
    auto postBody = "r=login2&u=" + user.username + "&t=" + user.token;

    const auto response =
        Post(cpr::Url{RA_DOREQUEST_URL}, headers, cpr::Body{postBody});

    if (response.error) {
      spdlog::warn("Failed to log in user: {} ({})", user.username,
                   response.error.message);
      continue;
    }

    auto json = nlohmann::json::parse(response.text);
    if (json.contains("Success") && json["Success"].is_boolean() &&
        json["Success"].get<bool>()) {
      spdlog::info("Logged in for user {}", user.username);
    } else {
      spdlog::error("Login was not successful for user {}", user.username);
      continue;
    }

    spdlog::info("[AchievementService] Syncing {} achievements for user {}",
                 unsyncedUnlocks.size(), user.username);
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

      auto achieveTimestamp = unlock.earnedHardcore
                                  ? unlock.unlockTimestampHardcore
                                  : unlock.unlockTimestamp;
      auto now = std::chrono::duration_cast<std::chrono::seconds>(
                     std::chrono::system_clock::now().time_since_epoch())
                     .count();

      // Calculate hash to send on payload
      auto secondsSinceUnlock = now - achieveTimestamp;
      auto hashContent = std::to_string(unlock.achievementId) + user.username +
                         std::to_string(unlock.earnedHardcore ? 1 : 0) +
                         std::to_string(unlock.achievementId) +
                         std::to_string(secondsSinceUnlock);

      // TODO: Remove qt dependency
      hashContent =
          QCryptographicHash::hash(hashContent, QCryptographicHash::Md5)
              .toHex()
              .toStdString();

      auto achievement = m_repository.getAchievement(unlock.achievementId);
      if (!achievement.has_value()) {
        spdlog::warn("Could not find achievement with ID: {}",
                     unlock.achievementId);
        continue;
      }

      auto gameHash = m_repository.getGameHash(achievement->setId);
      if (!gameHash.has_value()) {
        spdlog::warn("Could not find game hash for achievement set ID: {}",
                     achievement->setId);
        continue;
      }

      auto unlockPostBody =
          "r=awardachievement&u=" + user.username + "&t=" + user.token +
          "&a=" + std::to_string(unlock.achievementId) +
          "&h=" + std::to_string(unlock.earnedHardcore ? 1 : 0) +
          "&m=" + gameHash.value() +
          "&o=" + std::to_string(secondsSinceUnlock) + "&v=" + hashContent;

      const auto unlockResponse =
          Post(cpr::Url{RA_DOREQUEST_URL}, headers, cpr::Body{unlockPostBody});

      if (unlockResponse.error) {
        spdlog::warn("Failed to award achievement; will try later: {}",
                     unlockResponse.error.message);
        continue;
      }

      auto unlockJson = nlohmann::json::parse(unlockResponse.text);

      // If not successful, check error
      if (!unlockJson.contains("Success") ||
          !unlockJson["Success"].is_boolean() ||
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
            spdlog::info("Server already knows user {} has achievement {}",
                         user.username, unlock.achievementId);
          }

        } else {
          spdlog::error("Unsuccessful response did not contain Error flag...");
          continue;
        }
      }

      unlock.synced = true;

      if (unlockJson.contains("Score") && unlockJson["Score"].is_number()) {
        user.hardcore_points = unlockJson["Score"];
      }

      if (unlockJson.contains("SoftcoreScore") &&
          unlockJson["SoftcoreScore"].is_number()) {
        user.points = unlockJson["SoftcoreScore"];
      }

      m_repository.createOrUpdate(unlock);
      spdlog::info("[AchievementService] Synced achievement {} for user {}",
                   unlock.achievementId, user.username);
    }

    m_repository.createOrUpdateUser(user);
  }

  m_currentSessionHardcoreUnlocks.clear();
}

void AchievementService::startSession(const std::string &username,
                                      const unsigned setId,
                                      const bool hardcore) {
  m_inActiveSession = true;
  m_currentSessionUsername = username;
  m_currentSessionSetId = setId;
  m_currentSessionHardcore = hardcore;

  EventDispatcher::instance().publish(AchievementSessionStartedEvent{
      .username = m_currentSessionUsername,
      .setId = m_currentSessionSetId,
      .hardcore = m_currentSessionHardcore,
  });
}

void AchievementService::endSession() {
  m_inActiveSession = false;
  m_currentSessionHardcoreUnlocks.clear();

  const auto event = AchievementSessionEndedEvent{
      .username = m_currentSessionUsername,
      .setId = m_currentSessionSetId,
      .hardcore = m_currentSessionHardcore,
  };

  m_currentSessionUsername.clear();
  m_currentSessionSetId = 0;
  m_currentSessionHardcore = false;

  EventDispatcher::instance().publish(event);
}

bool AchievementService::inHardcoreSession() const {
  return m_inActiveSession && m_currentSessionHardcore;
}

unsigned AchievementService::getNumCurrentSessionHardcoreUnlocks() const {
  return static_cast<unsigned>(m_currentSessionHardcoreUnlocks.size());
}
} // namespace firelight::achievements