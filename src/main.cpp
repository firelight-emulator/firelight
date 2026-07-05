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
#include <QVulkanInstance>
#include <QWindow>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <qstandardpaths.h>
#include <set>
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
#include <firelight/input/shortcut_catalog.hpp>
#include <firelight/input/sqlite_controller_repository.hpp>
#include <firelight/cheats/sqlite_cheat_repository.hpp>
#include "app/emulation/emulation_context.hpp"
#include "app/library/gui/content_directory_model.hpp"
#include "app/library/gui/playlist_item_model.hpp"
#include <firelight/library/library_scanner2.hpp>
#include <firelight/saves/save_manager.hpp>
#include <firelight/saves/save_manager_impl.hpp>
#include "cli/cli_app.hpp"
#include "cli/console.hpp"
#include "cli/data_dirs.hpp"
#include "cli/launch_config.hpp"
#include "cli/list_command.hpp"
#include "cli/login_command.hpp"
#include "cli/rom_launch.hpp"
#include "cli/scan_command.hpp"
#include "cli/single_instance.hpp"
#include "cli/startup_options.hpp"
#include "libretro/core_registry.hpp"
#include <firelight/media/media_service.hpp>
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
#include "gui/qt_core_registry_proxy.hpp"
#include "gui/models/game_activity_list_model.hpp"
#include "gui/qt_achievement_service_proxy.hpp"
#include "gui/qt_save_manager_proxy.hpp"
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

    // Make CLI output visible when launched from a terminal (Windows GUI apps
    // have no console by default).
    firelight::cli::attachParentConsole();

    std::set_terminate([]() {
        spdlog::error("Terminating due to an unhandled exception");
        std::abort();
    });

    if (auto debug = std::getenv("FL_DEBUG"); debug != nullptr) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::info);
    }

    // QApplication::setOrganizationName("BiscuitCakes");
    QApplication::setOrganizationDomain("firelight-emulator.com");
    QApplication::setDesktopFileName("firelight");
    QApplication::setApplicationName("Firelight");
