

#include "spdlog/spdlog.h"
#include "src/app/controller/controller_manager.hpp"
#include "src/app/db/sqlite_content_database.hpp"
#include "src/app/emulation_manager.hpp"
#include "src/app/library/library_manager.hpp"
#include "src/gui/QLibEntryModelShort.hpp"
#include "src/gui/QLibraryManager.hpp"
#include <QFileSystemWatcher>
#include <QGuiApplication>
#include <QOpenGLFunctions>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QWindow>
#include <SDL.h>
#include <SDL_error.h>
#include <filesystem>

#include "app/controller/fps_multiplier.hpp"
#include "app/db/sqlite_library_database.hpp"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

bool create_dirs(const std::filesystem::path &paths...) {
  std::error_code error_code;

  for (const auto &p : paths) {
    if (!exists(p) && !create_directories(p, error_code)) {
      spdlog::error("Unable to create directory {}; Error code: {}\n",
                    p.string(), error_code.message());
      return false;
    }
  }

  return true;
}

int main(int argc, char *argv[]) {
  SDL_SetHint(SDL_HINT_APP_NAME, "Firelight");
  SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");

  if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO | SDL_INIT_HAPTIC |
               SDL_INIT_TIMER) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

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

  // **** Load Content Database ****
  SqliteContentDatabase content_database(system_dir / "content.db");

  // **** Load Library Manager ****
  // FL::Library::LibraryManager library_manager(userdata_dir / "library.db",
  //                                             roms_dir, &content_database);
  // library_manager.scanNow();

  SqliteLibraryDatabase library_database(userdata_dir / "library.db");

  QLibEntryModelShort shortModel;
  QLibraryManager libraryManager(&library_database, roms_dir, &content_database,
                                 &shortModel);

  QObject::connect(&libraryManager, &QLibraryManager::scanStarted,
                   [] { printf("scan started!\n"); });
  QObject::connect(&libraryManager, &QLibraryManager::scanFinished,
                   [] { printf("scan finished!\n"); });

  libraryManager.startScan();

  // libraryManager.refresh();
  // EmulationManager::setLibraryManager(&library_manager);

  // QFileSystemWatcher watcher;
  // watcher.addPath(QString::fromStdString(roms_dir.string()));
  // QObject::connect(&watcher, &QFileSystemWatcher::fileChanged, [](auto s) {
  // printf("file changed: %s\n", s.toStdString().c_str());
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
  QQuickStyle::setStyle("Material");
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  qmlRegisterType<EmulationManager>("Firelight", 1, 0, "EmulatorView");
  qmlRegisterType<FpsMultiplier>("Firelight", 1, 0, "FpsMultiplier");

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("libraryEntryModelShort",
                                           &shortModel);

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
