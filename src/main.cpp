#define SDL_MAIN_HANDLED
#ifdef _WIN32
#include <windows.h>
#endif

#include "app/achievements/gui/AchievementSetItem.hpp"
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkInformation>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlNetworkAccessManagerFactory>
#include <QQuickWindow>
#include <QWindow>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <qstandardpaths.h>
#include <spdlog/spdlog.h>

#include "achievements/gui/retro_achievements_game_item.hpp"
#include "activity/gui/game_activity_item.hpp"
#include "app/audio/SfxPlayer.hpp"
#include <firelight/db/sqlite_content_database.hpp>
#include <firelight/db/sqlite_userdata_database.hpp>
#include "app/input/gui/analog_settings_model.hpp"
#include "app/input/gui/binding_list_model.hpp"
#include "app/input/gui/controller_list_model.hpp"
#include "app/input/gui/platform_input_preferences.hpp"
#include "app/input/gui/profile_list_model.hpp"
#include "app/input/shortcut_catalog.hpp"
#include <firelight/input/sqlite_controller_repository.hpp>
#include "app/library/gui/content_directory_model.hpp"
#include "app/library/gui/playlist_item_model.hpp"
#include <firelight/library/library_scanner2.hpp>
#include <firelight/saves/save_manager.hpp>
#include <firelight/saves/save_manager_impl.hpp>
#include "gui/eventhandlers/input_method_detection_handler.hpp"
#include "gui/eventhandlers/window_resize_handler.hpp"
#include "gui/game_image_provider.hpp"
#include "gui/models/shop/shop_item_model.hpp"
#include "gui/platform_list_model.hpp"
#include "gui/router.hpp"
#include "network_cache.hpp"
#include <firelight/achievement_service.hpp>
#include <rcheevos/ra_client.hpp>
#include <sqlite_achievement_repository.hpp>

#include <QtConcurrent>

#include <firelight/activity/sqlite_activity_log.hpp>
#include "app/emulator_item.hpp"
#include "app/input/gui/gamepad_status_item.hpp"
#include "app/library/gui/entry_list_model.hpp"
#include "app/library/gui/library_entry_item.hpp"
#include "app/library/gui/library_path_model.hpp"
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/library_events.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/event_dispatcher.hpp>
#include <firelight/mods/sqlite_mod_repository.hpp>
#include "app/mods/gui/ModInfoItem.hpp"
#include "app/saves/gui/suspend_points_item.hpp"
#include "gui/EventEmitter.h"
#include "gui/filesystem_utils.hpp"
#include "gui/gamepad_profile_item.hpp"
#include "gui/models/core_options_model.hpp"
#include "gui/models/emulation_settings_model.hpp"
#include "gui/qt_audio_settings_proxy.hpp"
#include "gui/models/game_activity_list_model.hpp"
#include "gui/qt_achievement_service_proxy.hpp"
#include "gui/qt_emulation_service_proxy.hpp"
#include "gui/qt_input_service_proxy.hpp"
#include <firelight/input/sdl_input_service.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/sqlite_core_option_repository.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <input/gui/input_mappings_model.hpp>
#include <firelight/input/keyboard_input_handler.hpp>
#include <firelight/platforms/platform_service.hpp>
#include <saves/gui/save_files_item.hpp>
#include <unistd.h>

#include "gui/eventhandlers/windows_frame_filter.hpp"
#include "gui/models/activity_buckets_list_model.hpp"
#include "gui/models/search_results_list_model.hpp"
#include <firelight/discord/discord_manager_impl.hpp>

