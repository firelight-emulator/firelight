#pragma once

#include "cli/cli_app.hpp"

#include <QString>
#include <vector>

class QQmlApplicationEngine;
class QObject;

namespace firelight::cli {

/**
 * Mounts every registered route in turn and reports the ones that fail, so a UI
 * regression shows up without a human clicking through the app.
 */
class VerifyUiRunner {
public:
  /** The outcome of mounting one route. */
  struct RouteResult {
    QString path;
    bool mounted = false;
    bool synthetic = false; // a made-up path param was used
    bool skipped = false;   // not checkable without a running game
    QString failure;
    int fatalMessages = 0;
  };

  /**
   * Starts the sweep. Returns immediately; the caller keeps running the event
   * loop and the app exits with the result code once every route is visited.
   */
  static void start(QQmlApplicationEngine &engine, QObject *rootObject, const CliOptions &options);

  /** The routes visited when none are named on the command line. */
  static std::vector<QString> getDefaultRoutes();
};

} // namespace firelight::cli
