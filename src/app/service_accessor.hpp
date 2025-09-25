#pragma once

namespace firelight {
namespace achievements {
class AchievementService;
}
namespace settings {
class SettingsService;
}
namespace platforms {
class PlatformService;
}
namespace input {
class InputService;
}

class ServiceAccessor {
public:
  static void setInputService(input::InputService *service);
  static void setPlatformService(platforms::PlatformService *service);
  static void setSettingsService(settings::SettingsService *service);
  static void setAchievementService(achievements::AchievementService *service);

protected:
  static input::InputService *getInputService();
  static platforms::PlatformService *getPlatformService();
  static settings::SettingsService *getSettingsService();
  static achievements::AchievementService *getAchievementService();

private:
  static input::InputService *s_inputService;
  static platforms::PlatformService *s_platformService;
  static settings::SettingsService *s_settingsService;
  static achievements::AchievementService *s_achievementService;
};

} // namespace firelight
