#pragma once
#include "emulator.hpp"

#include <optional>
#include <string>

namespace firelight::emulation {

struct GameLoadedEvent {};
struct GameLoadFailedEvent {};

struct EmulationPausedEvent {};
struct EmulationResumedEvent {};

struct EmulationStartedEvent {};
struct EmulationStoppedEvent {};

struct EmulationSettingChangedEvent {};

struct AchievementsLoadedEvent {};
struct AchievementsLoadFailedEvent {};
struct AchievementUnlockedEvent {};
struct AchievementProgressUpdatedEvent {};
struct AchievementChallengeIndicatorStateChangedEvent {};

class EmulationService {
  std::optional<Emulator> loadGame(std::string contentHash);
};

} // namespace firelight::emulation