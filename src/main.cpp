#define SDL_MAIN_HANDLED

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QNetworkInformation>
#include <QQuickWindow>
#include <QWindow>
#include <filesystem>
#include <qstandardpaths.h>
#include <csignal>
#include <spdlog/spdlog.h>
#include <cstdlib>

#include <discord/discord.h>
#include "app/input/sqlite_controller_repository.hpp"
#include "app/rcheevos/ra_client.hpp"
#include "app/db/sqlite_content_database.hpp"
#include "app/db/sqlite_userdata_database.hpp"
#include "gui/router.hpp"
#include "app/audio/SfxPlayer.hpp"
#include "app/input/controller_manager.hpp"
#include "app/input/sdl_event_loop.hpp"
#include "app/library/library_scanner.hpp"
#include "app/library/library_scanner2.hpp"
#include "app/library/sqlite_library_database.hpp"
#include "app/saves/save_manager.hpp"
#include "app/input/controller_list_model.hpp"
#include "gui/game_image_provider.hpp"
#include "gui/eventhandlers/input_method_detection_handler.hpp"
#include "app/library/library_item_model.hpp"
#include "app/library/library_sort_filter_model.hpp"
#include "gui/platform_list_model.hpp"
#include "app/library/playlist_item_model.hpp"
#include "gui/eventhandlers/window_resize_handler.hpp"
#include "gui/models/shop/shop_item_model.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <QtConcurrent>

#include "app/PlatformMetadataItem.hpp"
#include "app/activity/sqlite_activity_log.hpp"
#include "app/emulator_item.hpp"
#include "app/image_cache_url_interceptor.h"
#include "app/input/gamepad_status_item.hpp"
#include "app/input/input_mapping_item.hpp"
#include "app/input/keyboard_mapping_item.hpp"
#include "app/library/sqlite_user_library.hpp"
#include "gui/models/library/entry_list_model.hpp"

