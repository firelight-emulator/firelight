

#include "src/app/controller/controller_manager.hpp"
#include "src/app/db/sqlite_content_database.hpp"
#include "src/app/emulation_manager.hpp"
#include "src/app/library/library_manager.hpp"
#include "src/gui/QLibEntryModelShort.hpp"
#include "src/gui/QLibraryManager.hpp"
#include <QGuiApplication>
#include <QOpenGLFunctions>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QWindow>
#include <SDL.h>
#include <SDL_error.h>

#include "app/controller/fps_multiplier.hpp"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

int main(int argc, char *argv[]) {
  SDL_SetHint(SDL_HINT_APP_NAME, "Firelight");
  SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");

  if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO | SDL_INIT_EVENTS |
               SDL_INIT_HAPTIC | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  // **** Load Content Database ****
  SqliteContentDatabase content_database("content.db");

  // **** Load Library Manager ****
  FL::Library::LibraryManager library_manager("library.db", "./roms",
                                              &content_database);
  library_manager.scanNow();

  QLibEntryModelShort shortModel;
  QLibraryManager libraryManager(&library_manager, &shortModel);

  libraryManager.refresh();
  EmulationManager::setLibraryManager(&library_manager);

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
