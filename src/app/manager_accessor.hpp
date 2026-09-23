#pragma once

#include <firelight/activity/activity_log.hpp>
#include <library/user_library.hpp>

#include "discord/discord_manager.hpp"
#include "emulator_config_manager.hpp"
#include "firelight/userdata_database.hpp"
#include "mods/mod_repository.hpp"
#include "rcheevos/ra_client.hpp"
#include "settings/settings_repository.hpp"

namespace firelight {

class ManagerAccessor {
public:

  static void setUserdataManager(db::IUserdataDatabase *t_userdataManager);

  static void
  setAchievementManager(achievements::RAClient *t_achievementManager);

  static void setEmulatorConfigManager(
      std::shared_ptr<EmulatorConfigManager> t_emulatorConfigManager);

  static void setUserLibrary(library::IUserLibrary *t_userLibrary);

  static void setActivityLog(activity::IActivityLog *t_activityLog);

  static void setCoreSystemDirectory(const std::string &t_coreSystemDirectory);

  static void setModRepository(mods::IModRepository *t_modDatabase);

  static void setEmulationSettingsManager(
      settings::ISettingsRepository *t_emulationSettingsManager);

  static void setDiscordManager(discord::DiscordManager *t_discordManager);

  static db::IUserdataDatabase *getUserdataManager();

  static achievements::RAClient *getAchievementManager();

  static std::shared_ptr<EmulatorConfigManager> getEmulatorConfigManager();

  static library::IUserLibrary *getUserLibrary();

  static activity::IActivityLog *getActivityLog();

  static std::string getCoreSystemDirectory();

  static mods::IModRepository *getModRepository();

  static settings::ISettingsRepository *getEmulationSettingsManager();

  static discord::DiscordManager *getDiscordManager();

private:
  static db::IUserdataDatabase *m_userdataDatabase;
  static achievements::RAClient *m_achievementManager;
  static std::shared_ptr<EmulatorConfigManager> m_emulatorConfigManager;
  static library::IUserLibrary *m_userLibrary;
  static activity::IActivityLog *m_activityLog;
  static std::string m_coreSystemDirectory;
  static mods::IModRepository *m_modDatabase;
  static settings::ISettingsRepository *m_emulationSettingsManager;
  static discord::DiscordManager *m_discordManager;
};
} // namespace firelight
