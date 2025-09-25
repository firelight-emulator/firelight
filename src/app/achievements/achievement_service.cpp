#include "achievement_service.hpp"

#include "models/achievement.hpp"

#include <rcheevos/ra_constants.h>
#include <spdlog/spdlog.h>

namespace firelight::achievements {

AchievementService::AchievementService(IAchievementRepository &m_repository)
    : m_repository(m_repository) {}

std::optional<AchievementSet>
AchievementService::getAchievementSetByContentHash(
    const std::string &contentHash) const {
  return m_repository.getAchievementSetByContentHash(contentHash);
}

bool AchievementService::setGameId(const std::string &contentHash,
                                   const int setId) {
  return m_repository.setGameId(contentHash, setId);
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
                                  .synced = true,
                                  .syncedHardcore = true};

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
      continue;
    }

    auto unlock = m_repository.getUserUnlock(username, a.ID);
    if (!unlock) {
      auto newUnlock =
          UserUnlock{.username = username,
                     .achievementId = a.ID,
                     .earned = true,
                     .earnedHardcore = false,
                     .unlockTimestamp = static_cast<uint64_t>(a.When),
                     .unlockTimestampHardcore = 0,
                     .synced = true,
                     .syncedHardcore = true};

      if (!m_repository.createOrUpdate(newUnlock)) {
        spdlog::error("Failed to createOrUpdate user unlock: {} for user {}",
                      a.ID, username);
        return false;
      }

      continue;
    }

    unlock->earned = true;
    unlock->unlockTimestamp = static_cast<uint64_t>(a.When);
    unlock->synced = true;

    if (!m_repository.createOrUpdate(*unlock)) {
      spdlog::error("Failed to update user unlock: {} for user {}", a.ID,
                    username);
      return false;
    }
  }

  // Hardcore unlocks
  for (const auto &a : startSessionResponse.HardcoreUnlocks) {
    if (a.ID == UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID) {
      foundUnsupportedEmu = true;
      continue;
    }

    auto unlock = m_repository.getUserUnlock(username, a.ID);
    if (!unlock) {
      auto newUnlock =
          UserUnlock{.username = username,
                     .achievementId = a.ID,
                     .earned = false,
                     .earnedHardcore = true,
                     .unlockTimestamp = 0,
                     .unlockTimestampHardcore = static_cast<uint64_t>(a.When),
                     .synced = true,
                     .syncedHardcore = true};

      if (!m_repository.createOrUpdate(newUnlock)) {
        spdlog::error("Failed to createOrUpdate user unlock: {} for user {}",
                      a.ID, username);
        return false;
      }

      continue;
    }

    unlock->earnedHardcore = true;
    unlock->unlockTimestampHardcore = static_cast<uint64_t>(a.When);
    unlock->syncedHardcore = true;

    if (!m_repository.createOrUpdate(*unlock)) {
      spdlog::error("Failed to update user unlock: {} for user {}", a.ID,
                    username);
      return false;
    }
  }

  // TODO: This could cause issues if achievements aren't synced beforehand

  // TODO: Update user score
  // Re-locks, in case user cleared in the RA site
  for (auto &a : m_repository.getAllUserUnlocks(username, setId)) {
    // If the achievement is not in the startSessionResponse, mark it as NOT
    // unlocked
    if (std::ranges::find_if(startSessionResponse.Unlocks,
                             [&a](const Unlock &u) {
                               return u.ID == a.achievementId;
                             }) == startSessionResponse.Unlocks.end()) {

      a.earned = false;
      a.unlockTimestamp = 0;
      a.synced = true;

      if (!m_repository.createOrUpdate(a)) {
        spdlog::error("Failed to update user unlock: {} for user {}",
                      a.achievementId, username);
      }
    }

    if (std::ranges::find_if(startSessionResponse.HardcoreUnlocks,
                             [&a](const Unlock &u) {
                               return u.ID == a.achievementId;
                             }) == startSessionResponse.HardcoreUnlocks.end()) {

      a.earnedHardcore = false;
      a.unlockTimestampHardcore = 0;
      a.syncedHardcore = true;

      if (!m_repository.createOrUpdate(a)) {
        spdlog::error("Failed to update user unlock: {} for user {}",
                      a.achievementId, username);
      }
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
      .synced = true,
      .syncedHardcore = true};

  if (!m_repository.createOrUpdate(newUnlock)) {
    spdlog::error("Failed to createOrUpdate unsupported achievement user "
                  "unlock: {} for user {}",
                  UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID, username);
  }

  return true;
}
} // namespace firelight::achievements