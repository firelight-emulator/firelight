#pragma once

#include <firelight/activity/activity_log.hpp>

#include <firelight/discord/idiscord_manager.hpp>
#include "emulator_config_manager.hpp"
#include "firelight/userdata_database.hpp"
#include <firelight/mods/mod_repository.hpp>
#include <firelight/saves/save_manager.hpp>
#include <firelight/settings/settings_repository.hpp>

#include "../../libs/firelight/achievements/src/rcheevos/ra_client.hpp"

namespace firelight {
    namespace gui {
        class GameImageProvider;
    }

    class ManagerAccessor {
    public:
        static void setSaveManager(saves::ISaveManager *t_manager);

        static void setUserdataManager(db::IUserdataDatabase *t_userdataManager);

        static void
        setAchievementManager(achievements::RAClient *t_achievementManager);

        static void setEmulatorConfigManager(
            std::shared_ptr<EmulatorConfigManager> t_emulatorConfigManager);

        static void setGameImageProvider(gui::GameImageProvider *t_gameImageProvider);

        static void setActivityLog(activity::IActivityLog *t_activityLog);

        static void setCoreSystemDirectory(const std::string &t_coreSystemDirectory);

        static void setModRepository(mods::IModRepository *t_modDatabase);

        static void setEmulationSettingsManager(
            settings::ISettingsRepository *t_emulationSettingsManager);

        static void setDiscordManager(discord::IDiscordManager *t_discordManager);

        static saves::ISaveManager *getSaveManager();

        static db::IUserdataDatabase *getUserdataManager();

        static achievements::RAClient *getAchievementManager();

        static std::shared_ptr<EmulatorConfigManager> getEmulatorConfigManager();

        static gui::GameImageProvider *getGameImageProvider();

        static activity::IActivityLog *getActivityLog();

        static std::string getCoreSystemDirectory();

        static mods::IModRepository *getModRepository();

        static settings::ISettingsRepository *getEmulationSettingsManager();

        static discord::IDiscordManager *getDiscordManager();

    private:
        static saves::ISaveManager *m_saveManager;
        static db::IUserdataDatabase *m_userdataDatabase;
        static achievements::RAClient *m_achievementManager;
        static std::shared_ptr<EmulatorConfigManager> m_emulatorConfigManager;
        static gui::GameImageProvider *m_gameImageProvider;
        static activity::IActivityLog *m_activityLog;
        static std::string m_coreSystemDirectory;
        static mods::IModRepository *m_modDatabase;
        static settings::ISettingsRepository *m_emulationSettingsManager;
        static discord::IDiscordManager *m_discordManager;
    };
} // namespace firelight
