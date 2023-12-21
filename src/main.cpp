

#include "rom_model.hpp"
#include "src/app/controller/controller_manager.hpp"
#include "src/app/db/in_memory_game_repository.hpp"
#include "src/app/emulation_manager.hpp"
#include "src/app/library/game_library.hpp"
#include "src/app/libretro/core.hpp"
#include "src/app/metrics/gameloop.hpp"
#include "src/app/saves/save_manager.hpp"
#include "src/gui/emulator_view.hpp"
#include "src/gui/fl_game.hpp"
#include <QGuiApplication>
#include <QOpenGLFunctions>
#include <QOpenGLVersionFunctionsFactory>
#include <QQmlApplicationEngine>
#include <QQuickRenderControl>
#include <QQuickStyle>
#include <QQuickView>
#include <QQuickWindow>
#include <QThread>
#include <QWindow>
#include <SDL.h>
#include <SDL_error.h>
#include <iostream>
#include <sqlite3.h>
#include <thread>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

void doDbThing() {
  // SQLite database connection
  sqlite3 *db;

  // Open a new SQLite database (or create if not exists)
  int rc = sqlite3_open("mydatabase.db", &db);

  if (rc) {
    std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
    return;
  } else {
    std::cout << "Database opened successfully!" << std::endl;
  }

  // SQLite statement
  const char *createTableSQL = "CREATE TABLE IF NOT EXISTS MyTable ("
                               "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                               "Name TEXT NOT NULL,"
                               "Age INTEGER);";

  // Execute the SQL statement
  rc = sqlite3_exec(db, createTableSQL, 0, 0, 0);

  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    return;
  } else {
    std::cout << "Table created successfully!" << std::endl;
  }

  // Insert data into the table
  const char *insertSQL =
      "INSERT INTO MyTable (Name, Age) VALUES ('John Doe', 30);";

  rc = sqlite3_exec(db, insertSQL, nullptr, nullptr, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    return;
  } else {
    std::cout << "Data inserted successfully!" << std::endl;
  }

  // Query data from the table
  const char *selectSQL = "SELECT * FROM MyTable;";

  sqlite3_stmt *stmt;
  rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    return;
  }

  // Iterate through the result set
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int id = sqlite3_column_int(stmt, 0);
    const unsigned char *name = sqlite3_column_text(stmt, 1);
    int age = sqlite3_column_int(stmt, 2);

    std::cout << "ID: " << id << ", Name: " << name << ", Age: " << age
              << std::endl;
  }

  // Finalize and close the statement
  sqlite3_finalize(stmt);

  // Close the database
  sqlite3_close(db);
}

int main(int argc, char *argv[]) {
  SDL_SetHint(SDL_HINT_APP_NAME, "Firelight");
  SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");

  if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO | SDL_INIT_EVENTS |
               SDL_INIT_HAPTIC | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  FL::DB::InMemoryGameRepository repository("games.json", "romhacks.json");

  FL::SaveManager saveManager;

  FL::Library::GameLibrary library("./userdata/library.json", &repository);
  library.scanNow();

  QGuiApplication app(argc, argv);
  QQuickStyle::setStyle("Material");
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  qmlRegisterType<RomModel>("Firelight", 1, 0, "RomModel");
  qmlRegisterType<FLGame>("Firelight", 1, 0, "FLGame");
  qmlRegisterType<EmulatorView>("Firelight", 1, 0, "EmulatorView");

  QQmlApplicationEngine engine;
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("QtFirelight", "Main");

  QObject *rootObject = engine.rootObjects().value(0);
  auto window = qobject_cast<QQuickWindow *>(rootObject);
  //  QObject::connect(window, &QQuickWindow::beforeRendering,
  //                   EmulationManager::getInstance(),
  //                   &EmulationManager::runOneFrame, Qt::DirectConnection);

  //  core->getVideo()->initialize(0, 0, 1280, 720);

  //  QObject::connect(window, &QQuickWindow::beforeRendering, [&]() {
  //    window->update();
  //    frameBegin = SDL_GetPerformanceCounter();
  //    LAST = NOW;
  //    NOW = SDL_GetPerformanceCounter();
  //
  //    deltaTime = ((NOW - LAST) * 1000 /
  //    (double)SDL_GetPerformanceFrequency());
  //    //    printf("delta time: %f\n", deltaTime);
  //    core->run(deltaTime);
  //
  //    frameEnd = SDL_GetPerformanceCounter();
  //    frameDiff = ((frameEnd - frameBegin) * 1000 /
  //                 (double)SDL_GetPerformanceFrequency());
  //    loopMetrics.recordFrameWorkDuration(frameDiff);
  //  });
  //
  //  QObject::connect(window, &QQuickWindow::frameSwapped, [&]() {
  //    auto newFrameEnd = SDL_GetPerformanceCounter();
  //    auto thing = ((newFrameEnd - frameEnd) * 1000 /
  //                  (double)SDL_GetPerformanceFrequency());
  //
  //    //    printf("time since stuff was drawn: %f\n", thing);
  //  });

  return QGuiApplication::exec();
}
