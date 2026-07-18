#pragma once

#include <firelight/netplay/lobby_backend.hpp>

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include <map>
#include <mutex>
#include <vector>

namespace firelight::netplay {

// Direct-connection lobby: the host listens on a TCP port and shares their
// IP; guests join by entering it. That one socket carries lobby membership,
// chat, and the WebRTC signaling — the game streams still flow over the peer
// transport. No accounts, no provider
//
// Sockets live on the main thread; calls arriving from network threads
// (sendSignal from the transport) are marshaled onto it. State read by other
// threads (currentLobby) is mirrored behind a mutex
class DirectLobbyBackend final : public QObject, public ILobbyBackend {
  Q_OBJECT

public:
  static constexpr uint16_t DEFAULT_PORT = 47774;

  explicit DirectLobbyBackend(QObject *parent = nullptr);

  void beginSignIn(std::function<void(bool)> done) override;
  [[nodiscard]] SignInState signInState() const override;
  [[nodiscard]] PlayerIdentity localIdentity() const override;
  [[nodiscard]] std::string providerName() const override { return ""; }
  void setPreferredDisplayName(const std::string &name) override;

  void createLobby(const std::string &joinCode, ResultCallback done) override;
  void joinLobby(const std::string &hostAddress, ResultCallback done) override;
  void leaveLobby() override;

  void sendChat(const std::string &text) override;
  void sendSignal(PlayerId to, const std::string &payload) override;

  [[nodiscard]] LobbyInfo currentLobby() const override;

private:
  struct GuestConnection {
    PlayerId id = 0;
    std::string name;
    QByteArray buffer;
    bool helloReceived = false;
  };

  void handleGuestLine(QTcpSocket *socket, const QJsonObject &message);
  void handleHostLine(const QJsonObject &message);
  void guestSocketClosed(QTcpSocket *socket);
  void hostSocketClosed();
  void broadcastToGuests(const QJsonObject &message, QTcpSocket *except);
  void resetState();
  void refreshMirror(); // main thread -> cross-thread-readable state

  [[nodiscard]] std::string listenAddressDisplay() const;

  std::string m_displayName;

  // Host side
  QTcpServer *m_server = nullptr;
  std::map<QTcpSocket *, GuestConnection> m_guests;
  PlayerId m_nextGuestId = 2;

  // Guest side
  QTcpSocket *m_hostSocket = nullptr;
  QByteArray m_hostBuffer;
  ResultCallback m_pendingJoin;

  // Mirror of lobby state, readable from any thread
  mutable std::mutex m_stateMutex;
  bool m_joined = false;
  bool m_isHost = false;
  PlayerId m_selfId = 1;
  PlayerId m_hostId = 1;
  std::string m_joinCodeDisplay;
  std::vector<LobbyMember> m_members;
};

} // namespace firelight::netplay
