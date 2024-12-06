#define SDL_MAIN_HANDLED

// #include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QWindow>
#include <filesystem>
#include <qstandardpaths.h>
#include <qstringlistmodel.h>
#include <csignal>
#include <spdlog/spdlog.h>
#include <cstdlib>

#include "app/rcheevos/ra_client.hpp"
#include "app/db/sqlite_content_database.hpp"
#include "app/db/sqlite_userdata_database.hpp"
#include "app/emulation_manager.hpp"
#include "app/router.hpp"
#include "app/audio/SfxPlayer.hpp"
#include "app/input/controller_manager.hpp"
#include "app/input/sdl_event_loop.hpp"
#include "app/library/library_scanner.hpp"
#include "app/library/library_scanner2.hpp"
#include "app/library/sqlite_library_database.hpp"
#include "app/saves/save_manager.hpp"
#include "gui/controller_list_model.hpp"
#include "gui/gamepad_profile.hpp"
#include "gui/game_image_provider.hpp"
#include "gui/eventhandlers/input_method_detection_handler.hpp"
#include "gui/library_item_model.hpp"
#include "gui/library_sort_filter_model.hpp"
#include "gui/mod_item_model.hpp"
#include "gui/platform_list_model.hpp"
#include "gui/playlist_item_model.hpp"
#include "gui/eventhandlers/window_resize_handler.hpp"
#include "gui/models/shop/shop_item_model.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <QtConcurrent>

