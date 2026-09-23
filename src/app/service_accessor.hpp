#pragma once

namespace firelight {
namespace gui {
class GameImageProvider;
}

namespace saves {
class ISaveManager;
}

namespace library {
class IUserLibrary;
}
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
  static void setLibraryService(library::IUserLibrary *service);
  static void setSaveManager(saves::ISaveManager *manager);
  static void setGameImageProvider(gui::GameImageProvider *provider);

protected:
  static input::InputService *getInputService();
  static platforms::PlatformService *getPlatformService();
  static settings::SettingsService *getSettingsService();
  static achievements::AchievementService *getAchievementService();
  static library::IUserLibrary *getLibraryService();
  static saves::ISaveManager *getSaveManager();
  static gui::GameImageProvider *getGameImageProvider();

private:
  static input::InputService *s_inputService;
  static platforms::PlatformService *s_platformService;
  static settings::SettingsService *s_settingsService;
  static achievements::AchievementService *s_achievementService;
  static library::IUserLibrary *s_libraryService;
  static saves::ISaveManager *s_saveManager;
  static gui::GameImageProvider *s_gameImageProvider;
};

} // namespace firelight
