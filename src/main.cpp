#define SDL_MAIN_HANDLED

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

#ifdef Q_OS_WIN
#include <dwmapi.h>
#include <windows.h>
#endif

#include "activity/gui/game_activity_item.hpp"
#include "app/audio/SfxPlayer.hpp"
#include "app/db/sqlite_content_database.hpp"
#include "app/db/sqlite_userdata_database.hpp"
#include "app/input/controller_manager.hpp"
#include "app/input/gui/controller_list_model.hpp"
#include "app/input/sdl_event_loop.hpp"
#include "app/input/sqlite_controller_repository.hpp"
#include "app/library/gui/playlist_item_model.hpp"
#include "app/library/library_scanner2.hpp"
#include "app/rcheevos/ra_client.hpp"
#include "app/saves/save_manager.hpp"
#include "gui/eventhandlers/input_method_detection_handler.hpp"
#include "gui/eventhandlers/window_resize_handler.hpp"
#include "gui/game_image_provider.hpp"
#include "gui/models/shop/shop_item_model.hpp"
#include "gui/platform_list_model.hpp"
#include "gui/router.hpp"
#include "network_cache.hpp"

#include <QtConcurrent>
#include <archive.h>
#include <archive_entry.h>
#include <discord/discord.h>

#include "app/PlatformMetadataItem.hpp"
#include "app/activity/sqlite_activity_log.hpp"
#include "app/emulator_item.hpp"
#include "app/input/gui/gamepad_status_item.hpp"
#include "app/input/gui/input_mapping_item.hpp"
#include "app/input/gui/keyboard_mapping_item.hpp"
#include "app/library/gui/entry_list_model.hpp"
#include "app/library/gui/library_entry_item.hpp"
#include "app/library/gui/library_path_model.hpp"
#include "app/library/sqlite_user_library.hpp"
#include "app/mods/SqliteModRepository.h"
#include "app/mods/gui/ModInfoItem.hpp"
#include "app/saves/gui/suspend_points_item.hpp"
#include "gui/EventEmitter.h"
#include "settings/sqlite_emulation_settings_manager.hpp"
#include <discordpp.h>

#include <settings/gui/game_settings_item.hpp>
#include <unistd.h>

bool create_dirs(const std::initializer_list<std::filesystem::path> list) {
  std::error_code error_code;
  for (const auto &path : list) {
    if (!exists(path)) {
      spdlog::info("Directory not found, creating: {}", path.string());
      if (!create_directories(path, error_code)) {
        spdlog::error("Unable to create directory {}; Error code: {}",
                      path.string(), error_code.message());
        return false;
      }
    }
  }
  return true;
}

