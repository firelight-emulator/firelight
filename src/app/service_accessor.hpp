#pragma once

#include <string>

namespace firelight {
  namespace library {
    class UserLibraryService;
  }

  namespace achievements {
    class AchievementService;
    class RAClient;
  }

  namespace settings {
    class SettingsService;
    class ICoreOptionRepository;
    class ISettingsRepository;
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

  namespace saves {
    class ISaveManager;
  }

  namespace mods {
    class IModRepository;
  }

  namespace discord {
    class IDiscordManager;
  }

  namespace gui {
    class GameImageProvider;
  }

  namespace media {
    class MediaService;
  }

  // The one app-wide service locator. Its contract: it exists only for objects
  // the QML engine default-constructs (qmlRegisterType models/items) which
  // cannot receive dependencies via a constructor. Everything we construct
  // ourselves should take its dependencies via the constructor instead. All
  // members are set once in main.cpp.
  class ServiceAccessor {
  public:
    static void setInputService(input::InputService *service);

    static void
    setControllerProfileRepository(input::IControllerRepository *repository);

    static void setPlatformService(platforms::PlatformService *service);

    static void
    setCoreOptionRepository(settings::ICoreOptionRepository *repository);

    static void setAchievementService(achievements::AchievementService *service);

    static void setLibraryService(library::UserLibraryService *service);

    static void setActivityService(activity::IActivityLog *service);

    static void setSaveManager(saves::ISaveManager *manager);

    static void setAchievementManager(achievements::RAClient *manager);

    static void setGameImageProvider(gui::GameImageProvider *provider);

    static void setModRepository(mods::IModRepository *repository);

    static void setDiscordManager(discord::IDiscordManager *manager);

    static void setMediaService(media::MediaService *service);

  protected:
    static input::InputService *getInputService();

    static input::IControllerRepository *getControllerProfileRepository();

    static platforms::PlatformService *getPlatformService();

    static settings::ICoreOptionRepository *getCoreOptionRepository();

    static achievements::AchievementService *getAchievementService();

    static library::UserLibraryService *getLibraryService();

    static activity::IActivityLog *getActivityService();

    static saves::ISaveManager *getSaveManager();

    static achievements::RAClient *getAchievementManager();

    static gui::GameImageProvider *getGameImageProvider();

    static mods::IModRepository *getModRepository();

    static discord::IDiscordManager *getDiscordManager();

    static media::MediaService *getMediaService();

  private:
    static input::InputService *s_inputService;
    static input::IControllerRepository *s_controllerProfileRepository;
    static platforms::PlatformService *s_platformService;
    static settings::ICoreOptionRepository *s_coreOptionRepository;
    static achievements::AchievementService *s_achievementService;
    static library::UserLibraryService *s_libraryService;
    static activity::IActivityLog *s_activityService;
    static saves::ISaveManager *s_saveManager;
    static achievements::RAClient *s_achievementManager;
    static gui::GameImageProvider *s_gameImageProvider;
    static mods::IModRepository *s_modRepository;
    static discord::IDiscordManager *s_discordManager;
    static media::MediaService *s_mediaService;
  };
} // namespace firelight
