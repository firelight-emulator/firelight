#pragma once

#include "cli/cli_app.hpp"

#include <QLocalServer>
#include <QObject>
#include <QString>

namespace firelight::library {
class UserLibraryService;
}

namespace firelight::platforms {
class IPlatformService;
}

namespace firelight::cli {

// Derives the per-user QLocalServer name from the resolved app-data dir so
// distinct --config-dir / --portable instances don't collide.
QString singleInstanceServerName(const QString &appDataPath);

// If another Firelight is already listening on `serverName`, forwards this
// launch (the ROM path) to it and returns true (the caller should exit).
// Returns false when no running instance was found (this process is primary).
bool forwardLaunchToRunningInstance(const QString &serverName,
                                    const CliOptions &opts);

// The primary instance's listener: turns each forwarded launch from a secondary
// process into a launchRequested(entryId) the root window connects to.
class SingleInstanceServer : public QObject {
  Q_OBJECT

public:
  SingleInstanceServer(QString serverName,
                       library::UserLibraryService &library,
                       platforms::IPlatformService &platformService,
                       QObject *parent = nullptr);

  // Starts listening (clearing any stale socket first). Returns false if the
  // address is already in use (another primary won a startup race).
  bool start();

signals:
  void launchRequested(int entryId);

private:
  void onNewConnection();

  QString m_serverName;
  QLocalServer m_server;
  library::UserLibraryService &m_library;
  platforms::IPlatformService &m_platformService;
};

} // namespace firelight::cli