#ifndef FL_VERSION
#define FL_VERSION "0.0.0"
#endif
    QApplication::setApplicationVersion(QStringLiteral(FL_VERSION));

    QSettings::setDefaultFormat(QSettings::Format::IniFormat);

    // Parse the command line before doing any GUI setup: --help/--version and
    // subcommands short-circuit here. The application/organization names above
    // are already set (they're static), so path resolution works in both paths.
    const auto cliOptions = firelight::cli::parseCli(argc, argv);
    if (cliOptions.action == firelight::cli::CliAction::Exit) {
        return cliOptions.exitCode;
    }
    if (cliOptions.verbose) {
        spdlog::set_level(spdlog::level::debug);
    }
    if (cliOptions.action == firelight::cli::CliAction::RunScan) {
        // Headless: no QApplication / QML engine.
        return firelight::cli::runScan(argc, argv, cliOptions);
    }
    if (cliOptions.action == firelight::cli::CliAction::Login) {
        return firelight::cli::runLogin(argc, argv, cliOptions);
    }
    if (cliOptions.action == firelight::cli::CliAction::ListSettings) {
        return firelight::cli::runListSettings(argc, argv);
    }
    if (cliOptions.action == firelight::cli::CliAction::ListCores) {
        return firelight::cli::runListCores(argc, argv);
    }

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

    // Default icon for every window/dialog (matches the executable's icon).
    app.setWindowIcon(QIcon(":/images/app-icon"));

    std::signal(SIGINT, [](int signal) { QApplication::quit(); });

    // Resolve data directories (honors --config-dir / --portable and the legacy
    // portable.txt marker). Runs after QApplication so applicationDirPath works.
    const auto dataDirs = firelight::cli::resolveDataDirs(cliOptions);
    auto docsPath = dataDirs.docsPath;
    auto defaultAppDataPathString = dataDirs.appDataPath;
    auto savesPath = dataDirs.savesPath;
    auto romsPath = dataDirs.romsPath;
    auto screenshotsPath = dataDirs.screenshotsPath;

    // Single-instance forwarding (opt-in). If another Firelight (with the same
    // data dir) is already running, hand it this launch and exit; otherwise we
    // become the primary and listen for forwards (set up after the library is
    // built). The server name is derived from the data dir so distinct
    // --config-dir instances stay independent.
    const auto singleInstanceName =
            firelight::cli::singleInstanceServerName(defaultAppDataPathString);
    if (cliOptions.singleInstance &&
        firelight::cli::forwardLaunchToRunningInstance(singleInstanceName,
                                                       cliOptions)) {
        return 0;
    }

    QFileInfo savesDirInfo(savesPath);
    if (!savesDirInfo.exists() && QDir().mkpath(savesPath)) {
        spdlog::info("Created saves directory at {}", savesPath.toStdString());
    }

    QFileInfo romsDirInfo(romsPath);
    if (!romsDirInfo.exists() && QDir().mkpath(romsPath)) {
        spdlog::info("Created roms directory at {}", romsPath.toStdString());
    }

    QDir().mkpath(screenshotsPath);
    // Owns writing captured screenshots to disk; reached by the emulator
    // renderer via ServiceAccessor. Declared here so it outlives the QML engine.
    firelight::media::MediaService mediaService(screenshotsPath);
    firelight::ServiceAccessor::setMediaService(&mediaService);

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

    // PPSSPP loads a runtime asset tree (fonts, VFPU tables, texture atlases,
    // compat db) from <system>/PPSSPP. These aren't part of the core DLL, so the
    // app can't run PSP games without them. Seed the writable core-system copy
    // once from the assets bundled next to the executable. Idempotent: keyed on a
    // marker asset so it only runs when the destination hasn't been seeded yet.
    {
        namespace fs = std::filesystem;
        const fs::path ppssppDest =
            fs::path(defaultAppDataPathString.toStdString()) / "core-system" /
            "PPSSPP";
        const fs::path ppssppSrc =
            fs::path(QCoreApplication::applicationDirPath().toStdString()) /
            "system" / "PPSSPP";
        std::error_code ec;
        if (!fs::exists(ppssppDest / "ppge_atlas.zim", ec) &&
            fs::exists(ppssppSrc / "ppge_atlas.zim", ec)) {
            spdlog::info("Seeding PPSSPP assets: {} -> {}", ppssppSrc.string(),
                         ppssppDest.string());
            fs::copy(ppssppSrc, ppssppDest,
                     fs::copy_options::recursive |
                         fs::copy_options::overwrite_existing,
                     ec);
            if (ec) {
                spdlog::warn("Failed to seed PPSSPP assets: {}", ec.message());
            }
        }
    }

    // Declared early so it outlives the QML engine and its EmulatorItem: those
    // are destroyed before the objects declared above them, and on exit while a
    // game is running the EmulatorItem's teardown calls back into the Discord
    // manager (to clear rich presence). If it were declared later it would be
    // destroyed first, and that call would hit freed memory (Discord SDK assert
    // / crash). `initialize()` still runs later, once the window exists.
    firelight::discord::DiscordManager discordManager;
    firelight::ServiceAccessor::setDiscordManager(&discordManager);

    firelight::input::SqliteControllerRepository controllerRepository(
        baseDir.filePath("controllers.db"),
        firelight::platforms::PlatformService::getInstance());

    firelight::input::registerDefaultShortcuts();

    firelight::input::SDLInputService inputService(controllerRepository);
    firelight::ServiceAccessor::setInputService(&inputService);
    firelight::ServiceAccessor::setControllerProfileRepository(
        &controllerRepository);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

    firelight::db::SqliteUserdataDatabase userdata_database(
        defaultAppDataPathString + "/userdata.db");

    firelight::activity::SqliteActivityLog activityLog(defaultAppDataPathString +
                                                       "/activity.db");
    firelight::ServiceAccessor::setActivityService(&activityLog);

    auto gameImageProvider = new firelight::gui::GameImageProvider();
    firelight::ServiceAccessor::setGameImageProvider(gameImageProvider);

    //   **** Load Content Database ****
    firelight::db::SqliteContentDatabase contentDatabase(
        defaultAppDataPathString + "/content.db");

    firelight::saves::SaveManager saveManager(savesPath.toStdString(),
                                              userdata_database);
    firelight::ServiceAccessor::setSaveManager(&saveManager);

    // Thin QML adapter over the (Qt-notification-free) save manager; exposes the
    // save directory as a bindable property for the settings UI. Declared here so
    // it outlives the QML engine registered below.
    firelight::gui::QtSaveManagerProxy saveManagerProxy(saveManager);

    firelight::library::SqliteUserLibraryRepository userLibrary(
        defaultAppDataPathString + "/library.db");

    // Drives content-file -> run-configuration -> entry orchestration off the
    // repository's events. Must outlive scanning.
    firelight::library::LibraryIngestService libIngestService(userLibrary);

    firelight::library::LibraryScanner2 libScanner2(userLibrary);

    // Re-watch/re-scan as content directories come and go. Subscribed before the
    // service guarantees the default directory below, so the initial seed of a
    // fresh install is caught here.
    const auto contentDirAddedConn = EventDispatcher::instance().subscribe<
        firelight::library::ContentDirectoryAddedEvent>(
        [&](const firelight::library::ContentDirectoryAddedEvent &e) {
            libScanner2.watchPath(QString::fromStdString(e.path));
            libScanner2.scanAll();
        });
    const auto contentDirUpdatedConn = EventDispatcher::instance().subscribe<
        firelight::library::ContentDirectoryUpdatedEvent>(
        [&](const firelight::library::ContentDirectoryUpdatedEvent &e) {
            libScanner2.removePath(QString::fromStdString(e.oldPath));
            libScanner2.watchPath(QString::fromStdString(e.newPath));
            libScanner2.scanAll();
        });
    const auto contentDirRemovedConn = EventDispatcher::instance().subscribe<
        firelight::library::ContentDirectoryRemovedEvent>(
        [&](const firelight::library::ContentDirectoryRemovedEvent &e) {
            libScanner2.removePath(QString::fromStdString(e.path));
            libScanner2.scanAll();
        });

    // The app-facing curation surface; also guarantees the default content
    // directory exists and is watched (fires the add event above when seeding).
    firelight::library::EntryResolver entryResolver(userLibrary);
    firelight::library::UserLibraryService userLibraryService(
        userLibrary, romsPath.toStdString());

    libScanner2.scanAll();

    // A ROM path passed on the command line resolves to a library entry that the
    // root window auto-launches once loaded (-1 when absent/unresolved).
    const int startupLaunchEntryId =
            firelight::cli::resolveRomEntryId(cliOptions.romPath, userLibraryService);

    // If we're the primary --single-instance process, start listening for
    // launches forwarded from secondary processes. Exposed to QML below so the
    // root window turns launchRequested into window.startGame(entryId).
    std::unique_ptr<firelight::cli::SingleInstanceServer> singleInstanceServer;
    if (cliOptions.singleInstance) {
        singleInstanceServer =
                std::make_unique<firelight::cli::SingleInstanceServer>(
                    singleInstanceName, userLibraryService);
        singleInstanceServer->start();
    }

    firelight::achievements::SqliteAchievementRepository achievementRepo(
        (defaultAppDataPathString + "/rcheevos3.db").toStdString());
    firelight::achievements::AchievementService achievementService(
        achievementRepo);

    firelight::achievements::RetroAchievementsOfflineClient offlineRaClient(
        achievementService);
    firelight::achievements::RAClient raClient(offlineRaClient,
                                               achievementService);
    firelight::ServiceAccessor::setAchievementManager(&raClient);
    firelight::ServiceAccessor::setAchievementService(&achievementService);

    // Set up the models for QML
    // ***********************************************
    firelight::library::EntryListModel entryListModel(userLibraryService,
                                                      activityLog);

    QObject::connect(&libScanner2,
                     &firelight::library::LibraryScanner2::scanFinished,
                     &entryListModel, &firelight::library::EntryListModel::reset,
                     Qt::QueuedConnection);

    firelight::gui::PlatformListModel platformListModel;
    firelight::shop::ShopItemModel shopItemModel(contentDatabase);

    firelight::gui::ContentDirectoryModel contentDirectoryModel(userLibraryService);

    firelight::ServiceAccessor::setLibraryService(&userLibraryService);

    firelight::mods::SqliteModRepository modRepository;
    firelight::ServiceAccessor::setModRepository(&modRepository);

    // Backs SettingsService (below) via the std-typed ISettingsRepository. Not a
    // QObject and not exposed to QML — the GUI reads settings through
    // SettingsService, not this repository.
    firelight::settings::SqliteSettingsRepository emulationSettingsManager(
        (defaultAppDataPathString + "/settings.db").toStdString());

    // Caches each core's declared options (populated after a core loads) so the
    // advanced options editor can list them without the core running.
    firelight::settings::SqliteCoreOptionRepository coreOptionRepository(
        (defaultAppDataPathString + "/settings.db").toStdString());
    firelight::ServiceAccessor::setCoreOptionRepository(&coreOptionRepository);

    // Per-game cheats (Game Genie / Action Replay), applied on load.
    firelight::cheats::SqliteCheatRepository cheatRepository(
        (defaultAppDataPathString + "/cheats.db").toStdString());
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
    // Resolve relative to the executable, not the working directory, so it loads
    // regardless of where the app is launched from (e.g. `firelight <rom>` from a
    // terminal in any directory).
    const auto catalogPath =
            (QCoreApplication::applicationDirPath() + "/system/settings_catalog.json")
            .toStdString();
    if (!firelight::settings::SettingsCatalog::instance().loadFromFile(
        catalogPath)) {
        spdlog::warn("Could not load settings catalog from {}; using core "
                     "defaults only",
                     catalogPath);
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

    firelight::emulation::EmulationContext emulationContext{
        .inputService = &inputService,
        .achievementManager = &raClient,
        .saveManager = &saveManager,
        .coreOptionRepository = &coreOptionRepository,
        .cheatRepository = &cheatRepository,
        .coreSystemDirectory =
        (defaultAppDataPathString + "/core-system").toStdString(),
    };
    firelight::emulation::EmulationService emuService(userLibraryService,
                                                      entryResolver,
                                                      settingsService,
                                                      emulationContext);
    firelight::emulation::EmulationService::setInstance(&emuService);

    // --- Apply per-launch CLI configuration as session overrides ------------
    // These affect only this run and are never written to the settings DB.
    {
        using firelight::settings::SettingsCatalog;

        // Emulation setting overrides: the bulk --settings-file first, then
        // inline --set on top (explicit inline wins).
        std::vector<std::pair<std::string, std::string> > overrides;
        if (!cliOptions.settingsFile.empty()) {
            try {
                const auto fileOverrides =
                        firelight::cli::loadOverrideFile(cliOptions.settingsFile);
                overrides.insert(overrides.end(), fileOverrides.begin(),
                                 fileOverrides.end());
            } catch (const std::exception &e) {
                spdlog::error("Ignoring --settings-file {}: {}",
                              cliOptions.settingsFile, e.what());
            }
        }
        overrides.insert(overrides.end(), cliOptions.sets.begin(),
                         cliOptions.sets.end());

        if (!overrides.empty()) {
            // Known friendly keys (common + every core's friendly settings) for a
            // typo warning. Raw core option keys can't be listed here, so unknown
            // keys are still applied (they may be valid advanced core options).
            std::set<std::string> knownKeys;
            for (const auto &s: SettingsCatalog::instance().commonSettings()) {
                knownKeys.insert(s.key);
            }
            for (const auto &core: firelight::CoreRegistry::instance().cores()) {
                for (const auto &s:
                     SettingsCatalog::instance().coreSpecificSettings(core.id)) {
                    knownKeys.insert(s.key);
                }
            }
            for (const auto &[key, value]: overrides) {
                if (!knownKeys.contains(key)) {
                    spdlog::warn("--set: '{}' is not a known friendly setting; "
                                 "applying as a raw core option",
                                 key);
                }
                settingsService.setSessionOverride(key, value);
            }
        }

        // Force a specific core for this launch (guarded by platform support in
        // CoreRegistry::resolveCoreName).
        if (!cliOptions.core.empty()) {
            firelight::CoreRegistry::instance().setSessionCoreOverride(
                cliOptions.core);
        }

        // Save slot, muted, and preferred controller apply to the launched game.
        const bool haveRom = startupLaunchEntryId >= 0;
        firelight::emulation::LaunchOverrides launch;
        // Born-muted at instance creation (robust against QML binding timing);
        // StartupOptions also carries it so the QML muted binding stays true.
        launch.muted = cliOptions.muted;
        if (cliOptions.saveSlot >= 0) {
            if (haveRom) {
                launch.saveSlot = cliOptions.saveSlot;
            } else {
                spdlog::warn("--save-slot ignored: no ROM was given to launch");
            }
        }
        emuService.setPendingLaunchOverrides(launch);
        if (!cliOptions.controller.empty()) {
            const auto type =
                    firelight::cli::parseControllerType(cliOptions.controller);
            if (!type.has_value()) {
                spdlog::warn("--controller '{}' is not a known type (valid: {})",
                             cliOptions.controller,
                             firelight::cli::controllerTypeNames());
            } else if (!haveRom) {
                spdlog::warn("--controller ignored: no ROM was given to launch");
            } else if (const auto entry =
                    userLibraryService.getEntry(startupLaunchEntryId)) {
                inputService.setSessionPreferredControllerType(
                    entry->platformId, static_cast<int>(*type));
            }
        }

        if (!cliOptions.raUsername.empty() && cliOptions.raPassword.empty() &&
            cliOptions.raToken.empty()) {
            spdlog::warn("--ra-username given without --ra-password or "
                "--ra-token; skipping startup login");
        }
    }
    // ------------------------------------------------------------------------

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

    engine.rootContext()->setContextProperty(
        "FilesystemUtils", new firelight::gui::FilesystemUtils());
    engine.rootContext()->setContextProperty("EventEmitter",
                                             new firelight::gui::EventEmitter());
    engine.rootContext()->setContextProperty(
        "AudioSettings", new firelight::gui::QtAudioSettingsProxy());
    engine.rootContext()->setContextProperty(
        "CoreRegistry", new firelight::gui::QtCoreRegistryProxy());
    engine.rootContext()->setContextProperty("Router",
                                             new firelight::gui::Router());
    engine.rootContext()->setContextProperty("achievement_manager", &raClient);
    engine.rootContext()->setContextProperty("shop_item_model", &shopItemModel);
    engine.rootContext()->setContextProperty("SaveManager", &saveManagerProxy);
    engine.rootContext()->setContextProperty("ContentDirectoryModel",
                                             &contentDirectoryModel);
    engine.rootContext()->setContextProperty("LibraryEntryModel",
                                             &entryListModel);
    engine.rootContext()->setContextProperty("PlatformModel",
                                             &platformListModel);

    const auto activityBucketsModel = new firelight::gui::ActivityBucketsListModel();
    engine.rootContext()->setContextProperty("ActivityBucketsModel",
                                             activityBucketsModel);

    const auto searchResultsModel = new firelight::gui::SearchResultsListModel(
        userLibraryService, firelight::platforms::PlatformService::getInstance());
    engine.rootContext()->setContextProperty("SearchResultsModel", searchResultsModel);


    engine.rootContext()->setContextProperty(
        "InputService", new firelight::gui::QtInputServiceProxy(inputService));
    engine.rootContext()->setContextProperty(
        "EmulationService", new firelight::gui::QtEmulationServiceProxy());
    engine.rootContext()->setContextProperty(
        "AchievementService",
        new firelight::gui::QtAchievementServiceProxy(achievementService));

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

    // Startup values from the CLI (e.g. a ROM to auto-launch, per-launch window
    // knobs, a pending RetroAchievements login). Registered before loadFromModule
    // so the root window sees it in Component.onCompleted.
    firelight::cli::StartupOptions::Data startupData;
    startupData.launchEntryId = startupLaunchEntryId;
    startupData.startMuted = cliOptions.muted;
    startupData.startPaused = cliOptions.paused;
    startupData.fullscreenOverride =
            cliOptions.fullscreen ? 1 : (cliOptions.windowed ? 0 : -1);
    startupData.exitOnClose = cliOptions.exitOnClose;
    startupData.raPendingLogin =
            !cliOptions.raUsername.empty() &&
            (!cliOptions.raPassword.empty() || !cliOptions.raToken.empty());
    startupData.raUsername = QString::fromStdString(cliOptions.raUsername);
    startupData.raPassword = QString::fromStdString(cliOptions.raPassword);
    startupData.raToken = QString::fromStdString(cliOptions.raToken);
    engine.rootContext()->setContextProperty(
        "StartupOptions",
        new firelight::cli::StartupOptions(std::move(startupData)));

    // The single-instance listener (null unless --single-instance). The root
    // window connects to its launchRequested(entryId) to play a forwarded ROM.
    engine.rootContext()->setContextProperty(
        "SingleInstance", singleInstanceServer.get());

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("QMLFirelight", "Main3");

    QObject *rootObject = engine.rootObjects().value(0);
    auto window = qobject_cast<QQuickWindow *>(rootObject);

    // Give Qt's Vulkan renderer a shared instance that enables the external
    // memory/semaphore/fence *capabilities* extensions. HW-render cores that
    // request only Vulkan 1.0 (PPSSPP) resolve their shared-image negotiation
    // through the KHR entry points and crash if those extensions aren't enabled;
    // cores that request 1.1+ (parallel-RDP) get them as core functions. The
    // apiVersion must stay >= 1.1 so those 1.1 cores keep working. Must be set
    // before the window is exposed. Static so it outlives the window.
    static QVulkanInstance vulkanInstance;
    if (window) {
      vulkanInstance.setApiVersion(QVersionNumber(1, 3));
      const QByteArrayList wanted = {
          "VK_KHR_surface",
          "VK_KHR_win32_surface",
          "VK_KHR_get_physical_device_properties2",
          "VK_KHR_external_memory_capabilities",
          "VK_KHR_external_semaphore_capabilities",
          "VK_KHR_external_fence_capabilities",
          "VK_EXT_swapchain_colorspace",
          "VK_EXT_debug_utils"};
      const auto supported = vulkanInstance.supportedExtensions();
      QByteArrayList enable;
      for (const auto &w : wanted) {
        for (const auto &s : supported) {
          if (s.name == w) {
            enable << w;
            break;
          }
        }
      }
      vulkanInstance.setExtensions(enable);
      if (!vulkanInstance.create()) {
        spdlog::error("Failed to create shared QVulkanInstance (VkResult {})",
                      static_cast<int>(vulkanInstance.errorCode()));
      } else {
        window->setVulkanInstance(&vulkanInstance);
      }
    }

    window->installEventFilter(resizeHandler);
    window->installEventFilter(inputMethodDetectionHandler);

    auto keyboardHandler = new firelight::input::KeyboardInputHandler();
    window->installEventFilter(keyboardHandler);

    inputService.setKeyboard(
        std::shared_ptr<firelight::input::IGamepad>(keyboardHandler));

    auto inputLoopFuture = QtConcurrent::run([&] { inputService.run(); });

    // discordManager is declared earlier (so it outlives the engine); just
    // initialize it now that the window/render loop exists.
    discordManager.initialize();

    QObject::connect(window, &QQuickWindow::afterRendering,
                     [&]() { discordManager.runCallbacks(); });

    window->setIcon(QIcon(":/images/app-icon"));

    engine.rootContext()->setContextProperty("sfx_player",
                                             new firelight::audio::SfxPlayer());

    int exitCode = QApplication::exec();

    spdlog::info("Exiting QApplication");

    // Stop any running game before the QML engine (and its EmulatorItem, which
    // is the core's video receiver) is torn down, so the core is destroyed while
    // the receiver is still alive. This mirrors the in-app "stop game" path;
    // otherwise the engine is destroyed first and the core's destructor
    // dereferences the freed receiver (crash on exit while a game is running).
    emuService.stopEmulation();

    inputService.stop();
    inputLoopFuture.waitForFinished();

    // engine.removeImageProvider("gameImages");
    // TODO: Let daemons finish

    return exitCode;
}