int main(int argc, char *argv[]) {
    // SDL_setenv("QT_QUICK_FLICKABLE_WHEEL_DECELERATION", "5000", true);

    std::set_terminate([]() {
        spdlog::error("Terminating due to an unhandled exception");
        std::abort();
    });

    if (auto debug = std::getenv("FL_DEBUG"); debug != nullptr) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::info);
    }

    QApplication::setOrganizationDomain("firelight-emulator.com");
    QApplication::setDesktopFileName("firelight");
    QApplication::setApplicationName("Firelight");

    QSettings::setDefaultFormat(QSettings::Format::IniFormat);

    // QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    // QSurfaceFormat format;
    // format.setProfile(QSurfaceFormat::OpenGLContextProfile::CompatibilityProfile);
    // format.setVersion(4, 1);
    // QSurfaceFormat::setDefaultFormat(format);

    firelight::ServiceAccessor::setPlatformService(
        &firelight::platforms::PlatformService::getInstance());

    QSurfaceFormat format;
    format.setSwapInterval(0);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);

    std::signal(SIGINT, [](int signal) { QApplication::quit(); });

    auto docsPath =
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
            "/Firelight";

    auto defaultAppDataPathString =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QFileInfo info(QCoreApplication::applicationDirPath() + "/portable.txt");
    if (info.exists()) {
        spdlog::info("Found \"portable.txt\"; Enabling portable mode");
        docsPath = QCoreApplication::applicationDirPath();
        defaultAppDataPathString = docsPath + "/appdata";
    }

    auto savesPath = docsPath + "/saves";
    auto romsPath = docsPath + "/roms";

    QFileInfo savesDirInfo(savesPath);
    if (!savesDirInfo.exists() && QDir().mkpath(savesPath)) {
        spdlog::info("Created saves directory at {}", savesPath.toStdString());
    }

    QFileInfo romsDirInfo(romsPath);
    if (!romsDirInfo.exists() && QDir().mkpath(romsPath)) {
        spdlog::info("Created roms directory at {}", romsPath.toStdString());
    }

    QSettings::setPath(QSettings::Format::IniFormat, QSettings::Scope::UserScope,
                       defaultAppDataPathString);
    // TODO:
    //  Roms

    // images
    // auto cachePath =
    //     QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    // "C:/Users/<USER>/AppData/Local/Firelight/cache/"

    // auto patchesDir = appDataDir / "patches";

    // If missing system directory, throw an error
    // TODO

    // **** Make sure all directories are good ****

    QDir baseDir(defaultAppDataPathString);
    if (!baseDir.mkpath("core-system")) {
        spdlog::warn("Unable to create core-system directory");
    }

    firelight::ManagerAccessor::setCoreSystemDirectory(
        (defaultAppDataPathString + "/core-system").toStdString());

    firelight::input::SqliteControllerRepository controllerRepository(
        baseDir.filePath("controllers.db"));

    firelight::input::registerDefaultShortcuts();

    firelight::input::SDLInputService inputService(controllerRepository);
    firelight::ServiceAccessor::setInputService(&inputService);
    firelight::ServiceAccessor::setControllerProfileRepository(
        &controllerRepository);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

    firelight::db::SqliteUserdataDatabase userdata_database(
        defaultAppDataPathString + "/userdata.db");
    firelight::ManagerAccessor::setUserdataManager(&userdata_database);

    firelight::activity::SqliteActivityLog activityLog(defaultAppDataPathString +
                                                       "/activity.db");
    firelight::ManagerAccessor::setActivityLog(&activityLog);
    firelight::ServiceAccessor::setActivityService(&activityLog);

    auto gameImageProvider = new firelight::gui::GameImageProvider();
    firelight::ManagerAccessor::setGameImageProvider(gameImageProvider);

    //   **** Load Content Database ****
    firelight::db::SqliteContentDatabase contentDatabase(
        defaultAppDataPathString + "/content.db");

    firelight::saves::SaveManager saveManager(savesPath, userdata_database);
    firelight::ManagerAccessor::setSaveManager(&saveManager);

    firelight::library::SqliteUserLibraryRepository userLibrary(
        defaultAppDataPathString + "/library.db");

    // Drives content-file -> run-configuration -> entry orchestration off the
    // repository's events. Must outlive scanning.
    firelight::library::LibraryIngestService libIngestService(userLibrary);

    firelight::library::LibraryScanner2 libScanner2(userLibrary);

    // Re-watch/re-scan as watched directories come and go. Subscribed before the
    // service guarantees the default directory below, so the initial seed of a
    // fresh install is caught here.
    const auto watchedDirAddedConn = EventDispatcher::instance().subscribe<
        firelight::library::WatchedDirectoryAddedEvent>(
        [&](const firelight::library::WatchedDirectoryAddedEvent &e) {
            libScanner2.watchPath(QString::fromStdString(e.path));
            libScanner2.scanAll();
        });
    const auto watchedDirUpdatedConn = EventDispatcher::instance().subscribe<
        firelight::library::WatchedDirectoryUpdatedEvent>(
        [&](const firelight::library::WatchedDirectoryUpdatedEvent &e) {
            libScanner2.removePath(QString::fromStdString(e.oldPath));
            libScanner2.watchPath(QString::fromStdString(e.newPath));
            libScanner2.scanAll();
        });
    const auto watchedDirRemovedConn = EventDispatcher::instance().subscribe<
        firelight::library::WatchedDirectoryRemovedEvent>(
        [&](const firelight::library::WatchedDirectoryRemovedEvent &e) {
            libScanner2.removePath(QString::fromStdString(e.path));
            libScanner2.scanAll();
        });

    // The app-facing curation surface; also guarantees the default content
    // directory exists and is watched (fires the add event above when seeding).
    firelight::library::EntryResolver entryResolver(userLibrary);
    firelight::library::UserLibraryService userLibraryService(userLibrary,
                                                              romsPath);

    libScanner2.scanAll();

    firelight::achievements::SqliteAchievementRepository achievementRepo(
        (defaultAppDataPathString + "/rcheevos3.db").toStdString());
    firelight::achievements::AchievementService achievementService(
        achievementRepo);

    firelight::achievements::RetroAchievementsOfflineClient offlineRaClient(
        achievementService);
    firelight::achievements::RAClient raClient(offlineRaClient,
                                               achievementService);
    firelight::ManagerAccessor::setAchievementManager(&raClient);
    firelight::ServiceAccessor::setAchievementService(&achievementService);

    // Set up the models for QML
    // ***********************************************
    firelight::library::EntryListModel entryListModel(userLibraryService);

    QObject::connect(&libScanner2,
                     &firelight::library::LibraryScanner2::scanFinished,
                     &entryListModel, &firelight::library::EntryListModel::reset,
                     Qt::QueuedConnection);

    firelight::gui::PlatformListModel platformListModel;
    firelight::shop::ShopItemModel shopItemModel(contentDatabase);

    firelight::gui::ContentDirectoryModel contentDirectoryModel(userLibraryService);

    firelight::ServiceAccessor::setLibraryService(&userLibraryService);

    auto emulatorConfigManager =
            std::make_shared<EmulatorConfigManager>(userdata_database);
    firelight::ManagerAccessor::setEmulatorConfigManager(emulatorConfigManager);

    firelight::mods::SqliteModRepository modRepository;
    firelight::ManagerAccessor::setModRepository(&modRepository);

    firelight::settings::SqliteSettingsRepository emulationSettingsManager(
        (defaultAppDataPathString + "/settings.db").toStdString());
    firelight::ManagerAccessor::setEmulationSettingsManager(
        &emulationSettingsManager);

    // Caches each core's declared options (populated after a core loads) so the
    // advanced options editor can list them without the core running.
    firelight::settings::SqliteCoreOptionRepository coreOptionRepository(
        (defaultAppDataPathString + "/settings.db").toStdString());
    firelight::ServiceAccessor::setCoreOptionRepository(&coreOptionRepository);
    //   QObject::connect(
    //     &libraryDatabase,
    //     &firelight::db::SqliteLibraryDatabase::contentDirectoriesUpdated,
    //     &libraryManager, &LibraryScanner::startScan);
    //
    //   QObject::connect(&libraryManager, &LibraryScanner::scanFinished,
    //   &libModel,
    //                    &firelight::gui::LibraryItemModel::refresh);
    //
    //   libraryManager.startScan();

    //   qRegisterMetaType<firelight::gui::GamepadMapping>("GamepadMapping");

    firelight::settings::SettingsService settingsService(
        emulationSettingsManager);

    firelight::settings::SettingsService::setInstance(&settingsService);

    // Friendly emulation settings + per-core option defaults. Loaded once into
    // the shared catalog; the emulation path and settings UI read from it.
    if (!firelight::settings::SettingsCatalog::instance().loadFromFile(
            "system/settings_catalog.json")) {
      spdlog::warn("Could not load settings catalog from "
                   "system/settings_catalog.json; using core defaults only");
    }

    qmlRegisterType<EmulatorItem>("Firelight", 1, 0, "EmulatorItem");
    qmlRegisterType<firelight::input::GamepadStatusItem>("Firelight", 1, 0,
                                                         "GamepadStatus");
    qmlRegisterType<firelight::gui::GamepadProfileItem>("Firelight", 1, 0,
                                                        "GamepadProfile");
    qmlRegisterType<firelight::mods::ModInfoItem>("Firelight", 1, 0, "ModInfo");

    qmlRegisterType<firelight::achievements::RetroAchievementsGameItem>(
        "Firelight", 1, 0, "RetroAchievementsGame");

    qmlRegisterType<firelight::LibraryEntryItem>("Firelight", 1, 0,
                                                 "LibraryEntry");
    qmlRegisterType<firelight::saves::SuspendPointsItem>("Firelight", 1, 0,
                                                         "SuspendPoints");
    qmlRegisterType<firelight::activity::GameActivityItem>("Firelight", 1, 0,
                                                           "GameActivity");

    qmlRegisterType<firelight::saves::SaveFilesItem>("Firelight", 1, 0,
                                                     "SaveDataInformation");

    qmlRegisterType<firelight::input::InputMappingsModel>("Firelight", 1, 0,
                                                          "InputMappingsModel");
    qmlRegisterType<firelight::input::AnalogSettingsModel>(
        "Firelight", 1, 0, "AnalogSettingsModel");
    qmlRegisterType<firelight::input::ProfileListModel>(
        "Firelight", 1, 0, "ProfileListModel");
    qmlRegisterType<firelight::input::BindingListModel>(
        "Firelight", 1, 0, "BindingListModel");
    qmlRegisterType<firelight::input::PlatformInputPreferences>(
        "Firelight", 1, 0, "PlatformInputPreferences");
    qmlRegisterType<firelight::gui::ControllerListModel>("Firelight", 1, 0,
                                                         "GamepadListModel");
    qmlRegisterType<firelight::settings::EmulationSettingsModel>(
        "Firelight", 1, 0, "EmulationSettingsModel");
    qmlRegisterType<firelight::settings::CoreOptionsModel>(
        "Firelight", 1, 0, "CoreOptionsModel");
    qmlRegisterType<firelight::activity::GameActivityListModel>(
        "Firelight", 1, 0, "GameActivityModel");

    QNetworkInformation::loadDefaultBackend();
    if (QNetworkInformation::instance()->reachability() ==
        QNetworkInformation::Reachability::Online) {
        raClient.m_connected = true;
        achievementService.syncOfflineAchievements();
    }

    QObject::connect(
        QNetworkInformation::instance(),
        &QNetworkInformation::reachabilityChanged,
        [&raClient,
            &achievementService](QNetworkInformation::Reachability reachability) {
            if (reachability == QNetworkInformation::Reachability::Online) {
                raClient.m_connected = true;
                achievementService.syncOfflineAchievements();
            } else {
                raClient.m_connected = false;
            }
        });

    // QQmlNetworkAccessManagerFactory::create();
    // QNetworkAccessManager *manager = new QNetworkAccessManager();
    // QNetworkDiskCache *diskCache = new QNetworkDiskCache();
    // QString directory =
    // QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
    //         + QLatin1StringView("/cacheDir/");
    // diskCache->setCacheDirectory(directory);
    // manager->setCache(diskCache);

    firelight::emulation::EmulationService emuService(userLibraryService,
                                                      entryResolver,
                                                      settingsService);
    firelight::emulation::EmulationService::setInstance(&emuService);

    firelight::gui::LibraryFolderListModel libraryFolderListModel;

    QObject::connect(&libraryFolderListModel,
                     &firelight::gui::LibraryFolderListModel::folderDeleted,
                     &entryListModel, &firelight::library::EntryListModel::reset,
                     Qt::QueuedConnection);

    auto cache = new CachingNetworkAccessManagerFactory();

    QQmlApplicationEngine engine;
    engine.setNetworkAccessManagerFactory(cache);
    // engine.networkAccessManager()->setCache(diskCache);

    // engine.addUrlInterceptor(&imageCacheUrlInterceptor);
    engine.addImageProvider("gameImages", gameImageProvider);

    engine.rootContext()->setContextProperty("EmulationSettingsManager",
                                             &emulationSettingsManager);
    engine.rootContext()->setContextProperty(
        "FilesystemUtils", new firelight::gui::FilesystemUtils());
    engine.rootContext()->setContextProperty("EventEmitter",
                                             new firelight::gui::EventEmitter());
    engine.rootContext()->setContextProperty(
        "AudioSettings", new firelight::gui::QtAudioSettingsProxy());
    engine.rootContext()->setContextProperty("Router",
                                             new firelight::gui::Router());
    engine.rootContext()->setContextProperty("emulator_config_manager",
                                             emulatorConfigManager.get());
    engine.rootContext()->setContextProperty("achievement_manager", &raClient);
    engine.rootContext()->setContextProperty("shop_item_model", &shopItemModel);
    engine.rootContext()->setContextProperty("SaveManager", &saveManager);
    engine.rootContext()->setContextProperty("ContentDirectoryModel",
                                             &contentDirectoryModel);
    engine.rootContext()->setContextProperty("LibraryEntryModel",
                                             &entryListModel);
    engine.rootContext()->setContextProperty("PlatformModel",
                                             &platformListModel);

    const auto activityBucketsModel = new firelight::gui::ActivityBucketsListModel();
    engine.rootContext()->setContextProperty("ActivityBucketsModel",
                                             activityBucketsModel);

    const auto searchResultsModel = new firelight::gui::SearchResultsListModel();
    engine.rootContext()->setContextProperty("SearchResultsModel", searchResultsModel);


    engine.rootContext()->setContextProperty(
        "InputService", new firelight::gui::QtInputServiceProxy());
    engine.rootContext()->setContextProperty(
        "EmulationService", new firelight::gui::QtEmulationServiceProxy());
    engine.rootContext()->setContextProperty(
        "AchievementService", new firelight::gui::QtAchievementServiceProxy());

    engine.rootContext()->setContextProperty("LibraryFolderModel",
                                             &libraryFolderListModel);
    engine.rootContext()->setContextProperty("LibraryScanner", &libScanner2);

    auto resizeHandler = new firelight::gui::WindowResizeHandler();
    engine.rootContext()->setContextProperty("window_resize_handler",
                                             resizeHandler);
    auto inputMethodDetectionHandler =
            new firelight::gui::InputMethodDetectionHandler();
    engine.rootContext()->setContextProperty("InputMethodManager",
                                             inputMethodDetectionHandler);

    auto *frameFilter = new WindowsFrameFilter();
    app.installNativeEventFilter(frameFilter);
    engine.rootContext()->setContextProperty("WindowFrame", frameFilter);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("QMLFirelight", "Main3");

    QObject *rootObject = engine.rootObjects().value(0);
    auto window = qobject_cast<QQuickWindow *>(rootObject);

    window->installEventFilter(resizeHandler);
    window->installEventFilter(inputMethodDetectionHandler);

    auto keyboardHandler = new firelight::input::KeyboardInputHandler();
    window->installEventFilter(keyboardHandler);

    inputService.setKeyboard(
        std::shared_ptr<firelight::input::IGamepad>(keyboardHandler));

    auto inputLoopFuture = QtConcurrent::run([&] { inputService.run(); });

    firelight::discord::DiscordManager discordManager;
    discordManager.initialize();
    firelight::ManagerAccessor::setDiscordManager(&discordManager);

    QObject::connect(window, &QQuickWindow::afterRendering,
                     [&]() { discordManager.runCallbacks(); });

    window->setIcon(QIcon("qrc:images/firelight-logo"));

    engine.rootContext()->setContextProperty("sfx_player",
                                             new firelight::audio::SfxPlayer());

    int exitCode = QApplication::exec();

    spdlog::info("Exiting QApplication");

    inputService.stop();
    inputLoopFuture.waitForFinished();

    // engine.removeImageProvider("gameImages");
    // TODO: Let daemons finish

    return exitCode;
}
