#pragma once

#include <firelight/library/user_library.hpp>
#include <firelight/activity/activity_log.hpp>

#include "emulator_config_manager.hpp"
#include "rcheevos/ra_client.hpp"
#include "firelight/userdata_database.hpp"
#include "input/controller_manager.hpp"
#include "input/input_manager.hpp"
#include "library/library_scanner.hpp"
#include "saves/save_manager.hpp"

namespace firelight {
    namespace gui {
        class GameImageProvider;
    }

    class ManagerAccessor {
    public:
        static void setControllerManager(Input::ControllerManager *t_manager);

        static void setSaveManager(saves::SaveManager *t_manager);

        static void setLibraryManager(LibraryScanner *t_libraryManager);

        static void setUserdataManager(db::IUserdataDatabase *t_userdataManager);

        static void setLibraryDatabase(db::ILibraryDatabase *t_libraryDatabase);

        static void
        setAchievementManager(achievements::RAClient *t_achievementManager);

        static void setEmulatorConfigManager(
            std::shared_ptr<EmulatorConfigManager> t_emulatorConfigManager);

        static void setGameImageProvider(gui::GameImageProvider *t_gameImageProvider);

        static void setUserLibrary(library::IUserLibrary *t_userLibrary);

        static void setActivityLog(activity::IActivityLog *t_activityLog);

        static void setInputManager(input::InputManager *t_inputManager);

        static Input::ControllerManager *getControllerManager();

        static saves::SaveManager *getSaveManager();

        static LibraryScanner *getLibraryManager();

        static db::IUserdataDatabase *getUserdataManager();

        static db::ILibraryDatabase *getLibraryDatabase();

        static achievements::RAClient *getAchievementManager();

        static std::shared_ptr<EmulatorConfigManager> getEmulatorConfigManager();

        static gui::GameImageProvider *getGameImageProvider();

        static library::IUserLibrary *getUserLibrary();

        static activity::IActivityLog *getActivityLog();

        static input::InputManager *getInputManager();

    private:
        static Input::ControllerManager *m_controllerManager;
        static saves::SaveManager *m_saveManager;
        static LibraryScanner *m_libraryManager;
        static db::IUserdataDatabase *m_userdataDatabase;
        static db::ILibraryDatabase *m_libraryDatabase;
        static achievements::RAClient *m_achievementManager;
        static std::shared_ptr<EmulatorConfigManager> m_emulatorConfigManager;
        static gui::GameImageProvider *m_gameImageProvider;
        static library::IUserLibrary *m_userLibrary;
        static activity::IActivityLog *m_activityLog;
        static input::InputManager *m_inputManager;
    };
} // namespace firelight
