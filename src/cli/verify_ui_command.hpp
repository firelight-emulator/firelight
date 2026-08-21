#pragma once

#include "cli/cli_app.hpp"

#include <QString>
#include <vector>

class QQmlApplicationEngine;
class QObject;

namespace firelight::cli {

// TODO
/** Mounts every registered route in turn and reports the ones that fail */
class VerifyUiRunner {
public:
  // TODO
  /** The outcome of mounting one route */
  struct RouteResult {
    QString path;
    bool mounted = false;
    bool synthetic = false; // a made-up path param was used
    bool skipped = false;   // not checkable without a running game
    QString failure;
    int fatalMessages = 0;
  };

  // TODO
  /**
   * Starts the sweep and returns immediately; the caller keeps running the
   * event loop, and the app exits with the result code once every route is
   * visited
   */
  static void start(QQmlApplicationEngine &engine, QObject *rootObject, const CliOptions &options);

  // TODO
  /** The routes visited when none are named on the command line */
  static std::vector<QString> getDefaultRoutes();
};

} // namespace firelight::cli
