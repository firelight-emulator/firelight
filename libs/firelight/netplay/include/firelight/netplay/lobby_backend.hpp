#pragma once

#include "protocol.hpp"

#include <functional>
#include <string>
#include <vector>

namespace firelight::netplay {

struct PlayerIdentity {
  PlayerId id = 0;
  std::string displayName;
};

enum class SignInState { SignedOut, SigningIn, Ready, Failed };

struct LobbyMember {
  PlayerId id = 0;
  std::string displayName;
};

struct LobbyInfo {
  bool joined = false;
  std::string joinCode;
  PlayerId hostId = 0;
  PlayerIdentity self;
  std::vector<LobbyMember> members;
};

// TODO
// Callbacks a backend fires as the lobby changes. chatReceived fires only for
// REMOTE senders — the session appends the local user's messages itself
// memberRenamed fires for every rename, the local user's included
struct LobbyEvents {
  std::function<void(const LobbyMember &)> memberJoined;
  std::function<void(PlayerId)> memberLeft;
  std::function<void(PlayerId, const std::string &newName)> memberRenamed;
  std::function<void(PlayerId senderId, const std::string &senderName,
                     const std::string &text)>
      chatReceived;
  std::function<void(PlayerId from, const std::string &payload)> signalReceived;
  std::function<void()> lobbyClosed;
};

// TODO
// The only seam the rest of the app sees for lobbies, chat, connection
// signaling, AND identity — implementations (Discord today, LAN or a custom
// server later) keep their sign-in flows and tokens internal.
class ILobbyBackend {
public:
  using ResultCallback = std::function<void(bool ok, const std::string &error)>;

  virtual ~ILobbyBackend() = default;

  virtual void beginSignIn(std::function<void(bool ok)> done) = 0;
  [[nodiscard]] virtual SignInState signInState() const = 0;
  [[nodiscard]] virtual PlayerIdentity localIdentity() const = 0;
  // Human-readable provider name for UI copy ("Discord")
  [[nodiscard]] virtual std::string providerName() const = 0;
  // Backends without account-owned names accept a user-chosen one; applies to
  // the next lobby joined. Provider-named backends ignore it
  virtual void setPreferredDisplayName(const std::string &) {}

  virtual void createLobby(const std::string &joinCode,
                           ResultCallback done) = 0;
  virtual void joinLobby(const std::string &joinCode, ResultCallback done) = 0;
  virtual void leaveLobby() = 0;

  virtual void sendChat(const std::string &text) = 0;
  // Carries connection-handshake payloads (e.g. SDP) to one member
  virtual void sendSignal(PlayerId to, const std::string &payload) = 0;

  [[nodiscard]] virtual LobbyInfo currentLobby() const = 0;

  void setEvents(LobbyEvents events) { m_events = std::move(events); }

protected:
  LobbyEvents m_events;
};

} // namespace firelight::netplay
