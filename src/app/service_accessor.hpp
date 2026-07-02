#pragma once

namespace firelight {
  namespace library {
    class UserLibraryService;
  }

  namespace achievements {
    class AchievementService;
  }

  namespace settings {
    class SettingsService;
    class ICoreOptionRepository;
  }

  namespace platforms {
    class PlatformService;
  }

  namespace input {
    class InputService;
    class IControllerRepository;
  }

  namespace activity {
    class IActivityLog;
  }

  class ServiceAccessor {
  public:
    static void setInputService(input::InputService *service);

    static void
    setControllerProfileRepository(input::IControllerRepository *repository);

    static void setPlatformService(platforms::PlatformService *service);

    static void setSettingsService(settings::SettingsService *service);

    static void
    setCoreOptionRepository(settings::ICoreOptionRepository *repository);

    static void setAchievementService(achievements::AchievementService *service);

    static void setLibraryService(library::UserLibraryService *service);

    static void setActivityService(activity::IActivityLog *service);

  protected:
    static input::InputService *getInputService();

    static input::IControllerRepository *getControllerProfileRepository();

    static platforms::PlatformService *getPlatformService();

    static settings::SettingsService *getSettingsService();

    static settings::ICoreOptionRepository *getCoreOptionRepository();

    static achievements::AchievementService *getAchievementService();

    static library::UserLibraryService *getLibraryService();

    static activity::IActivityLog *getActivityService();

  private:
    static input::InputService *s_inputService;
    static input::IControllerRepository *s_controllerProfileRepository;
    static platforms::PlatformService *s_platformService;
    static settings::SettingsService *s_settingsService;
    static settings::ICoreOptionRepository *s_coreOptionRepository;
    static achievements::AchievementService *s_achievementService;
    static library::UserLibraryService *s_libraryService;
    static activity::IActivityLog *s_activityService;
  };
} // namespace firelight
