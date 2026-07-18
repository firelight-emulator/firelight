#pragma once

#include <firelight/discord/token_store.hpp>
#include <firelight/netplay/lobby_backend.hpp>
#include <firelight/netplay/signal_chunker.hpp>

#include <cstdint>
#include <functional>
#include <string>

namespace discordpp {
class Client;
}

namespace firelight::discord {

// Maps ILobbyBackend onto the Discord Social SDK: the join code is the lobby
// secret, chat rides SendLobbyMessage, and connection signaling rides
// metadata-tagged lobby messages (chunked, filtered out of chat). Sign-in is
// the SDK OAuth flow with tokens persisted through the token store — nothing
// outside this adapter sees Discord types or tokens
//
// Threading: everything (UI calls + SDK callbacks) runs on the thread that
// pumps RunCallbacks — the main thread
class DiscordLobbyBackend final : public netplay::ILobbyBackend {
public:
  DiscordLobbyBackend(discordpp::Client &client, ITokenStore &tokenStore);

  void beginSignIn(std::function<void(bool ok)> done) override;
  [[nodiscard]] netplay::SignInState signInState() const override;
  [[nodiscard]] netplay::PlayerIdentity localIdentity() const override;
  [[nodiscard]] std::string providerName() const override { return "Discord"; }

  void createLobby(const std::string &joinCode, ResultCallback done) override;
  void joinLobby(const std::string &joinCode, ResultCallback done) override;
  void leaveLobby() override;

  void sendChat(const std::string &text) override;
  void sendSignal(netplay::PlayerId to, const std::string &payload) override;

  [[nodiscard]] netplay::LobbyInfo currentLobby() const override;

private:
  void registerCallbacks();
  void connectWithToken(const std::string &accessToken);
  void beginFullAuthorization();
  void refreshWithStoredToken();
  void finishSignIn(bool ok);
  void handleMessage(uint64_t messageId);
  void resetLobbyState();

  discordpp::Client &m_client;
  ITokenStore &m_tokenStore;

  netplay::SignInState m_signInState = netplay::SignInState::SignedOut;
  std::function<void(bool)> m_signInDone;
  bool m_triedRefresh = false;

  uint64_t m_lobbyId = 0;
  std::string m_joinCode;
  netplay::SignalReassembler m_reassembler;
};

} // namespace firelight::discord
