#pragma once
#include "game.hpp"

#include <optional>

namespace firelight::achievements {
class AchievementService2 {
public:
  virtual ~AchievementService2() = default;

  virtual std::optional<Game> getGameById(unsigned id) = 0;
  virtual std::optional<Game> getGameByContentHash(std::string contentHash) = 0;
  virtual void startSession() = 0;
};
} // namespace firelight::achievements
