#pragma once

#include <activity/activity_log.hpp>
#include <library/user_library.hpp>

#include "discord/discord_manager.hpp"
#include "emulator_config_manager.hpp"
#include "firelight/userdata_database.hpp"
#include "input/controller_manager.hpp"
#include "mods/mod_repository.hpp"
#include "rcheevos/ra_client.hpp"
#include "saves/save_manager.hpp"
#include "settings/emulation_settings_manager.hpp"

namespace firelight {
namespace gui {
class GameImageProvider;
}

class ManagerAccessor {
public:
  static void setSaveManager(saves::SaveManager *t_manager);

  static void setUserdataManager(db::IUserdataDatabase *t_userdataManager);

  static void setLibraryDatabase(db::ILibraryDatabase *t_libraryDatabase);

  static void
  setAchievementManager(achievements::RAClient *t_achievementManager);

  static void setEmulatorConfigManager(
      std::shared_ptr<EmulatorConfigManager> t_emulatorConfigManager);

  static void setGameImageProvider(gui::GameImageProvider *t_gameImageProvider);

  static void setUserLibrary(library::IUserLibrary *t_userLibrary);

  static void setActivityLog(activity::IActivityLog *t_activityLog);

  static void setCoreSystemDirectory(const std::string &t_coreSystemDirectory);

  static void setModRepository(mods::IModRepository *t_modDatabase);

  static void setEmulationSettingsManager(
      settings::IEmulationSettingsManager *t_emulationSettingsManager);

  static void setDiscordManager(discord::DiscordManager *t_discordManager);

  static saves::SaveManager *getSaveManager();

  static db::IUserdataDatabase *getUserdataManager();

  static db::ILibraryDatabase *getLibraryDatabase();

  static achievements::RAClient *getAchievementManager();

  static std::shared_ptr<EmulatorConfigManager> getEmulatorConfigManager();

  static gui::GameImageProvider *getGameImageProvider();

  static library::IUserLibrary *getUserLibrary();

  static activity::IActivityLog *getActivityLog();

  static std::string getCoreSystemDirectory();

  static mods::IModRepository *getModRepository();

  static settings::IEmulationSettingsManager *getEmulationSettingsManager();

  static discord::DiscordManager *getDiscordManager();

private:
  static input::ControllerManager *m_controllerManager;
  static saves::SaveManager *m_saveManager;
  static db::IUserdataDatabase *m_userdataDatabase;
  static db::ILibraryDatabase *m_libraryDatabase;
  static achievements::RAClient *m_achievementManager;
  static std::shared_ptr<EmulatorConfigManager> m_emulatorConfigManager;
  static gui::GameImageProvider *m_gameImageProvider;
  static library::IUserLibrary *m_userLibrary;
  static activity::IActivityLog *m_activityLog;
  static input::IControllerRepository *m_controllerRepository;
  static std::string m_coreSystemDirectory;
  static mods::IModRepository *m_modDatabase;
  static settings::IEmulationSettingsManager *m_emulationSettingsManager;
  static discord::DiscordManager *m_discordManager;
};
} // namespace firelight
