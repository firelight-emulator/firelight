#pragma once
#include "achievement.hpp"

#include <vector>

namespace firelight::achievements {
class IRetroAchievementsService {
public:
  virtual ~IRetroAchievementsService() = default;

  virtual void createHashRecord(const std::string &hash, int setId) = 0;

  virtual std::vector<Achievement> getAchievements(int setId) = 0;
};
} // namespace firelight::achievements