int main(int argc, char *argv[]) {
  // SDL_setenv("QT_QUICK_FLICKABLE_WHEEL_DECELERATION", "5000", true);

  // discord::Core *core{};
  // auto result = discord::Core::Create(
  //     1208162396921929739, DiscordCreateFlags_NoRequireDiscord, &core);
  // if (result == discord::Result::Ok) {
  //   spdlog::info("Discord core created");
  // } else {
  //   spdlog::error("Discord core creation failed: {}", (int)result);
  // }

  if (auto debug = std::getenv("FL_DEBUG"); debug != nullptr) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  QGuiApplication::setOrganizationDomain("firelight-emulator.com");
  QGuiApplication::setApplicationName("Firelight");

  QSettings::setDefaultFormat(QSettings::Format::IniFormat);

  // QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  // QSurfaceFormat format;
  // format.setProfile(QSurfaceFormat::OpenGLContextProfile::CompatibilityProfile);
  // format.setVersion(4, 1);
  // QSurfaceFormat::setDefaultFormat(format);

  QSurfaceFormat format;
  format.setSwapInterval(0);
  QSurfaceFormat::setDefaultFormat(format);

  QGuiApplication app(argc, argv);

  std::signal(SIGINT, [](int signal) { QGuiApplication::quit(); });

  // TODO: Check for portable mode marker file

  auto defaultUserPathString =
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  auto docsPath =
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
      "/Firelight";
  auto savesPath = docsPath + "/saves";
  auto romsPath = docsPath + "/roms";

  auto defaultAppDataPathString =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

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
  firelight::input::ControllerManager controllerManager(controllerRepository);
  firelight::input::InputManager inputManager;
  firelight::ManagerAccessor::setInputManager(&inputManager);

  firelight::ManagerAccessor::setControllerManager(&controllerManager);
  firelight::ManagerAccessor::setControllerRepository(&controllerRepository);

  controllerManager.refreshControllerList();
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  firelight::db::SqliteUserdataDatabase userdata_database(
      defaultAppDataPathString + "/userdata.db");
  firelight::ManagerAccessor::setUserdataManager(&userdata_database);

  firelight::activity::SqliteActivityLog activityLog(defaultAppDataPathString +
                                                     "/activity.db");
  firelight::ManagerAccessor::setActivityLog(&activityLog);

  auto gameImageProvider = new firelight::gui::GameImageProvider();
  firelight::ManagerAccessor::setGameImageProvider(gameImageProvider);

  //   **** Load Content Database ****
  firelight::db::SqliteContentDatabase contentDatabase(
      defaultAppDataPathString + "/content.db");

  firelight::saves::SaveManager saveManager(savesPath, userdata_database,
                                            *gameImageProvider);
  firelight::ManagerAccessor::setSaveManager(&saveManager);

  firelight::library::SqliteUserLibrary userLibrary(
      defaultAppDataPathString + "/library.db", romsPath);

  //   if (userLibrary.getWatchedDirectories().empty()) {
  //     firelight::library::WatchedDirectory dir{
  //       .path = QString::fromStdString(canonical(romsDir).string())
  //     };
  //
  //     userLibrary.addWatchedDirectory(dir);
  //   }

  firelight::ManagerAccessor::setUserLibrary(&userLibrary);

  QObject::connect(
      &userLibrary, &firelight::library::SqliteUserLibrary::romFileAdded,
      &userLibrary, [&](int id, const QString &contentHash) -> void {
        spdlog::info("Rom file added: {}", contentHash.toStdString());
      });

  QObject::connect(
      &userLibrary, &firelight::library::SqliteUserLibrary::entryCreated,
      &userLibrary, [&](int id, const QString &contentHash) -> void {
        spdlog::info("Entry created: {}", contentHash.toStdString());
      });

  firelight::library::LibraryScanner2 libScanner2(userLibrary);
  libScanner2.scanAll();

  firelight::achievements::RetroAchievementsCache raCache(
      defaultAppDataPathString + "/rcheevos.db");
  firelight::achievements::RetroAchievementsOfflineClient offlineRaClient(
      raCache);
  firelight::achievements::RAClient raClient(offlineRaClient, raCache);
  firelight::ManagerAccessor::setAchievementManager(&raClient);

  // Set up the models for QML ***********************************************
  firelight::gui::ControllerListModel controllerListModel(controllerManager);

  firelight::library::EntryListModel entryListModel(userLibrary);

  QSortFilterProxyModel entrySearchModel;
  entrySearchModel.setSourceModel(&entryListModel);

  QObject::connect(&libScanner2,
                   &firelight::library::LibraryScanner2::scanFinished,
                   &entryListModel, &firelight::library::EntryListModel::reset,
                   Qt::QueuedConnection);

  firelight::gui::PlatformListModel platformListModel;
  firelight::shop::ShopItemModel shopItemModel(contentDatabase);

  auto emulatorConfigManager =
      std::make_shared<EmulatorConfigManager>(userdata_database);
  firelight::ManagerAccessor::setEmulatorConfigManager(emulatorConfigManager);

  firelight::mods::SqliteModRepository modRepository;
  firelight::ManagerAccessor::setModRepository(&modRepository);

  firelight::settings::SqliteEmulationSettingsManager emulationSettingsManager(
      defaultAppDataPathString + "/settings.db");
  firelight::ManagerAccessor::setEmulationSettingsManager(
      &emulationSettingsManager);
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

  qmlRegisterType<EmulatorItem>("Firelight", 1, 0, "EmulatorItem");
  qmlRegisterType<firelight::input::GamepadStatusItem>("Firelight", 1, 0,
                                                       "GamepadStatus");
  qmlRegisterType<firelight::input::KeyboardMappingItem>("Firelight", 1, 0,
                                                         "KeyboardMapping");
  qmlRegisterType<firelight::input::InputMappingItem>("Firelight", 1, 0,
                                                      "InputMapping");
  qmlRegisterType<firelight::PlatformMetadataItem>("Firelight", 1, 0,
                                                   "PlatformMetadata");
  qmlRegisterType<firelight::mods::ModInfoItem>("Firelight", 1, 0, "ModInfo");
  qmlRegisterType<firelight::achievements::AchievementSetItem>(
      "Firelight", 1, 0, "AchievementSet");
  qmlRegisterType<firelight::LibraryEntryItem>("Firelight", 1, 0,
                                               "LibraryEntry");
  qmlRegisterType<firelight::saves::SuspendPointsItem>("Firelight", 1, 0,
                                                       "SuspendPoints");
  qmlRegisterType<firelight::activity::GameActivityItem>("Firelight", 1, 0,
                                                         "GameActivity");

  qmlRegisterType<firelight::settings::GameSettingsItem>("Firelight", 1, 0,
                                                         "GameSettings");

  firelight::gui::Router router;

  QNetworkInformation::loadDefaultBackend();
  if (QNetworkInformation::instance()->reachability() ==
      QNetworkInformation::Reachability::Online) {
    raClient.m_connected = true;
    offlineRaClient.syncOfflineAchievements();
  }

  QObject::connect(
      QNetworkInformation::instance(),
      &QNetworkInformation::reachabilityChanged,
      [&raClient,
       &offlineRaClient](QNetworkInformation::Reachability reachability) {
        if (reachability == QNetworkInformation::Reachability::Online) {
          raClient.m_connected = true;
          offlineRaClient.syncOfflineAchievements();
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

  auto cache = new CachingNetworkAccessManagerFactory();

  QQmlApplicationEngine engine;
  engine.setNetworkAccessManagerFactory(cache);
  // engine.networkAccessManager()->setCache(diskCache);

  // engine.addUrlInterceptor(&imageCacheUrlInterceptor);
  engine.addImageProvider("gameImages", gameImageProvider);

  engine.rootContext()->setContextProperty("EventEmitter",
                                           new firelight::gui::EventEmitter());
  engine.rootContext()->setContextProperty("Router", &router);
  engine.rootContext()->setContextProperty("emulator_config_manager",
                                           emulatorConfigManager.get());
  engine.rootContext()->setContextProperty("achievement_manager", &raClient);
  engine.rootContext()->setContextProperty("controller_model",
                                           &controllerListModel);
  engine.rootContext()->setContextProperty("controller_manager",
                                           &controllerManager);
  engine.rootContext()->setContextProperty("platform_model",
                                           &platformListModel);
  engine.rootContext()->setContextProperty("shop_item_model", &shopItemModel);
  engine.rootContext()->setContextProperty("SaveManager", &saveManager);
  engine.rootContext()->setContextProperty("UserLibrary", &userLibrary);
  engine.rootContext()->setContextProperty("LibraryEntryModel",
                                           &entryListModel);
  engine.rootContext()->setContextProperty("SearchLibraryEntryModel",
                                           &entrySearchModel);
  engine.rootContext()->setContextProperty("LibraryScanner", &libScanner2);

  auto resizeHandler = new firelight::gui::WindowResizeHandler();
  engine.rootContext()->setContextProperty("window_resize_handler",
                                           resizeHandler);
  auto inputMethodDetectionHandler =
      new firelight::gui::InputMethodDetectionHandler();
  engine.rootContext()->setContextProperty("InputMethodManager",
                                           inputMethodDetectionHandler);

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("QMLFirelight", "Main3");

  QObject *rootObject = engine.rootObjects().value(0);
  auto window = qobject_cast<QQuickWindow *>(rootObject);
  window->installEventFilter(resizeHandler);
  window->installEventFilter(inputMethodDetectionHandler);

  firelight::discord::DiscordManager discordManager;
  discordManager.initialize();
  firelight::ManagerAccessor::setDiscordManager(&discordManager);

  QObject::connect(window, &QQuickWindow::afterRendering,
                   [&]() { discordManager.runCallbacks(); });

  // window->setIcon(QIcon("firelight_logo1 (1).svg"));

  firelight::SdlEventLoop sdlEventLoop(window, &controllerManager);
  sdlEventLoop.moveToThread(&sdlEventLoop);
  sdlEventLoop.start();

  engine.rootContext()->setContextProperty("sfx_player",
                                           new firelight::audio::SfxPlayer());

  int exitCode = QGuiApplication::exec();

  spdlog::info("Exiting QGuiApplication");

  sdlEventLoop.stopProcessing();
  //
  sdlEventLoop.quit();
  sdlEventLoop.wait();

  // engine.removeImageProvider("gameImages");
  // TODO: Let daemons finish

  return exitCode;
}
