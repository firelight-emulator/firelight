#pragma once

#include "types.h"

namespace discord {

class AchievementManager final {
public:
    ~AchievementManager() = default;

    void SetUserAchievement(Snowflake achievementId,
                            uint8_t percentComplete,
                            std::function<void(Result)> callback);
    void FetchUserAchievements(std::function<void(Result)> callback);
    void CountUserAchievements(int32_t* count);
    Result GetUserAchievement(Snowflake userAchievementId, UserAchievement* userAchievement);
    Result GetUserAchievementAt(int32_t index, UserAchievement* userAchievement);

    Event<UserAchievement const&> OnUserAchievementUpdate;

private:
    friend class Core;

    AchievementManager() = default;
    AchievementManager(AchievementManager const& rhs) = delete;
    AchievementManager& operator=(AchievementManager const& rhs) = delete;
    AchievementManager(AchievementManager&& rhs) = delete;
    AchievementManager& operator=(AchievementManager&& rhs) = delete;

    IDiscordAchievementManager* internal_;
    static IDiscordAchievementEvents events_;
};

} // namespace discord
