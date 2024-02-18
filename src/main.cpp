

#include "src/app/db/sqlite_content_database.hpp"
#include "src/app/emulation_manager.hpp"
#include "src/app/game_loader.hpp"
#include "src/gui/QLibraryManager.hpp"
#include "src/gui/QLibraryViewModel.hpp"
#include <QGuiApplication>
#include <QOpenGLFunctions>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QWindow>
#include <SDL.h>
#include <filesystem>
#include <spdlog/spdlog.h>

#include "app/db/sqlite_library_database.hpp"
#include "app/db/sqlite_userdata_database.hpp"
#include "app/fps_multiplier.hpp"
#include "app/input/controller_manager.hpp"
#include "app/input/sdl_event_loop.hpp"
#include "gui/window_resize_handler.hpp"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

bool create_dirs(const std::initializer_list<std::filesystem::path> list)
{
  std::error_code error_code;
  for (const auto& path : list) {
    if (!exists(path))
    {
      spdlog::info("Directory not found, creating: {}", path.string());
      if (!create_directories(path, error_code)) {
        spdlog::error("Unable to create directory {}; Error code: {}", path.string(), error_code.message());
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

  // **** Make sure all directories are good ****
  std::filesystem::path appdata_dir = ".";
  auto system_dir = appdata_dir / "system";
  auto userdata_dir = appdata_dir / "userdata";
  auto roms_dir = appdata_dir / "roms";
  auto save_dir = userdata_dir / "savedata";

  if (!create_dirs({ appdata_dir, system_dir, userdata_dir, roms_dir, save_dir } )) {
    return 1;
  }



  // QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  QGuiApplication app(argc, argv);

  Firelight::Input::ControllerManager controllerManager;
  Firelight::SdlEventLoop sdlEventLoop(&controllerManager);

  Firelight::Saves::SaveManager saveManager(save_dir);

  Firelight::ManagerAccessor::setControllerManager(&controllerManager);
  Firelight::ManagerAccessor::setSaveManager(&saveManager);

  controllerManager.refreshControllerList();

  sdlEventLoop.start();

  // controllerManager.moveToThread(sdlEventThread);

  //
  // QString family = fontList.first();
  // QGuiApplication::setFont(QFont(family));

  // QQuickStyle::setStyle("FirelightStyle");
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  SqliteUserdataDatabase userdata_database(userdata_dir / "userdata.db");
  Firelight::ManagerAccessor::setUserdataManager(&userdata_database);

  // **** Load Content Database ****
  SqliteContentDatabase content_database(system_dir / "content.db");

  SqliteLibraryDatabase library_database(userdata_dir / "library.db");
  library_database.initialize();
  QLibraryViewModel shortModel;

  QLibraryManager libraryManager(&library_database, roms_dir, &content_database,
                                 &shortModel);
  libraryManager.startScan();
  Firelight::ManagerAccessor::setLibraryManager(&libraryManager);

  // auto *model = new QSqlQueryModel;
  // model->setQuery("SELECT id, display_name FROM library");

  qmlRegisterType<EmulationManager>("Firelight", 1, 0, "EmulatorView");
  qmlRegisterType<FpsMultiplier>("Firelight", 1, 0, "FpsMultiplier");
  qmlRegisterType<Firelight::GameLoader>("Firelight", 1, 0, "GameLoader");

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("library_short_model", &shortModel);
  engine.rootContext()->setContextProperty("library_manager", &libraryManager);
  engine.rootContext()->setContextProperty("controller_manager",
                                           &controllerManager);

  auto resizeHandler = new Firelight::WindowResizeHandler();
  engine.rootContext()->setContextProperty("window_resize_handler",
                                           resizeHandler);

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("QMLFirelight", "Main");

  QObject *rootObject = engine.rootObjects().value(0);
  auto window = qobject_cast<QQuickWindow *>(rootObject);
  window->installEventFilter(resizeHandler);

  // emulator->setWindow(window);
  // // window->setColor(Qt::black);
  //
  // QObject::connect(window, &QQuickWindow::beforeRenderPassRecording,
  // emulator,
  //                  &EmulationManager::runOneFrame, Qt::DirectConnection);

  //  window->setFlag(Qt::FramelessWindowHint, true);
  //  window->setFlag(Qt::WindowTitleHint, false);

  int exitCode = QGuiApplication::exec();

  sdlEventLoop.stopProcessing();

  sdlEventLoop.quit();
  sdlEventLoop.wait();

  return exitCode;
}