#include "app/emulator_item.hpp"
#include "app/gamepad_status_item.hpp"
#include "app/game_loader.hpp"
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

  // struct archive *a;
  // struct archive_entry *entry;
  // int r;
  //
  // a = archive_read_new();
  // archive_read_support_filter_all(a);
  // archive_read_support_format_all(a);
  // r = archive_read_open_filename(a, "Nintendo - Game Boy Advance.zip", 10240); // Note 1
  // if (r != ARCHIVE_OK)
  //   exit(1);
  // while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
  //   printf("%s\n", archive_entry_pathname(entry));
  //   archive_read_data_skip(a); // Note 2
  // }
  // r = archive_read_free(a); // Note 3
  // if (r != ARCHIVE_OK)
  //   exit(1);
  //
  // return 0;


  if (auto debug = std::getenv("FL_DEBUG"); debug != nullptr) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  QApplication::setOrganizationDomain("firelight-emulator.com");
  QApplication::setApplicationName("Firelight");

  QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  QSurfaceFormat format;
  format.setProfile(QSurfaceFormat::OpenGLContextProfile::CompatibilityProfile);
  format.setVersion(4, 1);
  QSurfaceFormat::setDefaultFormat(format);

  QApplication app(argc, argv);

  std::signal(SIGINT, [](int signal) {
    QApplication::quit();
  });

  auto defaultUserPathString = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  auto defaultUserPath = std::filesystem::path(defaultUserPathString.toStdString()) / "Firelight";

  auto defaultAppDataPathString = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  auto defaultAppDataPath = std::filesystem::path(defaultAppDataPathString.toStdString());

  // TODO:
  //  Roms

  // QSettings:
  // credentials?

  // images
  // auto cachePath =
  //     QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  // "C:/Users/<USER>/AppData/Local/Firelight/cache/"

  // saves files
  // userdata db
  // controller profiles
  // library db

  // auto docPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  //
  // printf("Documents Path: %s\n", docPath.toStdString().c_str());

  // auto appDataPath =
  //     QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  // C:/Users/<USER>/AppData/Roaming/Firelight/library.db
  // C:/Users/<USER>/AppData/Roaming/Firelight/patches/
  // C:/Users/<USER>/AppData/Roaming/Firelight/saves/<content_id>/slot<n>/
  // C:/Users/<USER>/AppData/Roaming/Firelight/userdata.db

  auto saveDir = defaultUserPath / "saves";
  auto romsDir = defaultUserPath / "roms";
  // auto patchesDir = appDataDir / "patches";

  // If missing system directory, throw an error
  // TODO

  // **** Make sure all directories are good ****

  if (!create_dirs({defaultUserPath, defaultAppDataPath, romsDir, saveDir})) {
    return 1;
  }

  firelight::Input::ControllerManager controllerManager;

  firelight::ManagerAccessor::setControllerManager(&controllerManager);

  controllerManager.refreshControllerList();
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  firelight::db::SqliteUserdataDatabase userdata_database(defaultAppDataPath /
                                                          "userdata.db");
  firelight::ManagerAccessor::setUserdataManager(&userdata_database);
  firelight::ManagerAccessor::setActivityLog(&userdata_database);

  auto gameImageProvider = new firelight::gui::GameImageProvider();
  firelight::ManagerAccessor::setGameImageProvider(gameImageProvider);

  // **** Load Content Database ****
  firelight::db::SqliteContentDatabase contentDatabase(defaultAppDataPath /
                                                       "content.db");

  firelight::db::SqliteLibraryDatabase libraryDatabase(defaultAppDataPath /
                                                       "library.db");
  firelight::ManagerAccessor::setLibraryDatabase(&libraryDatabase);

  firelight::saves::SaveManager saveManager(saveDir, libraryDatabase, userdata_database, *gameImageProvider);
  firelight::ManagerAccessor::setSaveManager(&saveManager);

  firelight::library::SqliteUserLibrary
      userLibrary(QString::fromStdString(defaultAppDataPath.string() + "/library.db"));

  if (userLibrary.getWatchedDirectories().empty()) {
    firelight::library::WatchedDirectory dir{
      .path = QString::fromStdString(canonical(romsDir).string())
    };

    userLibrary.addWatchedDirectory(dir);
  }

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
  // libScanner2.scanAll();

  firelight::achievements::RAClient raClient(contentDatabase);
  firelight::ManagerAccessor::setAchievementManager(&raClient);

  // Set up the models for QML ***********************************************
  firelight::gui::ModItemModel modListModel(contentDatabase);
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

  qmlRegisterType<EmulationManager>("Firelight", 1, 0, "EmulatorView");
  qmlRegisterType<EmulatorItem>("Firelight", 1, 0, "EmulatorItem");
  qmlRegisterType<firelight::input::GamepadStatusItem>("Firelight", 1, 0, "GamepadStatus");
  qmlRegisterType<firelight::gui::GamepadMapping>("Firelight", 1, 0, "GamepadMapping");
  qmlRegisterType<firelight::gui::GamepadProfile>("Firelight", 1, 0, "GamepadProfile");

  firelight::gui::Router router;

  QQmlApplicationEngine engine;
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
  engine.rootContext()->setContextProperty("mod_model", &modListModel);
  engine.rootContext()->setContextProperty("platform_model", &platformListModel);
  engine.rootContext()->setContextProperty("shop_item_model", &shopItemModel);
  engine.rootContext()->setContextProperty("SaveManager", &saveManager);
  engine.rootContext()->setContextProperty("LibraryEntryModel", &entryListModel);
  engine.rootContext()->setContextProperty("LibraryScanner", &libScanner2);
  engine.rootContext()->setContextProperty("GameLoader", new firelight::GameLoader());

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
  // QObject::connect(
  //   &engine, &QQmlApplicationEngine::objectCreated, &app,
  //   [](QObject *item, const QUrl &url) { printf("Created: %s\n", url.toDisplayString().toStdString().c_str()); },
  //   Qt::QueuedConnection);
  engine.loadFromModule("QMLFirelight", "Main2");
  // engine.loadFromModule("QMLFirelight", "ControllerTest");

  QObject *rootObject = engine.rootObjects().value(0);
  auto window = qobject_cast<QQuickWindow *>(rootObject);
  window->installEventFilter(resizeHandler);
  window->installEventFilter(inputMethodDetectionHandler);


  // QObject::connect(
  //   window, &QQuickWindow::frameSwapped, window,
  //   []() {
  //     static qint64 last = 0;
  //     auto now = QDateTime::currentMSecsSinceEpoch();
  //     auto elapsed = now - last;
  //     if (elapsed >= 20) {
  //       spdlog::info("Time since last frame: {}ms", elapsed);
  //     }
  //     last = now;
  //   }, Qt::QueuedConnection);


  firelight::SdlEventLoop sdlEventLoop(window, &controllerManager);
  sdlEventLoop.moveToThread(&sdlEventLoop);
  sdlEventLoop.start();

  // QObject::connect(window, &QQuickWindow::afterAnimating, &sdlEventLoop, &firelight::SdlEventLoop::pollEvents);
  // connect(sdlThread, &QThread::started, &sdlEventLoop, &firelight::SdlEventLoop::run);
  // sdlThread->start();
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
