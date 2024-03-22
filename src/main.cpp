#define SDL_MAIN_HANDLED

#include "app/db/sqlite_content_database.hpp"
#include "app/emulation_manager.hpp"
#include "app/game_loader.hpp"
#include "app/library/library_scanner.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QWindow>
#include <filesystem>
#include <spdlog/spdlog.h>

#include "app/db/sqlite_userdata_database.hpp"
#include "app/fps_multiplier.hpp"
#include "app/input/controller_manager.hpp"
#include "app/input/sdl_event_loop.hpp"
#include "app/library/sqlite_library_database.hpp"
#include "gui/controller_list_model.hpp"
#include "gui/library_item_model.hpp"
#include "gui/library_sort_filter_model.hpp"
#include "gui/playlist_item_model.hpp"
#include "gui/window_resize_handler.hpp"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

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
  if (auto debug = std::getenv("FL_DEBUG"); debug != nullptr) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  // If missing system directory, throw an error
  // TODO

  // **** Make sure all directories are good ****
  std::filesystem::path appdata_dir = ".";
  auto system_dir = appdata_dir / "system";
  auto userdata_dir = appdata_dir / "userdata";
  auto roms_dir = appdata_dir / "roms";
  auto save_dir = userdata_dir / "savedata";

  if (!create_dirs(
          {appdata_dir, system_dir, userdata_dir, roms_dir, save_dir})) {
    return 1;
  }

  QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  QGuiApplication app(argc, argv);

  QGuiApplication::setOrganizationName("Games by Firelight");
  QGuiApplication::setOrganizationDomain("firelight-emulator.com");
  QGuiApplication::setApplicationName("Firelight");

  firelight::Input::ControllerManager controllerManager;
  firelight::SdlEventLoop sdlEventLoop(&controllerManager);

  firelight::ManagerAccessor::setControllerManager(&controllerManager);

  controllerManager.refreshControllerList();

  sdlEventLoop.start();
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  firelight::db::SqliteUserdataDatabase userdata_database(userdata_dir /
                                                          "userdata.db");
  firelight::ManagerAccessor::setUserdataManager(&userdata_database);

  firelight::saves::SaveManager saveManager(save_dir, userdata_database);
  firelight::ManagerAccessor::setSaveManager(&saveManager);

  // **** Load Content Database ****
  SqliteContentDatabase contentDatabase(system_dir / "content.db");

  firelight::db::SqliteLibraryDatabase libraryDatabase(userdata_dir /
                                                       "library.db");
  firelight::ManagerAccessor::setLibraryDatabase(&libraryDatabase);

  // Set up the models for QML ***********************************************
  firelight::gui::ControllerListModel controllerListModel(controllerManager);
  firelight::gui::PlaylistItemModel playlistModel(&libraryDatabase);
  firelight::gui::LibraryItemModel libModel(&libraryDatabase);
  firelight::gui::LibrarySortFilterModel libSortModel;
  libSortModel.setSourceModel(&libModel);

  LibraryScanner libraryManager(&libraryDatabase, roms_dir, &contentDatabase);
  firelight::ManagerAccessor::setLibraryManager(&libraryManager);

  QObject::connect(&libraryManager, &LibraryScanner::scanFinished, &libModel,
                   &firelight::gui::LibraryItemModel::refresh);

  libraryManager.startScan();

  qmlRegisterType<EmulationManager>("Firelight", 1, 0, "EmulatorView");
  qmlRegisterType<FpsMultiplier>("Firelight", 1, 0, "FpsMultiplier");
  qmlRegisterType<firelight::GameLoader>("Firelight", 1, 0, "GameLoader");

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("playlist_model", &playlistModel);
  engine.rootContext()->setContextProperty("library_model", &libModel);
  engine.rootContext()->setContextProperty("library_short_model",
                                           &libSortModel);
  engine.rootContext()->setContextProperty("library_manager", &libraryManager);
  engine.rootContext()->setContextProperty("controller_manager",
                                           &controllerListModel);

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
