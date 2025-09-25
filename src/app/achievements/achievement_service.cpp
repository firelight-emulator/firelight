#include "achievement_service.hpp"

#include "models/achievement.hpp"

#include <spdlog/spdlog.h>

namespace firelight::achievements {

AchievementService::AchievementService(IAchievementRepository &m_repository)
    : m_repository(m_repository) {}

std::optional<AchievementSet>
AchievementService::getAchievementSetByContentHash(
    const std::string &contentHash) const {
  return m_repository.getAchievementSetByContentHash(contentHash);
}

bool AchievementService::setGameId(const int setId,
                                   const std::string &contentHash) {
  return m_repository.setGameHash(setId, contentHash);
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

bool AchievementService::processPatchResponse(const PatchResponse &response) {
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
    spdlog::error("Failed to create achievement set: {}", set.id);
    return false;
  }

  for (const auto &achievement : achievements) {
    if (!m_repository.create(achievement)) {
      spdlog::error("Failed to create achievement: {}", achievement.id);
      return false;
    }

    // Check if user unlock status exists, if not, create it as locked

    // if (!m_cache.getUserAchievementStatus(username, a.ID).has_value()) {
    //   auto newStatus = UserAchievementStatus{.achievementId = a.ID,
    //                                          .achieved = false,
    //                                          .achievedHardcore = false,
    //                                          .timestamp = 0,
    //                                          .timestampHardcore = 0};
    //
    //   m_cache.updateUserAchievementStatus(username, a.ID, newStatus);
    // }
  }
  return true;
}
} // namespace firelight::achievements