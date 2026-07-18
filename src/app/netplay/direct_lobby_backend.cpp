#include "direct_lobby_backend.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QSettings>
#include <QTimer>
#include <spdlog/spdlog.h>

namespace firelight::netplay {

namespace {
QByteArray encodeLine(const QJsonObject &message) {
  return QJsonDocument(message).toJson(QJsonDocument::Compact) + '\n';
}

// Pulls complete newline-delimited JSON objects out of a stream buffer
template <typename Fn> void drainLines(QByteArray &buffer, Fn &&handle) {
  int newline;
  while ((newline = buffer.indexOf('\n')) >= 0) {
    const auto line = buffer.left(newline);
    buffer.remove(0, newline + 1);
    const auto parsed = QJsonDocument::fromJson(line);
    if (parsed.isObject()) {
      handle(parsed.object());
    }
  }
}

const QString PLAYER_NAME_KEY = "netplay/playerName";

std::string localPlayerName() {
  const auto saved = QSettings().value(PLAYER_NAME_KEY).toString().trimmed();
  if (!saved.isEmpty()) {
    return saved.toStdString();
  }
  const auto user = qEnvironmentVariable("USERNAME");
  return user.isEmpty() ? "Player" : user.toStdString();
}
} // namespace

DirectLobbyBackend::DirectLobbyBackend(QObject *parent)
    : QObject(parent), m_displayName(localPlayerName()) {}

void DirectLobbyBackend::setPreferredDisplayName(const std::string &name) {
  const auto trimmed = QString::fromStdString(name).trimmed();
  if (trimmed.isEmpty()) {
    return;
  }
  QSettings().setValue(PLAYER_NAME_KEY, trimmed);

  const auto newName = trimmed.toStdString();
  bool joined = false;
  PlayerId selfId = 0;
  {
    std::lock_guard lock(m_stateMutex);
    if (m_displayName == newName) {
      return;
    }
    m_displayName = newName;
    joined = m_joined;
    selfId = m_selfId;
    for (auto &member : m_members) {
      if (member.id == selfId) {
        member.displayName = newName;
      }
    }
  }
  if (!joined) {
    return;
  }

  // Renames apply live: tell the lobby, and report the local change too
  if (m_server) {
    broadcastToGuests({{"t", "member-renamed"},
                       {"id", QString::number(selfId)},
                       {"name", trimmed}},
                      nullptr);
  } else if (m_hostSocket) {
    m_hostSocket->write(encodeLine({{"t", "rename"}, {"name", trimmed}}));
  }
  if (m_events.memberRenamed) {
    m_events.memberRenamed(selfId, newName);
  }
}

void DirectLobbyBackend::beginSignIn(std::function<void(bool)> done) {
  if (done) {
    done(true);
  }
}

SignInState DirectLobbyBackend::signInState() const {
  return SignInState::Ready;
}

PlayerIdentity DirectLobbyBackend::localIdentity() const {
  std::lock_guard lock(m_stateMutex);
  return PlayerIdentity{m_selfId, m_displayName};
}

// --- host ---

void DirectLobbyBackend::createLobby(const std::string &, ResultCallback done) {
  if (m_server || m_hostSocket) {
    if (done) {
      done(false, "Already in a lobby");
    }
    return;
  }

  m_server = new QTcpServer(this);
  if (!m_server->listen(QHostAddress::Any, DEFAULT_PORT)) {
    const auto error = m_server->errorString().toStdString();
    m_server->deleteLater();
    m_server = nullptr;
    if (done) {
      done(false, "Couldn't open port " + std::to_string(DEFAULT_PORT) + ": " +
                      error);
    }
    return;
  }

  connect(m_server, &QTcpServer::newConnection, this, [this] {
    while (auto *socket = m_server->nextPendingConnection()) {
      m_guests[socket] = GuestConnection{};
      connect(socket, &QTcpSocket::readyRead, this, [this, socket] {
        auto it = m_guests.find(socket);
        if (it == m_guests.end()) {
          return;
        }
        it->second.buffer += socket->readAll();
        drainLines(it->second.buffer, [this, socket](const QJsonObject &m) {
          handleGuestLine(socket, m);
        });
      });
      connect(socket, &QTcpSocket::disconnected, this,
              [this, socket] { guestSocketClosed(socket); });
    }
  });

  {
    std::lock_guard lock(m_stateMutex);
    m_joined = true;
    m_isHost = true;
    m_selfId = 1;
    m_hostId = 1;
    m_joinCodeDisplay = listenAddressDisplay();
    m_members = {LobbyMember{1, m_displayName}};
  }
  if (done) {
    done(true, "");
  }
}

void DirectLobbyBackend::handleGuestLine(QTcpSocket *socket,
                                         const QJsonObject &message) {
  auto it = m_guests.find(socket);
  if (it == m_guests.end()) {
    return;
  }
  auto &guest = it->second;
  const auto type = message.value("t").toString();

  if (type == "hello" && !guest.helloReceived) {
    guest.helloReceived = true;
    guest.id = m_nextGuestId++;
    guest.name = message.value("name").toString("Player").toStdString();

    QJsonArray members;
    {
      std::lock_guard lock(m_stateMutex);
      m_members.push_back(LobbyMember{guest.id, guest.name});
      for (const auto &member : m_members) {
        members.append(QJsonObject{
            {"id", QString::number(member.id)},
            {"name", QString::fromStdString(member.displayName)}});
      }
    }
    socket->write(encodeLine({{"t", "welcome"},
                              {"id", QString::number(guest.id)},
                              {"hostId", QString::number(m_hostId)},
                              {"members", members}}));
    broadcastToGuests({{"t", "member-joined"},
                       {"id", QString::number(guest.id)},
                       {"name", QString::fromStdString(guest.name)}},
                      socket);
    if (m_events.memberJoined) {
      m_events.memberJoined(LobbyMember{guest.id, guest.name});
    }
    return;
  }
  if (!guest.helloReceived) {
    return;
  }

  if (type == "chat") {
    const auto text = message.value("text").toString().toStdString();
    broadcastToGuests({{"t", "chat"},
                       {"from", QString::number(guest.id)},
                       {"name", QString::fromStdString(guest.name)},
                       {"text", message.value("text")}},
                      socket);
    if (m_events.chatReceived) {
      m_events.chatReceived(guest.id, guest.name, text);
    }
  } else if (type == "rename") {
    const auto newName = message.value("name").toString().trimmed();
    if (newName.isEmpty()) {
      return;
    }
    guest.name = newName.toStdString();
    {
      std::lock_guard lock(m_stateMutex);
      for (auto &member : m_members) {
        if (member.id == guest.id) {
          member.displayName = guest.name;
        }
      }
    }
    broadcastToGuests({{"t", "member-renamed"},
                       {"id", QString::number(guest.id)},
                       {"name", newName}},
                      socket);
    if (m_events.memberRenamed) {
      m_events.memberRenamed(guest.id, guest.name);
    }
  } else if (type == "signal") {
    const auto to = message.value("to").toString().toULongLong();
    if (to == m_hostId) {
      if (m_events.signalReceived) {
        m_events.signalReceived(guest.id,
                                message.value("payload").toString()
                                    .toStdString());
      }
      return;
    }
    for (auto &[otherSocket, other] : m_guests) {
      if (other.id == to) {
        auto relayed = message;
        relayed["from"] = QString::number(guest.id);
        otherSocket->write(encodeLine(relayed));
        break;
      }
    }
  }
}

void DirectLobbyBackend::guestSocketClosed(QTcpSocket *socket) {
  const auto it = m_guests.find(socket);
  if (it == m_guests.end()) {
    return;
  }
  const auto id = it->second.id;
  const bool announced = it->second.helloReceived;
  m_guests.erase(it);
  socket->deleteLater();
  if (!announced) {
    return;
  }
  {
    std::lock_guard lock(m_stateMutex);
    std::erase_if(m_members,
                  [id](const LobbyMember &member) { return member.id == id; });
  }
  broadcastToGuests({{"t", "member-left"}, {"id", QString::number(id)}},
                    nullptr);
  if (m_events.memberLeft) {
    m_events.memberLeft(id);
  }
}

void DirectLobbyBackend::broadcastToGuests(const QJsonObject &message,
                                           QTcpSocket *except) {
  const auto line = encodeLine(message);
  for (auto &[socket, guest] : m_guests) {
    if (socket != except && guest.helloReceived) {
      socket->write(line);
    }
  }
}

// --- guest ---

void DirectLobbyBackend::joinLobby(const std::string &hostAddress,
                                   ResultCallback done) {
  if (m_server || m_hostSocket) {
    if (done) {
      done(false, "Already in a lobby");
    }
    return;
  }

  const auto input = QString::fromStdString(hostAddress).trimmed();
  const auto host = input.section(':', 0, 0);
  const auto port = input.contains(':')
                        ? static_cast<uint16_t>(input.section(':', 1).toUInt())
                        : DEFAULT_PORT;
  if (host.isEmpty() || port == 0) {
    if (done) {
      done(false, "Enter the host's IP address");
    }
    return;
  }

  m_pendingJoin = std::move(done);
  m_hostSocket = new QTcpSocket(this);
  m_hostBuffer.clear();

  connect(m_hostSocket, &QTcpSocket::connected, this, [this] {
    m_hostSocket->write(encodeLine(
        {{"t", "hello"}, {"name", QString::fromStdString(m_displayName)}}));
  });
  connect(m_hostSocket, &QTcpSocket::readyRead, this, [this] {
    m_hostBuffer += m_hostSocket->readAll();
    drainLines(m_hostBuffer,
               [this](const QJsonObject &m) { handleHostLine(m); });
  });
  connect(m_hostSocket, &QTcpSocket::disconnected, this,
          [this] { hostSocketClosed(); });
  connect(m_hostSocket, &QTcpSocket::errorOccurred, this,
          [this](QAbstractSocket::SocketError) {
            if (m_pendingJoin) {
              const auto reason = m_hostSocket
                                      ? m_hostSocket->errorString().toStdString()
                                      : "Connection failed";
              const auto pending = std::move(m_pendingJoin);
              m_pendingJoin = nullptr;
              resetState();
              pending(false, reason);
            }
          });

  // Cap the OS connect timeout at something a person will wait through
  QTimer::singleShot(10000, this, [this] {
    if (m_pendingJoin && m_hostSocket &&
        m_hostSocket->state() != QAbstractSocket::ConnectedState) {
      m_hostSocket->abort();
    }
  });

  m_hostSocket->connectToHost(host, port);
}

void DirectLobbyBackend::handleHostLine(const QJsonObject &message) {
  const auto type = message.value("t").toString();

  if (type == "welcome") {
    {
      std::lock_guard lock(m_stateMutex);
      m_joined = true;
      m_isHost = false;
      m_selfId = message.value("id").toString().toULongLong();
      m_hostId = message.value("hostId").toString().toULongLong();
      m_joinCodeDisplay =
          m_hostSocket->peerAddress().toString().toStdString() + ":" +
          std::to_string(m_hostSocket->peerPort());
      m_members.clear();
      for (const auto &entry : message.value("members").toArray()) {
        const auto member = entry.toObject();
        m_members.push_back(
            LobbyMember{member.value("id").toString().toULongLong(),
                        member.value("name").toString().toStdString()});
      }
    }
    if (m_pendingJoin) {
      const auto pending = std::move(m_pendingJoin);
      m_pendingJoin = nullptr;
      pending(true, "");
    }
    return;
  }

  if (type == "member-joined") {
    const LobbyMember member{message.value("id").toString().toULongLong(),
                             message.value("name").toString().toStdString()};
    {
      std::lock_guard lock(m_stateMutex);
      m_members.push_back(member);
    }
    if (m_events.memberJoined) {
      m_events.memberJoined(member);
    }
  } else if (type == "member-left") {
    const auto id = message.value("id").toString().toULongLong();
    {
      std::lock_guard lock(m_stateMutex);
      std::erase_if(m_members, [id](const LobbyMember &member) {
        return member.id == id;
      });
    }
    if (m_events.memberLeft) {
      m_events.memberLeft(id);
    }
  } else if (type == "member-renamed") {
    const auto id = message.value("id").toString().toULongLong();
    const auto name = message.value("name").toString().toStdString();
    {
      std::lock_guard lock(m_stateMutex);
      for (auto &member : m_members) {
        if (member.id == id) {
          member.displayName = name;
        }
      }
    }
    if (m_events.memberRenamed) {
      m_events.memberRenamed(id, name);
    }
  } else if (type == "chat") {
    if (m_events.chatReceived) {
      m_events.chatReceived(message.value("from").toString().toULongLong(),
                            message.value("name").toString().toStdString(),
                            message.value("text").toString().toStdString());
    }
  } else if (type == "signal") {
    if (m_events.signalReceived) {
      m_events.signalReceived(message.value("from").toString().toULongLong(),
                              message.value("payload").toString()
                                  .toStdString());
    }
  }
}

void DirectLobbyBackend::hostSocketClosed() {
  const bool wasJoined = [this] {
    std::lock_guard lock(m_stateMutex);
    return m_joined;
  }();
  if (m_pendingJoin) {
    const auto pending = std::move(m_pendingJoin);
    m_pendingJoin = nullptr;
    resetState();
    pending(false, "Couldn't reach the host");
    return;
  }
  resetState();
  if (wasJoined && m_events.lobbyClosed) {
    m_events.lobbyClosed();
  }
}

// --- both ---

void DirectLobbyBackend::leaveLobby() {
  // Marshal: teardown touches sockets, which live on the main thread
  QMetaObject::invokeMethod(this, [this] { resetState(); },
                            Qt::QueuedConnection);
}

void DirectLobbyBackend::sendChat(const std::string &text) {
  QMetaObject::invokeMethod(
      this,
      [this, text] {
        if (m_server) {
          std::string name;
          {
            std::lock_guard lock(m_stateMutex);
            name = m_displayName;
          }
          broadcastToGuests({{"t", "chat"},
                             {"from", QString::number(m_hostId)},
                             {"name", QString::fromStdString(name)},
                             {"text", QString::fromStdString(text)}},
                            nullptr);
        } else if (m_hostSocket) {
          m_hostSocket->write(encodeLine(
              {{"t", "chat"}, {"text", QString::fromStdString(text)}}));
        }
      },
      Qt::QueuedConnection);
}

void DirectLobbyBackend::sendSignal(const PlayerId to,
                                    const std::string &payload) {
  QMetaObject::invokeMethod(
      this,
      [this, to, payload] {
        const QJsonObject message{
            {"t", "signal"},
            {"from", QString::number(m_selfId)},
            {"to", QString::number(to)},
            {"payload", QString::fromStdString(payload)}};
        if (m_server) {
          for (auto &[socket, guest] : m_guests) {
            if (guest.id == to) {
              socket->write(encodeLine(message));
              return;
            }
          }
        } else if (m_hostSocket) {
          m_hostSocket->write(encodeLine(message));
        }
      },
      Qt::QueuedConnection);
}

LobbyInfo DirectLobbyBackend::currentLobby() const {
  std::lock_guard lock(m_stateMutex);
  LobbyInfo info;
  info.self = PlayerIdentity{m_selfId, m_displayName};
  info.joined = m_joined;
  if (!m_joined) {
    return info;
  }
  info.joinCode = m_joinCodeDisplay;
  info.hostId = m_hostId;
  info.members = m_members;
  return info;
}

void DirectLobbyBackend::resetState() {
  if (m_server) {
    for (auto &[socket, guest] : m_guests) {
      socket->disconnect(this);
      socket->close();
      socket->deleteLater();
    }
    m_guests.clear();
    m_server->close();
    m_server->deleteLater();
    m_server = nullptr;
  }
  if (m_hostSocket) {
    m_hostSocket->disconnect(this);
    m_hostSocket->close();
    m_hostSocket->deleteLater();
    m_hostSocket = nullptr;
  }
  m_nextGuestId = 2;
  m_hostBuffer.clear();

  std::lock_guard lock(m_stateMutex);
  m_joined = false;
  m_isHost = false;
  m_selfId = 1;
  m_hostId = 1;
  m_joinCodeDisplay.clear();
  m_members.clear();
}

std::string DirectLobbyBackend::listenAddressDisplay() const {
  QString address = "127.0.0.1";
  for (const auto &candidate : QNetworkInterface::allAddresses()) {
    if (candidate.protocol() == QAbstractSocket::IPv4Protocol &&
        !candidate.isLoopback()) {
      address = candidate.toString();
      break;
    }
  }
  return address.toStdString() + ":" + std::to_string(DEFAULT_PORT);
}

} // namespace firelight::netplay
