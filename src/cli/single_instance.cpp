#include "cli/single_instance.hpp"

#include "cli/rom_launch.hpp"

#include <QCryptographicHash>
#include <QLocalSocket>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::cli {

namespace {
// Timeouts for the tiny local handshake; a live instance answers in <1ms
constexpr int CONNECT_TIMEOUT_MS = 300;
constexpr int IO_TIMEOUT_MS = 300;
} // namespace

QString singleInstanceServerName(const QString &appDataPath) {
  const auto digest = QCryptographicHash::hash(appDataPath.toUtf8(),
                                               QCryptographicHash::Sha1)
                          .toHex();
  return "firelight-" + QString::fromLatin1(digest.left(16));
}

bool forwardLaunchToRunningInstance(const QString &serverName,
                                    const CliOptions &opts) {
  QLocalSocket socket;
  socket.connectToServer(serverName);
  if (!socket.waitForConnected(CONNECT_TIMEOUT_MS)) {
    return false; // no primary is listening -> we are it
  }
  // v1 payload: the ROM path (empty just pings the running instance)
  QByteArray payload = QByteArray::fromStdString(opts.romPath);
  payload.append('\n');
  socket.write(payload);
  socket.flush();
  socket.waitForBytesWritten(IO_TIMEOUT_MS);
  socket.disconnectFromServer();
  spdlog::info("Forwarded launch to the running Firelight instance");
  return true;
}

SingleInstanceServer::SingleInstanceServer(
    QString serverName, library::UserLibraryService &library,
    platforms::IPlatformService &platformService, QObject *parent)
    : QObject(parent), m_serverName(std::move(serverName)), m_library(library),
      m_platformService(platformService) {
  connect(&m_server, &QLocalServer::newConnection, this,
          &SingleInstanceServer::onNewConnection);
}

bool SingleInstanceServer::start() {
  // A prior crash can leave a stale socket file; clear it before listening
  QLocalServer::removeServer(m_serverName);
  if (!m_server.listen(m_serverName)) {
    spdlog::warn("Single-instance: could not listen on {} ({})",
                 m_serverName.toStdString(),
                 m_server.errorString().toStdString());
    return false;
  }
  return true;
}

void SingleInstanceServer::onNewConnection() {
  auto *socket = m_server.nextPendingConnection();
  if (!socket) {
    return;
  }
  connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);

  socket->waitForReadyRead(IO_TIMEOUT_MS);
  const auto romPath = QString::fromUtf8(socket->readAll()).trimmed();
  socket->disconnectFromServer();

  if (romPath.isEmpty()) {
    return; // a bare ping with no ROM to launch
  }
  const int entryId =
      resolveRomEntryId(romPath.toStdString(), m_library, m_platformService);
  if (entryId >= 0) {
    emit launchRequested(entryId);
  } else {
    spdlog::warn("Single-instance: forwarded ROM '{}' did not resolve to a "
                 "library entry",
                 romPath.toStdString());
  }
}

} // namespace firelight::cli
