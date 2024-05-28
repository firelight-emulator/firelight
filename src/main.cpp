#define SDL_MAIN_HANDLED

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QWindow>
#include <filesystem>
#include <qstandardpaths.h>
#include <qstringlistmodel.h>

#include <spdlog/spdlog.h>

#include "app/achieve/ra_client.hpp"
#include "app/db/sqlite_content_database.hpp"
#include "app/db/sqlite_userdata_database.hpp"
#include "app/emulation_manager.hpp"
#include "app/fps_multiplier.hpp"
#include "app/input/controller_manager.hpp"
#include "app/input/sdl_event_loop.hpp"
#include "app/library/library_scanner.hpp"
#include "app/library/sqlite_library_database.hpp"
#include "app/saves/save_manager.hpp"
#include "gui/controller_list_model.hpp"
#include "gui/library_item_model.hpp"
#include "gui/library_sort_filter_model.hpp"
#include "gui/mod_item_model.hpp"
#include "gui/playlist_item_model.hpp"
#include "gui/window_resize_handler.hpp"

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
  if (auto debug = std::getenv("FL_DEBUG"); debug != nullptr) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  QGuiApplication::setOrganizationDomain("firelight-emulator.com");
  QGuiApplication::setApplicationName("Firelight");

  QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  QGuiApplication app(argc, argv);


  //TODO:
  // Roms

  // QSettings:
  // credentials?

  // images
  auto cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  // "C:/Users/<USER>/AppData/Local/Firelight/cache/"

  // saves files
  // userdata db
  // controller profiles
  // library db
  auto appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);


  // C:/Users/<USER>/AppData/Roaming/Firelight/library.db
  // C:/Users/<USER>/AppData/Roaming/Firelight/patches/
  // C:/Users/<USER>/AppData/Roaming/Firelight/saves/<content_id>/slot<n>/
  // C:/Users/<USER>/AppData/Roaming/Firelight/userdata.db

  printf("Cache path: %s\n", cachePath.toStdString().c_str());
  printf("App data path: %s\n", appDataPath.toStdString().c_str());

  auto appDataDir = std::filesystem::path(appDataPath.toStdString());
  auto saveDir = appDataDir / "saves";
  auto romsDir = appDataDir / "roms";
  auto patchesDir = appDataDir / "patches";

  // If missing system directory, throw an error
  // TODO

  // **** Make sure all directories are good ****
  std::filesystem::path appdata_dir = ".";
  auto systemDir = appdata_dir / "system";

  if (!create_dirs({
    appdata_dir, systemDir, romsDir,
    patchesDir, saveDir
  })) {
    return 1;
  }

  firelight::Input::ControllerManager controllerManager;
  firelight::SdlEventLoop sdlEventLoop(&controllerManager);

  firelight::ManagerAccessor::setControllerManager(&controllerManager);

  controllerManager.refreshControllerList();

  sdlEventLoop.start();
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  firelight::db::SqliteUserdataDatabase userdata_database(appDataDir /
                                                          "userdata.db");
  firelight::ManagerAccessor::setUserdataManager(&userdata_database);

  firelight::saves::SaveManager saveManager(saveDir, userdata_database);
  firelight::ManagerAccessor::setSaveManager(&saveManager);

  // **** Load Content Database ****
  firelight::db::SqliteContentDatabase contentDatabase(systemDir /
                                                       "content.db");

  firelight::db::SqliteLibraryDatabase libraryDatabase(appDataDir /
                                                       "library.db");
  firelight::ManagerAccessor::setLibraryDatabase(&libraryDatabase);

  auto contentDirs = libraryDatabase.getAllLibraryContentDirectories();
  if (contentDirs.empty()) {
    firelight::db::LibraryContentDirectory dir{
      .path = canonical(romsDir).string()
    };

    libraryDatabase.createLibraryContentDirectory(dir);
  }

  firelight::achievements::RAClient raClient;
  firelight::ManagerAccessor::setAchievementManager(&raClient);

  // Set up the models for QML ***********************************************
  firelight::gui::ModItemModel modListModel(contentDatabase);
  firelight::gui::ControllerListModel controllerListModel(controllerManager);
  firelight::gui::PlaylistItemModel playlistModel(&libraryDatabase);
  firelight::gui::LibraryItemModel libModel(&libraryDatabase, &contentDatabase,
                                            &userdata_database);
  firelight::gui::LibrarySortFilterModel libSortModel;
  libSortModel.setSourceModel(&libModel);

  LibraryScanner libraryManager(&libraryDatabase, &contentDatabase);
  firelight::ManagerAccessor::setLibraryManager(&libraryManager);

  QObject::connect(&libraryDatabase,
                   &firelight::db::SqliteLibraryDatabase::contentDirectoriesUpdated,
                   &libraryManager,
                   &LibraryScanner::startScan);

  QObject::connect(&libraryManager, &LibraryScanner::scanFinished, &libModel,
                   &firelight::gui::LibraryItemModel::refresh);

  libraryManager.startScan();

  qmlRegisterType<EmulationManager>("Firelight", 1, 0, "EmulatorView");
  qmlRegisterType<FpsMultiplier>("Firelight", 1, 0, "FpsMultiplier");

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("achievement_manager", &raClient);
  engine.rootContext()->setContextProperty("playlist_model", &playlistModel);
  engine.rootContext()->setContextProperty("library_model", &libModel);
  engine.rootContext()->setContextProperty("library_short_model",
                                           &libSortModel);
  engine.rootContext()->setContextProperty("library_manager", &libraryManager);
  engine.rootContext()->setContextProperty("library_scan_path_model",
                                           libraryManager.scanDirectoryModel());
  engine.rootContext()->setContextProperty("controller_manager",
                                           &controllerListModel);
  engine.rootContext()->setContextProperty("mod_model", &modListModel);

  auto resizeHandler = new firelight::gui::WindowResizeHandler();
  engine.rootContext()->setContextProperty("window_resize_handler",
                                           resizeHandler);

  QObject::connect(
    &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
    []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("QMLFirelight", "Main");

  QObject *rootObject = engine.rootObjects().value(0);
  auto window = qobject_cast<QQuickWindow *>(rootObject);
  window->installEventFilter(resizeHandler);

  int exitCode = QGuiApplication::exec();

  sdlEventLoop.stopProcessing();

  sdlEventLoop.quit();
  sdlEventLoop.wait();

  // TODO: Let daemons finish

  return exitCode;
}
