#pragma once
#include "user_achievement_status.hpp"

#include <QSqlDatabase>
#include <cstdint>
#include <optional>
#include <rcheevos/user.h>
#include <string>

namespace firelight::achievements {
class RetroAchievementsBackend {
public:
  RetroAchievementsBackend();
  void updateUserScore(const std::string &username, int newScore,
                       bool hardcore);

  void incrementUserScore(const std::string &username, int pointsToAdd,
                          bool hardcore);

  void updateAchievementStatus(const std::string &username,
                               UserAchievementStatus status);

private:
  QString m_dbFilename;
  [[nodiscard]] QSqlDatabase getDatabase() const;
};
} // namespace firelight::achievements