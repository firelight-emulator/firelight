

#include "src/app/controller/controller_manager.hpp"
#include "src/app/db/sqlite_content_database.hpp"
#include "src/app/emulation_manager.hpp"
#include "src/app/library/library_manager.hpp"
#include "src/gui/QLibraryManager.hpp"
#include "src/gui/QLibraryViewModel.hpp"
#include <QFileSystemWatcher>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QOpenGLFunctions>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QWindow>
#include <SDL.h>
#include <SDL_error.h>
#include <filesystem>
#include <spdlog/spdlog.h>

#include "app/controller/fps_multiplier.hpp"
#include "app/db/sqlite_library_database.hpp"
#include "gui/controller_manager.hpp"
#include "gui/sdl_event_loop.hpp"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

bool create_dirs(const std::filesystem::path &paths...) {
  std::error_code error_code;

  for (const auto &p : paths) {
    if (!exists(p) && !create_directories(p, error_code)) {
      // spdlog::error("Unable to create directory {}; Error code: {}\n",
      //               p.string(), error_code.message());
      return false;
    }
  }

  return true;
}

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::info);

  // **** Make sure installation directories are all good ****
  const std::filesystem::path installation_dir = ".";

  if (!exists(installation_dir)) {
    spdlog::error("Installation directory {} somehow does not exist. Please "
                  "reinstall the application\n",
                  installation_dir.string());
    return 1;
  }

  // Verify system directory for cores
  const auto system_dir = installation_dir / "system";
  if (!exists(system_dir)) {
    spdlog::error("System directory {} does not exist. Please reinstall the "
                  "application\n",
                  system_dir.string());
    return 1;
  }

  // **** Make sure appdata directories are all good ****
  std::filesystem::path appdata_dir = ".";
  auto userdata_dir = appdata_dir / "userdata";
  auto roms_dir = appdata_dir / "roms";

  if (!create_dirs(appdata_dir, userdata_dir, roms_dir)) {
    return 1;
  }
  //
  // QObject::connect(&libraryManager, &QLibraryManager::scanStarted,
  //                  [] { printf("scan started!\n"); });
  // QObject::connect(&libraryManager, &QLibraryManager::scanFinished, [&] {
  //   printf("setting entries\n");
  //   shortModel.setEntries(library_database.get_all_entries());
  // });
  //

  // libraryManager.refresh();
  // EmulationManager::setLibraryManager(&library_manager);

  // QFileSystemWatcher watcher;
  // watcher.addPath(QString::fromStdString(roms_dir.string()));
  // QObject::connect(&watcher, &QFileSystemWatcher::fileChanged, [](auto s)
  // { printf("file changed: %s\n", s.toStdString().c_str());
  // });
  // QObject::connect(&watcher, &QFileSystemWatcher::directoryChanged,
  // [](const QString &s) {
  // printf("directory changed: %s\n", s.toStdString().c_str());
  // });

  // auto emulator = EmulationManager::getInstance();

  // Load:
  //   SDL2 Event Loop
  //   ControllerManager
  //   SaveManager
  //   LibraryManager
  //   ContentDatabase
  //   EmulationManager
  //

  // QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  QGuiApplication app(argc, argv);
  // qint32 fontId =
  //     QFontDatabase::addApplicationFont("/assets/Lexend-Regular.ttf");
  // if (fontId == -1) {
  //   return 1;
  //

  Firelight::Input::ControllerManager controllerManager;
  Firelight::SdlEventLoop sdlEventLoop(&controllerManager);

  Firelight::ManagerAccessor::setControllerManager(&controllerManager);
  controllerManager.refreshControllerList();

  sdlEventLoop.start();

  // controllerManager.moveToThread(sdlEventThread);

  //
  // QString family = fontList.first();
  // QGuiApplication::setFont(QFont(family));

  QQuickStyle::setStyle("Material");
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  // **** Load Content Database ****
  SqliteContentDatabase content_database(system_dir / "content.db");

  SqliteLibraryDatabase library_database(userdata_dir / "library.db");
  library_database.initialize();
  QLibraryViewModel shortModel;

  QLibraryManager libraryManager(&library_database, roms_dir, &content_database,
                                 &shortModel);
  libraryManager.startScan();

  EmulationManager::setLibraryManager(&libraryManager);

  // auto *model = new QSqlQueryModel;
  // model->setQuery("SELECT id, display_name FROM library");

  qmlRegisterType<EmulationManager>("Firelight", 1, 0, "EmulatorView");
  qmlRegisterType<FpsMultiplier>("Firelight", 1, 0, "FpsMultiplier");

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("library_short_model", &shortModel);
  engine.rootContext()->setContextProperty("library_manager", &libraryManager);

  // engine.rootContext()->setContextProperty("library_short_model", model);

  // engine.rootContext()->setContextProperty("emulator", emulator);

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("QtFirelight", "Main");

  QObject *rootObject = engine.rootObjects().value(0);
  auto window = qobject_cast<QQuickWindow *>(rootObject);
  // emulator->setWindow(window);
  // // window->setColor(Qt::black);
  //
  // QObject::connect(window, &QQuickWindow::beforeRenderPassRecording,
  // emulator,
  //                  &EmulationManager::runOneFrame, Qt::DirectConnection);

  //  window->setFlag(Qt::FramelessWindowHint, true);
  //  window->setFlag(Qt::WindowTitleHint, false);

  return QGuiApplication::exec();
}
