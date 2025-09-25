#pragma once

#include "achievement_repository.hpp"
#include "models/achievement_progress.hpp"
#include "models/user_unlock.hpp"

#include <optional>
#include <rcheevos/patch_response.hpp>

namespace firelight::achievements {

class AchievementService {
public:
  explicit AchievementService(IAchievementRepository &m_repository);

  [[nodiscard]] std::optional<AchievementSet>
  getAchievementSetByContentHash(const std::string &contentHash) const;

  bool setGameId(int setId, const std::string &contentHash);
  bool updateAchievementProgress(const AchievementProgress &progress);

  [[nodiscard]] std::optional<UserUnlock>
  getUserUnlock(const std::string &username, unsigned achievementId) const;

  std::optional<PatchResponse> getPatchResponse(int gameId) const;
  bool processPatchResponse(const PatchResponse &response);

private:
  IAchievementRepository &m_repository;
};

} // namespace firelight::achievements
