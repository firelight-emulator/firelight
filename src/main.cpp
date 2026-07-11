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
#include <QTimer>
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
#include "app/audio/audio_manager.hpp"
#include "app/audio/qt_microphone.hpp"
#include <firelight/saves/sqlite_save_database.hpp>
#include "app/input/gui/analog_settings_model.hpp"
#include "app/input/gui/binding_list_model.hpp"
#include "app/input/gui/controller_list_model.hpp"
#include "app/input/gui/platform_input_preferences.hpp"
#include "app/input/gui/profile_list_model.hpp"
#include <firelight/input/shortcut_catalog.hpp>
#include <firelight/input/sqlite_controller_repository.hpp>
#include <firelight/cheats/sqlite_cheat_repository.hpp>
#include "app/emulation/emulation_context.hpp"
#include "app/emulation/emulation_service.hpp"
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
#include <firelight/media/sqlite_game_capture_repository.hpp>
#include "app/media/gui/capture_list_model.hpp"
#include "app/netplay/gui/netplay_chat_model.hpp"
#include "app/netplay/gui/netplay_slots_model.hpp"
#include "app/netplay/gui/netplay_stream_item.hpp"
#include "app/netplay/netplay_service.hpp"
#include "gui/eventhandlers/input_method_detection_handler.hpp"
#include "gui/eventhandlers/window_resize_handler.hpp"
#include "gui/game_image_provider.hpp"
#include "gui/models/shop/shop_item_model.hpp"
#include "gui/platform_list_model.hpp"
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
#include <firelight/metadata/sqlite_game_metadata_source.hpp>
#include <firelight/metadata/sqlite_media_asset_repository.hpp>
#include <firelight/metadata/steamgriddb_art_provider.hpp>
#include "metadata/cpr_http_client.hpp"
#include "metadata/metadata_service.hpp"
#include "gui/qt_game_art_proxy.hpp"
#include <firelight/event_dispatcher.hpp>
#include <firelight/mods/sqlite_mod_repository.hpp>
#include "app/mods/gui/ModInfoItem.hpp"
#include "app/saves/gui/suspend_points_item.hpp"
#include "gui/EventEmitter.h"
#include "gui/filesystem_utils.hpp"
#include "gui/image_utils.hpp"
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
#include "gui/qt_network_service_proxy.hpp"
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
#include "app/netplay/direct_lobby_backend.hpp"
#include "app/netplay/tee_audio_output.hpp"
#include <firelight/discord/discord_manager_impl.hpp>
#include <firelight/netplay/rtc_transport.hpp>

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

    // ===== CLI parsing ===============================================================
    const auto cliOptions = firelight::cli::parseCli(argc, argv);
    if (cliOptions.action == firelight::cli::CliAction::Exit) {
        return cliOptions.exitCode;
    }
    if (cliOptions.verbose) {
        spdlog::set_level(spdlog::level::debug);
    }

    // Handle subcommands that don't need GUI and exit immediately
    if (cliOptions.action == firelight::cli::CliAction::RunScan) {
        return runScan(argc, argv, cliOptions);
    }
    if (cliOptions.action == firelight::cli::CliAction::Login) {
        return runLogin(argc, argv, cliOptions);
    }
    if (cliOptions.action == firelight::cli::CliAction::ListSettings) {
        return firelight::cli::runListSettings(argc, argv);
    }
    if (cliOptions.action == firelight::cli::CliAction::ListCores) {
        return firelight::cli::runListCores(argc, argv);
    }

    // ===== Set up QApplication ===============================================================
    QSurfaceFormat format;
    format.setSwapInterval(0);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/images/app-icon"));

    std::signal(SIGINT, [](int signal) { QApplication::quit(); });

    // ===== Resolve data directories ===============================================================
    const auto dataDirs = resolveDataDirs(cliOptions);
    auto docsPath = dataDirs.docsPath;
    auto defaultAppDataPathString = dataDirs.appDataPath;
    auto savesPath = dataDirs.savesPath;
    auto romsPath = dataDirs.romsPath;
    auto capturesPath = dataDirs.capturesPath;
    auto coreSystemPath = dataDirs.coreSystemPath;

    // Set path to settings file
    QSettings::setPath(QSettings::Format::IniFormat, QSettings::Scope::UserScope,
                       defaultAppDataPathString);

    // Create directories if they don't exist
    if (!QFileInfo::exists(defaultAppDataPathString) && QDir().mkpath(defaultAppDataPathString)) {
        spdlog::info("[Startup] App data directory does not exist; creating: {}",
                     defaultAppDataPathString.toStdString());
    }

    if (!QFileInfo::exists(coreSystemPath) && QDir().mkpath(coreSystemPath)) {
        spdlog::info("[Startup] Core system directory does not exist; creating: {}",
                     coreSystemPath.toStdString());
    }

    if (!QFileInfo::exists(docsPath) && QDir().mkpath(docsPath)) {
        spdlog::info("[Startup] Documents directory does not exist; creating: {}",
                     docsPath.toStdString());
    }

    if (!QFileInfo::exists(savesPath) && QDir().mkpath(savesPath)) {
        spdlog::info("[Startup] Saves directory does not exist; creating: {}", savesPath.toStdString());
    }

    if (!QFileInfo::exists(romsPath) && QDir().mkpath(romsPath)) {
        spdlog::info("[Startup] Content directory does not exist; creating: {}", romsPath.toStdString());
    }

    if (!QFileInfo::exists(capturesPath) && QDir().mkpath(capturesPath)) {
        spdlog::info("[Startup] Captures directory does not exist; creating: {}", capturesPath.toStdString());
    }

    // ===== Check for single-instance mode =======================================================
    // If the user requested single-instance mode, check if another instance is already running.
    // If so, forward the launch request to that instance and exit. Otherwise, continue launching
    // this instance and a little bit later we'll start listening for incoming launch requests from
    // future instances.
    // ============================================================================================
    const auto singleInstanceName =
            firelight::cli::singleInstanceServerName(defaultAppDataPathString);
    if (cliOptions.singleInstance &&
        forwardLaunchToRunningInstance(singleInstanceName, cliOptions)) {
        return 0;
    }

    // ===== Create and register services ===========================================================

    // Platform service
    firelight::platforms::PlatformService platformService;
    firelight::ServiceAccessor::setPlatformService(&platformService);

    // Media service (writes captures to disk + indexes them in captures.db)
    firelight::media::SqliteGameCaptureRepository gameCaptureRepository(
        (defaultAppDataPathString + "/captures.db").toStdString());
    firelight::media::MediaService mediaService(capturesPath,
                                                gameCaptureRepository);
    firelight::ServiceAccessor::setMediaService(&mediaService);
    // Sync the index with the captures folder (picks up files added/removed
    // outside the app; regenerates any missing clip posters).
    mediaService.reconcile();

    // Discord service
    firelight::discord::DiscordManager discordManager(platformService);
    firelight::ServiceAccessor::setDiscordManager(&discordManager);

    // Activity service
    const auto activityDbPath = defaultAppDataPathString + "/activity.db";
    firelight::activity::SqliteActivityLog activityLog(activityDbPath);
    firelight::ServiceAccessor::setActivityService(&activityLog);

    const auto userdataDbPath = defaultAppDataPathString + "/userdata.db";
    firelight::saves::SqliteSaveDatabase saveDatabase(
        userdataDbPath.toStdString());

    // Save data service. The persisted save-directory override lives in the app
    // layer now (the Qt-free SaveManager no longer owns QSettings); resolve it
    // here and pass the result in. QtSaveManagerProxy writes it back on change.
    QSettings savesSettings;
    const auto resolvedSaveDir =
        savesSettings.value("Saves/SaveDirectory", savesPath)
            .toString()
            .toStdString();
    firelight::saves::SaveManager saveManager(resolvedSaveDir, saveDatabase);
    firelight::ServiceAccessor::setSaveManager(&saveManager);

    // Achievement service
    const auto achievementDbPath = (defaultAppDataPathString + "/rcheevos3.db").toStdString();
    firelight::achievements::SqliteAchievementRepository achievementRepo(achievementDbPath);
    firelight::achievements::AchievementService achievementService(
        achievementRepo);

    firelight::achievements::RetroAchievementsOfflineClient offlineRaClient(
        achievementService);
    firelight::achievements::RAClient raClient(offlineRaClient,
                                               achievementService);
    firelight::ServiceAccessor::setAchievementManager(&raClient);
    firelight::ServiceAccessor::setAchievementService(&achievementService);

    // Library service
    const auto libraryDbPath = defaultAppDataPathString + "/library.db";
    firelight::library::SqliteUserLibraryRepository userLibrary(libraryDbPath);

    // Drives content-file -> run-configuration -> entry orchestration off the
    // repository's events. Must outlive scanning.
    firelight::library::LibraryIngestService libIngestService(userLibrary);

    // Auto-populates entry name/metadata/art from the shipped offline metadata
    // DB when a game is added (and backfills existing entries). Constructed
    // before scanning so it catches the initial scan's new entries; the read-only
    // metadata source tolerates a missing shipped DB.
    firelight::metadata::SqliteGameMetadataSource gameMetadataSource(
        (defaultAppDataPathString + "/metadata.db").toStdString());
    firelight::metadata::SqliteMediaAssetRepository mediaAssetRepository(
        (defaultAppDataPathString + "/media.db").toStdString());
    firelight::metadata::MetadataService metadataService(
        userLibrary, gameMetadataSource, mediaAssetRepository,
        (defaultAppDataPathString + "/media").toStdString());
    metadataService.backfillMissing();

    // Optional online art provider backing the "Change artwork" picker. The key
    // is a user setting (empty until supplied); the provider stays unconfigured
    // and no-ops until then. cpr lives here in the app so the metadata module
    // stays HTTP-dependency-free.
    firelight::metadata::CprHttpClient cprHttpClient;
    firelight::metadata::SteamGridDbArtProvider steamGridDbArtProvider(
        cprHttpClient, "");

    firelight::library::LibraryScanner2 libScanner2(userLibrary,
                                                    platformService);

    firelight::library::EntryResolver entryResolver(userLibrary);
    firelight::library::UserLibraryService userLibraryService(
        userLibrary, romsPath.toStdString());

    // Input service
    const auto controllerRepositoryDbPath = defaultAppDataPathString + "/controllers.db";
    firelight::input::SqliteControllerRepository controllerRepository(
        controllerRepositoryDbPath, platformService);

    firelight::input::registerDefaultShortcuts();

    firelight::input::SDLInputService inputService(controllerRepository);
    firelight::ServiceAccessor::setInputService(&inputService);
    firelight::ServiceAccessor::setControllerProfileRepository(
        &controllerRepository);

    // Online play: direct-connection lobby (host shares their IP) + WebRTC
    // data plane + session. DiscordLobbyBackend can swap back in here once the
    // app's OAuth client is configured in the Discord developer portal.
    firelight::netplay::DirectLobbyBackend netplayLobbyBackend;
    firelight::netplay::RtcTransport netplayTransport;
    firelight::netplay::NetplayService netplayService(
        netplayLobbyBackend, netplayTransport, userLibraryService, FL_VERSION,
        &raClient, &inputService,
        [] { return std::make_shared<AudioManager>(); });

    // Settings service
    firelight::settings::SqliteSettingsRepository settingsRepository(
        (defaultAppDataPathString + "/settings.db").toStdString());

    firelight::settings::SettingsService settingsService(
        settingsRepository);
    firelight::settings::SettingsService::setInstance(&settingsService);

    // Caches each core's declared options (populated after a core loads) so the
    // advanced options editor can list them without the core running.
    firelight::settings::SqliteCoreOptionRepository coreOptionRepository(
        (defaultAppDataPathString + "/settings.db").toStdString());
    firelight::ServiceAccessor::setCoreOptionRepository(&coreOptionRepository);

    // Cheat service
    firelight::cheats::SqliteCheatRepository cheatRepository(
        (defaultAppDataPathString + "/cheats.db").toStdString());

    // ===== Create Qt proxy (glue) services =====================================================
    firelight::gui::QtSaveManagerProxy saveManagerProxy(saveManager);
    firelight::gui::QtInputServiceProxy inputServiceProxy(inputService);
    firelight::gui::QtAchievementServiceProxy achievementServiceProxy(achievementService);
    firelight::gui::QtGameArtProxy gameArtProxy(
        metadataService, steamGridDbArtProvider, mediaAssetRepository);

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

    // QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

    auto gameImageProvider = new firelight::gui::GameImageProvider();
    firelight::ServiceAccessor::setGameImageProvider(gameImageProvider);

    // Thin QML adapter over the (Qt-notification-free) save manager; exposes the
    // save directory as a bindable property for the settings UI. Declared here so
    // it outlives the QML engine registered below.

    // TODO: Move this to before service?
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

    // Pause library scanning while a game is running so background hashing/IO
    // never contends with the emulator; resume (and catch up) when it stops.
    const auto scanSuspendConn = EventDispatcher::instance().subscribe<
        firelight::emulation::EmulationStartedEvent>(
        [&](const firelight::emulation::EmulationStartedEvent &) {
            libScanner2.setScanningSuspended(true);
        });
    const auto scanResumeConn = EventDispatcher::instance().subscribe<
        firelight::emulation::EmulationStoppedEvent>(
        [&](const firelight::emulation::EmulationStoppedEvent &) {
            libScanner2.setScanningSuspended(false);
        });

    // The app-facing curation surface; also guarantees the default content
    // directory exists and is watched (fires the add event above when seeding).

    libScanner2.scanAll();

    // A ROM path passed on the command line resolves to a library entry that the
    // root window auto-launches once loaded (-1 when absent/unresolved).
    const int startupLaunchEntryId =
            firelight::cli::resolveRomEntryId(cliOptions.romPath, userLibraryService,
                                              platformService);

    // If we're the primary --single-instance process, start listening for
    // launches forwarded from secondary processes. Exposed to QML below so the
    // root window turns launchRequested into window.startGame(entryId).
    std::unique_ptr<firelight::cli::SingleInstanceServer> singleInstanceServer;
    if (cliOptions.singleInstance) {
        singleInstanceServer =
                std::make_unique<firelight::cli::SingleInstanceServer>(
                    singleInstanceName, userLibraryService, platformService);
        singleInstanceServer->start();
    }

    // Set up the models for QML
    // ***********************************************
    firelight::library::EntryListModel entryListModel(
        userLibraryService, activityLog, platformService);

    // Gallery model over the capture index (screenshots + clips).
    firelight::gui::CaptureListModel captureListModel(gameCaptureRepository,
                                                      userLibraryService);

    // No scanFinished -> reset: the model keeps itself in sync incrementally via
    // EntryCreatedEvent/EntryUpdatedEvent (see EntryListModel), so scans update
    // the list in place without a full reset (no flicker / scroll loss).

    firelight::gui::PlatformListModel platformListModel(platformService);
    // firelight::shop::ShopItemModel shopItemModel(contentDatabase);

    firelight::gui::ContentDirectoryModel contentDirectoryModel(userLibraryService);

    firelight::ServiceAccessor::setLibraryService(&userLibraryService);

    firelight::mods::SqliteModRepository modRepository;
    firelight::ServiceAccessor::setModRepository(&modRepository);

    // Backs SettingsService (below) via the std-typed ISettingsRepository. Not a
    // QObject and not exposed to QML — the GUI reads settings through
    // SettingsService, not this repository.

    // Per-game cheats (Game Genie / Action Replay), applied on load.
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
    qmlRegisterType<firelight::gui::NetplayStreamItem>("Firelight", 1, 0,
                                                       "NetplayStreamItem");
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

    firelight::emulation::EmulationContext emulationContext{
        .inputService = &inputService,
        .achievementManager = &raClient,
        .saveManager = &saveManager,
        .coreOptionRepository = &coreOptionRepository,
        .cheatRepository = &cheatRepository,
        .platformService = &platformService,
        .coreSystemDirectory = dataDirs.coreSystemPath.toStdString(),
        .retropadProvider = &netplayService.retropadProvider(),
        .netplayStreamSink = &netplayService.streamSender(),
        // The tee mirrors PCM into the netplay stream (a no-op unless a host
        // stream is armed) on its way to the real output.
        .audioOutputFactory =
        [&netplayService] {
            return std::make_shared<firelight::netplay::TeeAudioOutput>(
                std::make_shared<AudioManager>(),
                &netplayService.streamSender());
        },
        .audioInputFactory =
        [] { return std::make_unique<firelight::audio::QtMicrophone>(); }
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
    engine.rootContext()->setContextProperty(
        "ImageUtils", new firelight::gui::ImageUtils());
    engine.rootContext()->setContextProperty("EventEmitter",
                                             new firelight::gui::EventEmitter());
    engine.rootContext()->setContextProperty(
        "AudioSettings", new firelight::gui::QtAudioSettingsProxy());
    engine.rootContext()->setContextProperty(
        "CoreRegistry", new firelight::gui::QtCoreRegistryProxy());
    engine.rootContext()->setContextProperty("achievement_manager", &raClient);
    // engine.rootContext()->setContextProperty("shop_item_model", &shopItemModel);
    engine.rootContext()->setContextProperty("SaveManager", &saveManagerProxy);
    engine.rootContext()->setContextProperty("ContentDirectoryModel",
                                             &contentDirectoryModel);
    engine.rootContext()->setContextProperty("LibraryEntryModel",
                                             &entryListModel);
    engine.rootContext()->setContextProperty("CaptureModel", &captureListModel);
    engine.rootContext()->setContextProperty("PlatformModel",
                                             &platformListModel);

    firelight::gui::QtNetworkServiceProxy networkServiceProxy(netplayService);
    firelight::gui::NetplaySlotsModel netplaySlotsModel(netplayService);
    firelight::gui::NetplayChatModel netplayChatModel(netplayService);
    QObject::connect(&networkServiceProxy,
                     &firelight::gui::QtNetworkServiceProxy::slotsChanged,
                     &netplaySlotsModel,
                     &firelight::gui::NetplaySlotsModel::refresh);
    QObject::connect(&networkServiceProxy,
                     &firelight::gui::QtNetworkServiceProxy::lobbyStateChanged,
                     &netplaySlotsModel,
                     &firelight::gui::NetplaySlotsModel::refresh);
    QObject::connect(&networkServiceProxy,
                     &firelight::gui::QtNetworkServiceProxy::chatChanged,
                     &netplayChatModel,
                     &firelight::gui::NetplayChatModel::refresh);
    engine.rootContext()->setContextProperty("NetworkService",
                                             &networkServiceProxy);
    engine.rootContext()->setContextProperty("NetplaySlotsModel",
                                             &netplaySlotsModel);
    engine.rootContext()->setContextProperty("NetplayChatModel",
                                             &netplayChatModel);

    const auto activityBucketsModel = new firelight::gui::ActivityBucketsListModel();
    engine.rootContext()->setContextProperty("ActivityBucketsModel",
                                             activityBucketsModel);

    const auto searchResultsModel = new firelight::gui::SearchResultsListModel(
        userLibraryService, platformService);
    engine.rootContext()->setContextProperty("SearchResultsModel", searchResultsModel);


    engine.rootContext()->setContextProperty("InputService", &inputServiceProxy);
    engine.rootContext()->setContextProperty(
        "EmulationService", new firelight::gui::QtEmulationServiceProxy());
    engine.rootContext()->setContextProperty("AchievementService", &achievementServiceProxy);
    engine.rootContext()->setContextProperty("GameArtService", &gameArtProxy);

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
    engine.loadFromModule("QMLFirelight", "Main4");

    QObject *rootObject = engine.rootObjects().value(0);
    auto window = qobject_cast<QQuickWindow *>(rootObject);

    // Give Qt's Vulkan renderer a shared instance that enables the external
    // memory/semaphore/fence *capabilities* extensions. HW-render cores that
    // request only Vulkan 1.0 (PPSSPP) resolve their shared-image negotiation
    // through the KHR entry points and crash if those extensions aren't enabled;
    // cores that request 1.1+ (parallel-RDP) get them as core functions. The
    // apiVersion must stay >= 1.1 so those 1.1 cores keep working. Must be set
    // before the window is exposed. Static so it outlives the window.
    // static QVulkanInstance vulkanInstance;
    // if (window) {
    //     vulkanInstance.setApiVersion(QVersionNumber(1, 3));
    //     const QByteArrayList wanted = {
    //         "VK_KHR_surface",
    //         "VK_KHR_win32_surface",
    //         "VK_KHR_get_physical_device_properties2",
    //         "VK_KHR_external_memory_capabilities",
    //         "VK_KHR_external_semaphore_capabilities",
    //         "VK_KHR_external_fence_capabilities",
    //         "VK_EXT_swapchain_colorspace",
    //         "VK_EXT_debug_utils"
    //     };
    //     const auto supported = vulkanInstance.supportedExtensions();
    //     QByteArrayList enable;
    //     for (const auto &w: wanted) {
    //         for (const auto &s: supported) {
    //             if (s.name == w) {
    //                 enable << w;
    //                 break;
    //             }
    //         }
    //     }
    //     vulkanInstance.setExtensions(enable);
    //     if (!vulkanInstance.create()) {
    //         spdlog::error("Failed to create shared QVulkanInstance (VkResult {})",
    //                       static_cast<int>(vulkanInstance.errorCode()));
    //     } else {
    //         window->setVulkanInstance(&vulkanInstance);
    //     }
    // }

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

    // Pump SDK callbacks on the main thread. A render-driven pump would stall
    // on static scenes, freezing lobby/chat events while idling in menus.
    QTimer discordCallbackTimer;
    discordCallbackTimer.setInterval(16);
    QObject::connect(&discordCallbackTimer, &QTimer::timeout,
                     [&] { discordManager.runCallbacks(); });
    discordCallbackTimer.start();

    window->setIcon(QIcon(":/images/app-icon"));

    engine.rootContext()->setContextProperty("sfx_player",
                                             new firelight::audio::SfxPlayer());

    const auto richPresenceMessageChangedSubscriber = EventDispatcher::instance().subscribe<
        firelight::achievements::RichPresenceMessageChangedEvent>(
        [&](const firelight::achievements::RichPresenceMessageChangedEvent &e) {
            discordManager.setRichPresenceMessage(e.newRichPresenceMessage);
        });

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
