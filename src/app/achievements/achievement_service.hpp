#pragma once

#include "achievement_repository.hpp"
#include "models/achievement_progress.hpp"
#include "models/user_unlock.hpp"

#include <optional>
#include <rcheevos/patch_response.hpp>
#include <rcheevos/startsession_response.hpp>

namespace firelight::achievements {

class AchievementService {
public:
  explicit AchievementService(IAchievementRepository &m_repository);

  [[nodiscard]] std::optional<AchievementSet>
  getAchievementSetByContentHash(const std::string &contentHash) const;

  bool setGameId(const std::string &contentHash, int setId);
  bool updateAchievementProgress(const AchievementProgress &progress);

  [[nodiscard]] std::optional<UserUnlock>
  getUserUnlock(const std::string &username, unsigned achievementId) const;

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
