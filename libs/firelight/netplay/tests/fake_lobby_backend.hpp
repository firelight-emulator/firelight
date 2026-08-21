#pragma once

#include <firelight/netplay/lobby_backend.hpp>

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace firelight::netplay {

class FakeLobbyBackend;

// One in-memory lobby shared by every FakeLobbyBackend attached to the hub —
// the fake stand-in for the provider's servers
struct FakeLobbyHub {
  std::string joinCode;
  PlayerId hostId = 0;
  std::vector<FakeLobbyBackend *> members;
};

class FakeLobbyBackend final : public ILobbyBackend {
public:
  FakeLobbyBackend(std::shared_ptr<FakeLobbyHub> hub, PlayerIdentity identity)
      : m_hub(std::move(hub)), m_identity(std::move(identity)) {}

  void beginSignIn(std::function<void(bool)> done) override {
    if (done) {
      done(true);
    }
  }

  [[nodiscard]] SignInState signInState() const override { return SignInState::Ready; }

  [[nodiscard]] PlayerIdentity localIdentity() const override { return m_identity; }

  [[nodiscard]] std::string providerName() const override { return "Fake"; }

  void setPreferredDisplayName(const std::string &name) override {
    m_identity.displayName = name;
    if (!m_joined) {
      return;
    }
    for (auto *member : m_hub->members) {
      if (member->m_events.memberRenamed) {
        member->m_events.memberRenamed(m_identity.id, name);
      }
    }
  }

  void createLobby(const std::string &joinCode, ResultCallback done) override {
    m_hub->joinCode = joinCode;
    m_hub->hostId = m_identity.id;
    m_hub->members = {this};
    m_joined = true;
    if (done) {
      done(true, "");
    }
  }

  void joinLobby(const std::string &joinCode, ResultCallback done) override {
    if (joinCode != m_hub->joinCode || m_hub->members.empty()) {
      if (done) {
        done(false, "no such lobby");
      }
      return;
    }
    m_hub->members.push_back(this);
    m_joined = true;
    for (auto *other : m_hub->members) {
      if (other != this && other->m_events.memberJoined) {
        other->m_events.memberJoined(LobbyMember{m_identity.id, m_identity.displayName});
      }
    }
    if (done) {
      done(true, "");
    }
  }

  void leaveLobby() override {
    if (!m_joined) {
      return;
    }
    m_joined = false;
    std::erase(m_hub->members, this);
    if (m_identity.id == m_hub->hostId) {
      for (auto *other : m_hub->members) {
        if (other->m_events.lobbyClosed) {
          other->m_events.lobbyClosed();
        }
      }
    } else {
      for (auto *other : m_hub->members) {
        if (other->m_events.memberLeft) {
          other->m_events.memberLeft(m_identity.id);
        }
      }
    }
  }

  void sendChat(const std::string &text) override {
    for (auto *other : m_hub->members) {
      if (other != this && other->m_events.chatReceived) {
        other->m_events.chatReceived(m_identity.id, m_identity.displayName, text);
      }
    }
  }

  void sendSignal(const PlayerId to, const std::string &payload) override {
    for (auto *other : m_hub->members) {
      if (other->m_identity.id == to && other->m_events.signalReceived) {
        other->m_events.signalReceived(m_identity.id, payload);
      }
    }
  }

  [[nodiscard]] LobbyInfo currentLobby() const override {
    LobbyInfo info;
    info.joined = m_joined;
    info.self = m_identity;
    if (!m_joined) {
      return info;
    }
    info.joinCode = m_hub->joinCode;
    info.hostId = m_hub->hostId;
    for (const auto *member : m_hub->members) {
      info.members.push_back(LobbyMember{member->m_identity.id, member->m_identity.displayName});
    }
    return info;
  }

private:
  std::shared_ptr<FakeLobbyHub> m_hub;
  PlayerIdentity m_identity;
  bool m_joined = false;
};

} // namespace firelight::netplay
