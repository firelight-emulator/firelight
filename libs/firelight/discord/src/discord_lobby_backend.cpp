#include <firelight/discord/discord_lobby_backend.hpp>

#include <firelight/netplay/protocol.hpp>

#include <discordpp.h>
#include <spdlog/spdlog.h>

namespace firelight::discord {

namespace {
constexpr uint64_t APPLICATION_ID = 1208162396921929739;
// Comfortably under Discord's 2,000-char message cap with metadata overhead
constexpr size_t SIGNAL_CHUNK_CHARS = 1200;

const std::string METADATA_HOST_KEY = "host";
const std::string METADATA_PROTO_KEY = "proto";

std::string displayNameOf(const discordpp::UserHandle &user) {
  const auto name = user.DisplayName();
  return name.empty() ? user.Username() : name;
}
} // namespace

DiscordLobbyBackend::DiscordLobbyBackend(discordpp::Client &client,
                                         ITokenStore &tokenStore)
    : m_client(client), m_tokenStore(tokenStore) {
  registerCallbacks();
}

// --- sign-in ---

void DiscordLobbyBackend::beginSignIn(std::function<void(bool)> done) {
  if (m_signInState == netplay::SignInState::Ready) {
    if (done) {
      done(true);
    }
    return;
  }
  if (m_signInState == netplay::SignInState::SigningIn) {
    if (done) {
      done(false);
    }
    return;
  }

  m_signInState = netplay::SignInState::SigningIn;
  m_signInDone = std::move(done);
  m_triedRefresh = false;

  if (const auto stored = m_tokenStore.load()) {
    connectWithToken(stored->accessToken);
  } else {
    beginFullAuthorization();
  }
}

netplay::SignInState DiscordLobbyBackend::signInState() const {
  return m_signInState;
}

netplay::PlayerIdentity DiscordLobbyBackend::localIdentity() const {
  if (m_signInState != netplay::SignInState::Ready) {
    return {};
  }
  const auto user = m_client.GetCurrentUser();
  return netplay::PlayerIdentity{user.Id(), displayNameOf(user)};
}

void DiscordLobbyBackend::connectWithToken(const std::string &accessToken) {
  m_client.UpdateToken(
      discordpp::AuthorizationTokenType::User, accessToken,
      [this](const discordpp::ClientResult &result) {
        if (result.Successful()) {
          m_client.Connect();
        } else {
          spdlog::info("[Discord] stored token rejected: {}", result.Error());
          beginFullAuthorization();
        }
      });
}

void DiscordLobbyBackend::beginFullAuthorization() {
  const auto verifier = m_client.CreateAuthorizationCodeVerifier();

  discordpp::AuthorizationArgs args{};
  args.SetClientId(APPLICATION_ID);
  args.SetScopes(discordpp::Client::GetDefaultCommunicationScopes());
  args.SetCodeChallenge(verifier.Challenge());

  m_client.Authorize(args, [this, verifier](
                               const discordpp::ClientResult &result,
                               const std::string &code,
                               const std::string &redirectUri) {
    if (!result.Successful()) {
      spdlog::warn("[Discord] authorization declined: {}", result.Error());
      finishSignIn(false);
      return;
    }
    m_client.GetToken(
        APPLICATION_ID, code, verifier.Verifier(), redirectUri,
        [this](const discordpp::ClientResult &tokenResult,
               const std::string &accessToken, const std::string &refreshToken,
               discordpp::AuthorizationTokenType, int32_t,
               const std::string &) {
          if (!tokenResult.Successful()) {
            spdlog::warn("[Discord] token exchange failed: {}",
                         tokenResult.Error());
            finishSignIn(false);
            return;
          }
          m_tokenStore.save({accessToken, refreshToken});
          connectWithToken(accessToken);
        });
  });
}

void DiscordLobbyBackend::refreshWithStoredToken() {
  const auto stored = m_tokenStore.load();
  if (!stored || stored->refreshToken.empty()) {
    if (m_signInState == netplay::SignInState::SigningIn) {
      beginFullAuthorization();
    }
    return;
  }
  m_client.RefreshToken(
      APPLICATION_ID, stored->refreshToken,
      [this](const discordpp::ClientResult &result,
             const std::string &accessToken, const std::string &refreshToken,
             discordpp::AuthorizationTokenType, int32_t, const std::string &) {
        if (result.Successful()) {
          m_tokenStore.save({accessToken, refreshToken});
          connectWithToken(accessToken);
        } else {
          spdlog::info("[Discord] token refresh failed: {}", result.Error());
          m_tokenStore.clear();
          if (m_signInState == netplay::SignInState::SigningIn) {
            beginFullAuthorization();
          }
        }
      });
}

void DiscordLobbyBackend::finishSignIn(const bool ok) {
  m_signInState =
      ok ? netplay::SignInState::Ready : netplay::SignInState::Failed;
  if (m_signInDone) {
    const auto done = std::move(m_signInDone);
    m_signInDone = nullptr;
    done(ok);
  }
}

// --- lobby ---

void DiscordLobbyBackend::createLobby(const std::string &joinCode,
                                      ResultCallback done) {
  if (m_signInState != netplay::SignInState::Ready) {
    if (done) {
      done(false, "not signed in");
    }
    return;
  }
  const std::unordered_map<std::string, std::string> lobbyMetadata{
      {METADATA_HOST_KEY, std::to_string(m_client.GetCurrentUser().Id())},
      {METADATA_PROTO_KEY, std::to_string(netplay::PROTOCOL_VERSION)},
  };
  m_client.CreateOrJoinLobbyWithMetadata(
      joinCode, lobbyMetadata, {},
      [this, joinCode, done = std::move(done)](
          const discordpp::ClientResult &result, const uint64_t lobbyId) {
        if (!result.Successful()) {
          if (done) {
            done(false, result.Error());
          }
          return;
        }
        m_lobbyId = lobbyId;
        m_joinCode = joinCode;
        if (done) {
          done(true, "");
        }
      });
}

void DiscordLobbyBackend::joinLobby(const std::string &joinCode,
                                    ResultCallback done) {
  if (m_signInState != netplay::SignInState::Ready) {
    if (done) {
      done(false, "not signed in");
    }
    return;
  }
  m_client.CreateOrJoinLobby(
      joinCode,
      [this, joinCode, done = std::move(done)](
          const discordpp::ClientResult &result, const uint64_t lobbyId) {
        if (!result.Successful()) {
          if (done) {
            done(false, result.Error());
          }
          return;
        }
        // CreateOrJoin makes a fresh (host-less) lobby when the code doesn't
        // match one — treat that as "no such lobby"
        const auto handle = m_client.GetLobbyHandle(lobbyId);
        const auto metadata =
            handle ? handle->Metadata()
                   : std::unordered_map<std::string, std::string>{};
        if (!metadata.contains(METADATA_HOST_KEY)) {
          m_client.LeaveLobby(lobbyId, [](const discordpp::ClientResult &) {});
          if (done) {
            done(false, "That code doesn't match an open lobby");
          }
          return;
        }
        if (metadata.contains(METADATA_PROTO_KEY) &&
            metadata.at(METADATA_PROTO_KEY) !=
                std::to_string(netplay::PROTOCOL_VERSION)) {
          m_client.LeaveLobby(lobbyId, [](const discordpp::ClientResult &) {});
          if (done) {
            done(false, "The host is on a different Firelight version");
          }
          return;
        }
        m_lobbyId = lobbyId;
        m_joinCode = joinCode;
        if (done) {
          done(true, "");
        }
      });
}

void DiscordLobbyBackend::leaveLobby() {
  if (m_lobbyId == 0) {
    return;
  }
  m_client.LeaveLobby(m_lobbyId, [](const discordpp::ClientResult &result) {
    if (!result.Successful()) {
      spdlog::warn("[Discord] leave lobby failed: {}", result.Error());
    }
  });
  resetLobbyState();
}

void DiscordLobbyBackend::sendChat(const std::string &text) {
  if (m_lobbyId == 0) {
    return;
  }
  m_client.SendLobbyMessage(
      m_lobbyId, text, [](const discordpp::ClientResult &result, uint64_t) {
        if (!result.Successful()) {
          spdlog::warn("[Discord] chat send failed: {}", result.Error());
        }
      });
}

void DiscordLobbyBackend::sendSignal(const netplay::PlayerId to,
                                     const std::string &payload) {
  if (m_lobbyId == 0) {
    return;
  }
  const auto chunks = netplay::chunkSignal(payload, SIGNAL_CHUNK_CHARS,
                                           netplay::generateSignalId());
  for (const auto &chunk : chunks) {
    const std::unordered_map<std::string, std::string> metadata{
        {"fl", "sig"},
        {"to", std::to_string(to)},
        {"sid", chunk.signalId},
        {"i", std::to_string(chunk.index)},
        {"n", std::to_string(chunk.total)},
    };
    m_client.SendLobbyMessageWithMetadata(
        m_lobbyId, chunk.content, metadata,
        [](const discordpp::ClientResult &result, uint64_t) {
          if (!result.Successful()) {
            spdlog::warn("[Discord] signal send failed: {}", result.Error());
          }
        });
  }
}

netplay::LobbyInfo DiscordLobbyBackend::currentLobby() const {
  netplay::LobbyInfo info;
  info.self = localIdentity();
  if (m_lobbyId == 0) {
    return info;
  }
  const auto handle = m_client.GetLobbyHandle(m_lobbyId);
  if (!handle) {
    return info;
  }
  info.joined = true;
  info.joinCode = m_joinCode;

  const auto metadata = handle->Metadata();
  if (const auto it = metadata.find(METADATA_HOST_KEY); it != metadata.end()) {
    info.hostId = std::stoull(it->second);
  }
  for (const auto memberId : handle->LobbyMemberIds()) {
    std::string name = "Player";
    if (const auto member = handle->GetLobbyMemberHandle(memberId)) {
      if (const auto user = member->User()) {
        name = displayNameOf(*user);
      }
    }
    info.members.push_back(netplay::LobbyMember{memberId, std::move(name)});
  }
  return info;
}

// --- SDK callbacks ---

void DiscordLobbyBackend::registerCallbacks() {
  m_client.SetStatusChangedCallback(
      [this](const discordpp::Client::Status status, discordpp::Client::Error,
             int32_t) {
        if (status == discordpp::Client::Status::Ready) {
          finishSignIn(true);
          return;
        }
        if (status == discordpp::Client::Status::Disconnected) {
          if (m_signInState == netplay::SignInState::SigningIn &&
              !m_triedRefresh) {
            // The stored access token likely expired; try its refresh token
            m_triedRefresh = true;
            refreshWithStoredToken();
          } else if (m_signInState == netplay::SignInState::Ready) {
            m_signInState = netplay::SignInState::SignedOut;
          }
        }
      });

  m_client.SetTokenExpirationCallback([this] { refreshWithStoredToken(); });

  m_client.SetLobbyDeletedCallback([this](const uint64_t lobbyId) {
    if (lobbyId != m_lobbyId) {
      return;
    }
    resetLobbyState();
    if (m_events.lobbyClosed) {
      m_events.lobbyClosed();
    }
  });

  m_client.SetLobbyMemberAddedCallback(
      [this](const uint64_t lobbyId, const uint64_t memberId) {
        if (lobbyId != m_lobbyId || memberId == localIdentity().id) {
          return;
        }
        if (!m_events.memberJoined) {
          return;
        }
        std::string name = "Player";
        if (const auto handle = m_client.GetLobbyHandle(lobbyId)) {
          if (const auto member = handle->GetLobbyMemberHandle(memberId)) {
            if (const auto user = member->User()) {
              name = displayNameOf(*user);
            }
          }
        }
        m_events.memberJoined(netplay::LobbyMember{memberId, std::move(name)});
      });

  m_client.SetLobbyMemberRemovedCallback(
      [this](const uint64_t lobbyId, const uint64_t memberId) {
        if (lobbyId != m_lobbyId || memberId == localIdentity().id) {
          return;
        }
        if (m_events.memberLeft) {
          m_events.memberLeft(memberId);
        }
      });

  m_client.SetMessageCreatedCallback(
      [this](const uint64_t messageId) { handleMessage(messageId); });
}

void DiscordLobbyBackend::handleMessage(const uint64_t messageId) {
  const auto handle = m_client.GetMessageHandle(messageId);
  if (!handle || m_lobbyId == 0) {
    return;
  }
  const auto lobby = handle->Lobby();
  if (!lobby || lobby->Id() != m_lobbyId) {
    return;
  }
  const auto authorId = handle->AuthorId();
  if (authorId == localIdentity().id) {
    return;
  }

  const auto metadata = handle->Metadata();
  if (const auto it = metadata.find("fl");
      it != metadata.end() && it->second == "sig") {
    if (!metadata.contains("to") ||
        metadata.at("to") != std::to_string(localIdentity().id)) {
      return;
    }
    try {
      netplay::SignalChunk chunk{
          .signalId = metadata.at("sid"),
          .index = std::stoi(metadata.at("i")),
          .total = std::stoi(metadata.at("n")),
          .content = handle->Content(),
      };
      if (const auto payload = m_reassembler.accept(authorId, chunk)) {
        if (m_events.signalReceived) {
          m_events.signalReceived(authorId, *payload);
        }
      }
    } catch (const std::exception &) {
      spdlog::warn("[Discord] malformed signal chunk from {}", authorId);
    }
    return;
  }

  if (m_events.chatReceived) {
    std::string name = "Player";
    if (const auto author = handle->Author()) {
      name = displayNameOf(*author);
    }
    m_events.chatReceived(authorId, name, handle->Content());
  }
}

void DiscordLobbyBackend::resetLobbyState() {
  m_lobbyId = 0;
  m_joinCode.clear();
  m_reassembler = netplay::SignalReassembler{};
}

} // namespace firelight::discord