bool create_dirs(const std::initializer_list<std::filesystem::path> list) {
  std::error_code error_code;
  for (const auto &path: list) {
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

  discord::Core* core{};
  // discord::Core::Create(1208162396921929739, DiscordCreateFlags_Default, &core);

  if (auto debug = std::getenv("FL_DEBUG"); debug != nullptr) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  QApplication::setOrganizationDomain("firelight-emulator.com");
  QApplication::setApplicationName("Firelight");

  QSettings::setDefaultFormat(QSettings::Format::IniFormat);

  // QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  // QSurfaceFormat format;
  // format.setProfile(QSurfaceFormat::OpenGLContextProfile::CompatibilityProfile);
  // format.setVersion(4, 1);
  // QSurfaceFormat::setDefaultFormat(format);

  QApplication app(argc, argv);

  std::signal(SIGINT, [](int signal) {
    QApplication::quit();
  });

  // TODO: Check for portable mode marker file

  auto defaultUserPathString = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  auto docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Firelight";
  auto savesPath = docsPath + "/saves";
  auto romsPath = docsPath + "/roms";

  auto defaultAppDataPathString = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

  QSettings::setPath(QSettings::Format::IniFormat, QSettings::Scope::UserScope, defaultAppDataPathString);

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

  firelight::ManagerAccessor::setCoreSystemDirectory((defaultAppDataPathString + "/core-system").toStdString());

  firelight::input::SqliteControllerRepository controllerRepository(baseDir.filePath("controllers.db"));
  firelight::input::ControllerManager controllerManager(controllerRepository);
  firelight::input::InputManager inputManager;
  firelight::ManagerAccessor::setInputManager(&inputManager);

  firelight::ManagerAccessor::setControllerManager(&controllerManager);
  firelight::ManagerAccessor::setControllerRepository(&controllerRepository);

  controllerManager.refreshControllerList();
  // QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  firelight::db::SqliteUserdataDatabase userdata_database(defaultAppDataPathString +  "/userdata.db");
  firelight::ManagerAccessor::setUserdataManager(&userdata_database);

  firelight::activity::SqliteActivityLog activityLog(defaultAppDataPathString + "/activity.db");
  firelight::ManagerAccessor::setActivityLog(&activityLog);

  auto gameImageProvider = new firelight::gui::GameImageProvider();
  firelight::ManagerAccessor::setGameImageProvider(gameImageProvider);

  // **** Load Content Database ****
  firelight::db::SqliteContentDatabase contentDatabase(defaultAppDataPathString + "/content.db");

  firelight::db::SqliteLibraryDatabase libraryDatabase(defaultAppDataPathString + "/library.db");
  firelight::ManagerAccessor::setLibraryDatabase(&libraryDatabase);

  firelight::saves::SaveManager saveManager(savesPath, libraryDatabase, userdata_database, *gameImageProvider);
  firelight::ManagerAccessor::setSaveManager(&saveManager);

  firelight::library::SqliteUserLibrary userLibrary(defaultAppDataPathString + "/library.db", romsPath);

  // if (userLibrary.getWatchedDirectories().empty()) {
  //   firelight::library::WatchedDirectory dir{
  //     .path = QString::fromStdString(canonical(romsDir).string())
  //   };
  //
  //   userLibrary.addWatchedDirectory(dir);
  // }

  firelight::ManagerAccessor::setUserLibrary(&userLibrary);

  QObject::connect(&userLibrary, &firelight::library::SqliteUserLibrary::romFileAdded, &userLibrary,
                   [&](int id, const QString &contentHash) -> void {
                     spdlog::info("Rom file added: {}", contentHash.toStdString());
                   });

  QObject::connect(&userLibrary, &firelight::library::SqliteUserLibrary::entryCreated, &userLibrary,
                   [&](int id, const QString &contentHash) -> void {
                     spdlog::info("Entry created: {}", contentHash.toStdString());
                   });

  firelight::library::LibraryScanner2 libScanner2(userLibrary);
  libScanner2.scanAll();

  firelight::achievements::RetroAchievementsCache raCache(defaultAppDataPathString + "/rcheevos.db");
  firelight::achievements::RetroAchievementsOfflineClient offlineRaClient(raCache);
  firelight::achievements::RAClient raClient(contentDatabase, offlineRaClient, raCache);
  firelight::ManagerAccessor::setAchievementManager(&raClient);

  // Set up the models for QML ***********************************************
  firelight::gui::ControllerListModel controllerListModel(controllerManager);
  firelight::gui::PlaylistItemModel playlistModel(&libraryDatabase);
  firelight::gui::LibraryItemModel libModel(&libraryDatabase, &contentDatabase,
                                            &userdata_database);
  firelight::gui::LibrarySortFilterModel libSortModel;
  libSortModel.setSourceModel(&libModel);

  firelight::library::EntryListModel entryListModel(userLibrary);

  QObject::connect(&libScanner2, &firelight::library::LibraryScanner2::scanFinished,
                   &entryListModel, &firelight::library::EntryListModel::reset, Qt::QueuedConnection);

  firelight::gui::LibraryPathModel libraryPathModel(userLibrary);

  firelight::gui::PlatformListModel platformListModel;
  firelight::shop::ShopItemModel shopItemModel(contentDatabase);

  LibraryScanner libraryManager(&libraryDatabase, &contentDatabase);
  firelight::ManagerAccessor::setLibraryManager(&libraryManager);

  auto emulatorConfigManager = std::make_shared<EmulatorConfigManager>(userdata_database);
  firelight::ManagerAccessor::setEmulatorConfigManager(emulatorConfigManager);

  // QObject::connect(
  //   &libraryDatabase,
  //   &firelight::db::SqliteLibraryDatabase::contentDirectoriesUpdated,
  //   &libraryManager, &LibraryScanner::startScan);
  //
  // QObject::connect(&libraryManager, &LibraryScanner::scanFinished, &libModel,
  //                  &firelight::gui::LibraryItemModel::refresh);

  // libraryManager.startScan();

  // qRegisterMetaType<firelight::gui::GamepadMapping>("GamepadMapping");

  qmlRegisterType<EmulatorItem>("Firelight", 1, 0, "EmulatorItem");
  qmlRegisterType<firelight::input::GamepadStatusItem>("Firelight", 1, 0, "GamepadStatus");
  qmlRegisterType<firelight::input::KeyboardMappingItem>("Firelight", 1, 0, "KeyboardMapping");
  qmlRegisterType<firelight::input::InputMappingItem>("Firelight", 1, 0, "InputMapping");
  qmlRegisterType<firelight::PlatformMetadataItem>("Firelight", 1, 0, "PlatformMetadata");

  firelight::gui::Router router;

  QNetworkInformation::loadDefaultBackend();
  spdlog::info("Reachability: {}", (int)QNetworkInformation::instance()->reachability());
  if (QNetworkInformation::instance()->reachability() == QNetworkInformation::Reachability::Online) {
    raClient.m_connected = true;
    offlineRaClient.syncOfflineAchievements();
  }

  QObject::connect(QNetworkInformation::instance(), &QNetworkInformation::reachabilityChanged, [&raClient, &offlineRaClient](QNetworkInformation::Reachability reachability) {
    spdlog::info("Reachability changed: {}", (int)reachability);
    if (reachability == QNetworkInformation::Reachability::Online) {
      raClient.m_connected = true;
      offlineRaClient.syncOfflineAchievements();
    } else {
      raClient.m_connected = false;
    }
  });

  firelight::ImageCacheUrlInterceptor imageCacheUrlInterceptor("");

  QQmlApplicationEngine engine;
  // engine.addUrlInterceptor(&imageCacheUrlInterceptor);
  engine.addImageProvider("gameImages", gameImageProvider);

  engine.rootContext()->setContextProperty("Router", &router);
  engine.rootContext()->setContextProperty("emulator_config_manager", emulatorConfigManager.get());
  engine.rootContext()->setContextProperty("achievement_manager", &raClient);
  engine.rootContext()->setContextProperty("playlist_model", &playlistModel);
  engine.rootContext()->setContextProperty("library_model", &libModel);
  engine.rootContext()->setContextProperty("library_short_model",
                                           &libSortModel);
  engine.rootContext()->setContextProperty("library_manager", &libraryManager);
  engine.rootContext()->setContextProperty("library_database",
                                           &libraryDatabase);
  engine.rootContext()->setContextProperty("library_scan_path_model", &libraryPathModel);
  engine.rootContext()->setContextProperty("controller_model",
                                           &controllerListModel);
  engine.rootContext()->setContextProperty("controller_manager",
                                           &controllerManager);
  engine.rootContext()->setContextProperty("platform_model", &platformListModel);
  engine.rootContext()->setContextProperty("shop_item_model", &shopItemModel);
  engine.rootContext()->setContextProperty("SaveManager", &saveManager);
  engine.rootContext()->setContextProperty("UserLibrary", &userLibrary);
  engine.rootContext()->setContextProperty("LibraryEntryModel", &entryListModel);
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
  engine.loadFromModule("QMLFirelight", "Main2");

  QObject *rootObject = engine.rootObjects().value(0);
  auto window = qobject_cast<QQuickWindow *>(rootObject);
  window->installEventFilter(resizeHandler);
  window->installEventFilter(inputMethodDetectionHandler);

  QObject::connect(window, &QQuickWindow::afterRendering, [&]() {
    if (core != nullptr) {
      core->RunCallbacks();
    }
  });

  if (core) {
  discord::Activity activity{};
    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
      if (result != discord::Result::Ok) {
        spdlog::error("Failed to update activity: {}", (int)result);
      }
    });
  }

  window->setIcon(QIcon("qrc:images/firelight-logo"));

  firelight::SdlEventLoop sdlEventLoop(window, &controllerManager);
  sdlEventLoop.moveToThread(&sdlEventLoop);
  sdlEventLoop.start();

  engine.rootContext()->setContextProperty("sfx_player", new firelight::audio::SfxPlayer());

  int exitCode = QApplication::exec();

  spdlog::info("Exiting QApplication");

  sdlEventLoop.stopProcessing();
  //
  sdlEventLoop.quit();
  sdlEventLoop.wait();

  engine.removeImageProvider("gameImages");
  // TODO: Let daemons finish

  return exitCode;
}
