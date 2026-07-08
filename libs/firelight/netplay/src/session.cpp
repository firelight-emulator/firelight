#include <firelight/netplay/protocol.hpp>
#include <firelight/netplay/session.hpp>

#include <spdlog/spdlog.h>

#include <chrono>

namespace firelight::netplay {

namespace {
int64_t nowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}
} // namespace

NetplaySession::NetplaySession(ILobbyBackend &lobbyBackend,
                               IPeerTransport &transport,
                               std::string appVersion)
    : m_lobbyBackend(lobbyBackend), m_transport(transport),
      m_appVersion(std::move(appVersion)) {
  wireBackendEvents();
  wireTransportEvents();
}

NetplaySession::~NetplaySession() {
  m_lobbyBackend.setEvents({});
  m_transport.setEvents({});
  m_transport.setSignalOut(nullptr);
  m_transport.closeAll();
}

void NetplaySession::setEvents(SessionEvents events) {
  m_events = std::move(events);
}

// --- host ---

void NetplaySession::hostLobby(ILobbyBackend::ResultCallback done) {
  m_lobbyBackend.createLobby(
      generateJoinCode(),
      [this, done = std::move(done)](const bool ok, const std::string &error) {
        {
          std::lock_guard lock(m_mutex);
          m_isHost = ok;
          m_inLobby = ok;
        }
        if (ok && m_events.lobbyChanged) {
          m_events.lobbyChanged();
        }
        if (done) {
          done(ok, error);
        }
      });
}

void NetplaySession::assignSlot(const int slot, const PlayerId memberId,
                                const int localPadIndex) {
  {
    std::lock_guard lock(m_mutex);
    if (!m_isHost) {
      return;
    }
    std::string displayName;
    for (const auto &member : m_lobbyBackend.currentLobby().members) {
      if (member.id == memberId) {
        displayName = member.displayName;
        break;
      }
    }
    if (!m_slots.assign(slot, SlotAssignment{.memberId = memberId,
                                             .localPadIndex = localPadIndex,
                                             .displayName = displayName})) {
      return;
    }
  }
  broadcastSlots();
  if (m_events.slotsChanged) {
    m_events.slotsChanged();
  }
}

void NetplaySession::clearSlot(const int slot) {
  {
    std::lock_guard lock(m_mutex);
    if (!m_isHost) {
      return;
    }
    m_slots.clear(slot);
  }
  broadcastSlots();
  if (m_events.slotsChanged) {
    m_events.slotsChanged();
  }
}

void NetplaySession::selectGame(SessionDescriptor game) {
  {
    std::lock_guard lock(m_mutex);
    if (!m_isHost) {
      return;
    }
    m_game = std::move(game);
  }
  broadcastControl(GameSelected{.game = *this->game()});
  if (m_events.gameChanged) {
    m_events.gameChanged();
  }
}

void NetplaySession::startGame(std::optional<StreamConfig> streamConfig) {
  {
    std::lock_guard lock(m_mutex);
    if (!m_isHost || m_phase != GamePhase::Idle) {
      return;
    }
    m_phase = GamePhase::Starting;
    m_streamConfig = std::move(streamConfig);
  }
  broadcastControl(SessionStarting{});
  if (const auto config = this->streamConfig()) {
    broadcastControl(*config);
  }
  if (m_events.phaseChanged) {
    m_events.phaseChanged(GamePhase::Starting);
  }
}

void NetplaySession::announceStreamConfig(const StreamConfig &config) {
  {
    std::lock_guard lock(m_mutex);
    if (!m_isHost || m_phase == GamePhase::Idle) {
      return;
    }
    m_streamConfig = config;
  }
  broadcastControl(config);
}

void NetplaySession::markStreamStarted() {
  {
    std::lock_guard lock(m_mutex);
    if (!m_isHost || m_phase != GamePhase::Starting) {
      return;
    }
    m_phase = GamePhase::InGame;
  }
  broadcastControl(StreamStart{});
  if (m_events.phaseChanged) {
    m_events.phaseChanged(GamePhase::InGame);
  }
}

void NetplaySession::setPaused(const bool paused) {
  {
    std::lock_guard lock(m_mutex);
    if (!m_isHost) {
      return;
    }
    m_paused = paused;
  }
  broadcastControl(PauseState{.paused = paused});
  if (m_events.pauseChanged) {
    m_events.pauseChanged(paused);
  }
}

void NetplaySession::endGame(const std::string &reason) {
  {
    std::lock_guard lock(m_mutex);
    if (!m_isHost || m_phase == GamePhase::Idle) {
      return;
    }
    m_phase = GamePhase::Idle;
    m_paused = false;
    m_streamConfig.reset();
    for (int i = 0; i < MAX_SLOTS; ++i) {
      if (const auto &slot = m_slots.slot(i)) {
        m_slots.setReady(slot->memberId, false);
      }
    }
  }
  broadcastControl(SessionEnded{.reason = reason});
  broadcastSlots();
  if (m_events.phaseChanged) {
    m_events.phaseChanged(GamePhase::Idle);
  }
  if (m_events.slotsChanged) {
    m_events.slotsChanged();
  }
}

// --- guest ---

void NetplaySession::joinLobby(const std::string &joinCode,
                               ILobbyBackend::ResultCallback done) {
  m_lobbyBackend.joinLobby(
      joinCode,
      [this, done = std::move(done)](const bool ok, const std::string &error) {
        PlayerId hostId = 0;
        {
          std::lock_guard lock(m_mutex);
          m_isHost = false;
          m_inLobby = ok;
          if (ok) {
            hostId = m_lobbyBackend.currentLobby().hostId;
          }
        }
        if (ok) {
          if (m_events.lobbyChanged) {
            m_events.lobbyChanged();
          }
          m_transport.connectToPeer(hostId, true);
        }
        if (done) {
          done(ok, error);
        }
      });
}

void NetplaySession::setReady(const bool ready) {
  bool isHost = false;
  PlayerId selfId = 0;
  {
    std::lock_guard lock(m_mutex);
    if (!m_inLobby) {
      return;
    }
    isHost = m_isHost;
    selfId = m_lobbyBackend.currentLobby().self.id;
    if (isHost) {
      m_slots.setReady(selfId, ready);
    }
  }
  if (isHost) {
    broadcastSlots();
    if (m_events.slotsChanged) {
      m_events.slotsChanged();
    }
  } else {
    sendToHost(ChannelKind::Control,
               [&] {
                 const auto text = encodeMessage(ReadyState{.ready = ready});
                 return std::vector<uint8_t>(text.begin(), text.end());
               }());
  }
}

void NetplaySession::reconnectToHost() {
  PlayerId hostId = 0;
  {
    std::lock_guard lock(m_mutex);
    if (!m_inLobby || m_isHost) {
      return;
    }
    hostId = m_lobbyBackend.currentLobby().hostId;
  }
  m_transport.connectToPeer(hostId, true);
}

// --- both ---

void NetplaySession::leaveLobby() { teardown("left", true); }

void NetplaySession::sendChat(const std::string &text) {
  ChatMessage message;
  {
    std::lock_guard lock(m_mutex);
    if (!m_inLobby) {
      return;
    }
    const auto self = m_lobbyBackend.currentLobby().self;
    message = ChatMessage{.senderId = self.id,
                          .senderName = self.displayName,
                          .text = text,
                          .timestampMs = nowMs()};
    m_chat.append(message);
  }
  m_lobbyBackend.sendChat(text);
  if (m_events.chatMessageAdded) {
    m_events.chatMessageAdded(message);
  }
}

void NetplaySession::sendToMember(const PlayerId memberId,
                                  const ChannelKind channel,
                                  std::span<const uint8_t> data) {
  IPeerLink *link = nullptr;
  {
    std::lock_guard lock(m_mutex);
    const auto it = m_peers.find(memberId);
    if (it != m_peers.end() && it->second.admitted) {
      link = it->second.link;
    }
  }
  if (link) {
    link->send(channel, data);
  }
}

void NetplaySession::broadcastPacket(const ChannelKind channel,
                                     std::span<const uint8_t> data) {
  std::vector<IPeerLink *> links;
  {
    std::lock_guard lock(m_mutex);
    for (const auto &[id, peer] : m_peers) {
      if (peer.admitted && peer.link) {
        links.push_back(peer.link);
      }
    }
  }
  for (auto *link : links) {
    link->send(channel, data);
  }
}

void NetplaySession::sendToHost(const ChannelKind channel,
                                std::span<const uint8_t> data) {
  IPeerLink *link = nullptr;
  {
    std::lock_guard lock(m_mutex);
    const auto hostId = m_lobbyBackend.currentLobby().hostId;
    const auto it = m_peers.find(hostId);
    if (it != m_peers.end() && it->second.link) {
      link = it->second.link;
    }
  }
  if (link) {
    link->send(channel, data);
  }
}

// --- state reads ---

bool NetplaySession::isHost() const {
  std::lock_guard lock(m_mutex);
  return m_isHost;
}
bool NetplaySession::inLobby() const {
  std::lock_guard lock(m_mutex);
  return m_inLobby;
}
GamePhase NetplaySession::phase() const {
  std::lock_guard lock(m_mutex);
  return m_phase;
}
SlotTable NetplaySession::slotTable() const {
  std::lock_guard lock(m_mutex);
  return m_slots;
}
std::optional<SessionDescriptor> NetplaySession::game() const {
  std::lock_guard lock(m_mutex);
  return m_game;
}
std::optional<StreamConfig> NetplaySession::streamConfig() const {
  std::lock_guard lock(m_mutex);
  return m_streamConfig;
}
std::string NetplaySession::joinCode() const {
  return m_lobbyBackend.currentLobby().joinCode;
}
LobbyInfo NetplaySession::lobby() const {
  return m_lobbyBackend.currentLobby();
}
std::deque<ChatMessage> NetplaySession::chatMessages() const {
  std::lock_guard lock(m_mutex);
  return m_chat.entries();
}
bool NetplaySession::isPaused() const {
  std::lock_guard lock(m_mutex);
  return m_paused;
}
std::vector<PlayerId> NetplaySession::connectedPeers() const {
  std::lock_guard lock(m_mutex);
  std::vector<PlayerId> result;
  for (const auto &[id, peer] : m_peers) {
    if (peer.admitted) {
      result.push_back(id);
    }
  }
  return result;
}

// --- wiring ---

void NetplaySession::wireBackendEvents() {
  LobbyEvents events;
  events.memberJoined = [this](const LobbyMember &) {
    if (m_events.lobbyChanged) {
      m_events.lobbyChanged();
    }
  };
  events.memberLeft = [this](const PlayerId memberId) {
    bool wasHost = false;
    bool clearedSlots = false;
    bool amHost = false;
    {
      std::lock_guard lock(m_mutex);
      if (!m_inLobby) {
        return;
      }
      amHost = m_isHost;
      wasHost = memberId == m_lobbyBackend.currentLobby().hostId && !m_isHost;
      if (m_isHost) {
        clearedSlots = !m_slots.slotsFor(memberId).empty();
        m_slots.clearMember(memberId);
      }
    }
    if (wasHost) {
      teardown("host-left", true);
      return;
    }
    m_transport.closePeer(memberId);
    if (amHost && clearedSlots) {
      broadcastSlots();
      if (m_events.slotsChanged) {
        m_events.slotsChanged();
      }
    }
    if (m_events.lobbyChanged) {
      m_events.lobbyChanged();
    }
  };
  events.memberRenamed = [this](const PlayerId memberId,
                                const std::string &newName) {
    bool renamedSlots = false;
    {
      std::lock_guard lock(m_mutex);
      if (!m_inLobby) {
        return;
      }
      // Slot names are the host's to keep current; guests get the broadcast.
      if (m_isHost) {
        renamedSlots = !m_slots.slotsFor(memberId).empty();
        m_slots.renameMember(memberId, newName);
      }
    }
    if (renamedSlots) {
      broadcastSlots();
      if (m_events.slotsChanged) {
        m_events.slotsChanged();
      }
    }
    if (m_events.lobbyChanged) {
      m_events.lobbyChanged();
    }
  };
  events.chatReceived = [this](const PlayerId senderId,
                               const std::string &senderName,
                               const std::string &text) {
    ChatMessage message{.senderId = senderId,
                        .senderName = senderName,
                        .text = text,
                        .timestampMs = nowMs()};
    {
      std::lock_guard lock(m_mutex);
      if (!m_inLobby) {
        return;
      }
      m_chat.append(message);
    }
    if (m_events.chatMessageAdded) {
      m_events.chatMessageAdded(message);
    }
  };
  events.signalReceived = [this](const PlayerId from,
                                 const std::string &payload) {
    m_transport.handleSignal(from, payload);
  };
  events.lobbyClosed = [this] { teardown("lobby-closed", true); };
  m_lobbyBackend.setEvents(std::move(events));
}

void NetplaySession::wireTransportEvents() {
  m_transport.setSignalOut(
      [this](const PlayerId to, const std::string &payload) {
        m_lobbyBackend.sendSignal(to, payload);
      });

  TransportEvents events;
  events.peerConnected = [this](const PlayerId memberId, IPeerLink &link) {
    bool sendHello = false;
    {
      std::lock_guard lock(m_mutex);
      if (!m_inLobby) {
        return;
      }
      m_peers[memberId] = Peer{.link = &link, .admitted = false};
      // Guests speak first; the host admits on Hello.
      sendHello = !m_isHost;
    }
    if (sendHello) {
      const auto self = m_lobbyBackend.currentLobby().self;
      sendControl(memberId, Hello{.proto = PROTOCOL_VERSION,
                                  .appVersion = m_appVersion,
                                  .memberId = self.id});
    }
  };
  events.peerDisconnected = [this](const PlayerId memberId) {
    {
      std::lock_guard lock(m_mutex);
      m_peers.erase(memberId);
    }
    if (m_events.peerLost) {
      m_events.peerLost(memberId);
    }
  };
  events.messageReceived = [this](const PlayerId from, const ChannelKind channel,
                                  std::span<const uint8_t> data) {
    if (channel == ChannelKind::Control) {
      handleControlMessage(from,
                           std::string(data.begin(), data.end()));
    } else if (m_events.packetReceived) {
      m_events.packetReceived(from, channel, data);
    }
  };
  m_transport.setEvents(std::move(events));
}

// --- control-message handling ---

void NetplaySession::handleControlMessage(const PlayerId from,
                                          const std::string &text) {
  const auto decoded = decodeMessage(text);
  if (!decoded) {
    spdlog::warn("[Netplay] undecodable control message from {}", from);
    return;
  }

  std::visit(
      [&](const auto &message) {
        using T = std::decay_t<decltype(message)>;

        if constexpr (std::is_same_v<T, Hello>) {
          handleHello(from, message);
        } else if constexpr (std::is_same_v<T, Welcome>) {
          handleWelcome(message);
        } else if constexpr (std::is_same_v<T, Reject>) {
          spdlog::info("[Netplay] rejected: {} ({})", message.code,
                       message.message);
          teardown(message.code, true);
        } else if constexpr (std::is_same_v<T, SlotTableMessage>) {
          {
            std::lock_guard lock(m_mutex);
            if (m_isHost) {
              return;
            }
            m_slots = message.table;
          }
          if (m_events.slotsChanged) {
            m_events.slotsChanged();
          }
        } else if constexpr (std::is_same_v<T, GameSelected>) {
          {
            std::lock_guard lock(m_mutex);
            if (m_isHost) {
              return;
            }
            m_game = message.game;
          }
          if (m_events.gameChanged) {
            m_events.gameChanged();
          }
        } else if constexpr (std::is_same_v<T, ReadyState>) {
          bool changed = false;
          {
            std::lock_guard lock(m_mutex);
            if (!m_isHost) {
              return;
            }
            changed = !m_slots.slotsFor(from).empty();
            m_slots.setReady(from, message.ready);
          }
          if (changed) {
            broadcastSlots();
            if (m_events.slotsChanged) {
              m_events.slotsChanged();
            }
          }
        } else if constexpr (std::is_same_v<T, SessionStarting>) {
          {
            std::lock_guard lock(m_mutex);
            if (m_isHost) {
              return;
            }
            m_phase = GamePhase::Starting;
          }
          if (m_events.phaseChanged) {
            m_events.phaseChanged(GamePhase::Starting);
          }
        } else if constexpr (std::is_same_v<T, StreamConfig>) {
          {
            std::lock_guard lock(m_mutex);
            if (m_isHost) {
              return;
            }
            m_streamConfig = message;
          }
          if (m_events.streamConfigReceived) {
            m_events.streamConfigReceived(message);
          }
        } else if constexpr (std::is_same_v<T, StreamStart>) {
          {
            std::lock_guard lock(m_mutex);
            if (m_isHost) {
              return;
            }
            m_phase = GamePhase::InGame;
          }
          if (m_events.phaseChanged) {
            m_events.phaseChanged(GamePhase::InGame);
          }
        } else if constexpr (std::is_same_v<T, PauseState>) {
          {
            std::lock_guard lock(m_mutex);
            if (m_isHost) {
              return;
            }
            m_paused = message.paused;
          }
          if (m_events.pauseChanged) {
            m_events.pauseChanged(message.paused);
          }
        } else if constexpr (std::is_same_v<T, SessionEnded>) {
          {
            std::lock_guard lock(m_mutex);
            if (m_isHost) {
              return;
            }
            m_phase = GamePhase::Idle;
            m_paused = false;
            m_streamConfig.reset();
          }
          if (m_events.phaseChanged) {
            m_events.phaseChanged(GamePhase::Idle);
          }
        } else if constexpr (std::is_same_v<T, Ping>) {
          sendControl(from, Pong{.t = message.t});
        } else if constexpr (std::is_same_v<T, Pong>) {
          // RTT bookkeeping arrives with the transport phase.
        }
      },
      *decoded);
}

void NetplaySession::handleHello(const PlayerId from, const Hello &hello) {
  bool amHost = false;
  bool accepted = false;
  Welcome welcome;
  IPeerLink *link = nullptr;
  {
    std::lock_guard lock(m_mutex);
    amHost = m_isHost;
    if (!amHost) {
      return;
    }
    const auto it = m_peers.find(from);
    if (it == m_peers.end() || !it->second.link) {
      return;
    }
    link = it->second.link;
    accepted = hello.proto == PROTOCOL_VERSION;
    if (accepted) {
      it->second.admitted = true;
      welcome = buildWelcome();
    }
  }
  if (!accepted) {
    sendControl(from, Reject{.code = REJECT_PROTOCOL_MISMATCH,
                             .message = "Update Firelight to play together"});
    m_transport.closePeer(from);
    return;
  }
  sendControl(from, welcome);
  if (m_events.peerReady && link) {
    m_events.peerReady(from, *link);
  }
}

void NetplaySession::handleWelcome(const Welcome &welcome) {
  bool protoOk = false;
  std::optional<StreamConfig> config;
  GamePhase phase = GamePhase::Idle;
  IPeerLink *hostLink = nullptr;
  PlayerId hostId = 0;
  bool hasGame = false;
  {
    std::lock_guard lock(m_mutex);
    if (m_isHost) {
      return;
    }
    protoOk = welcome.proto == PROTOCOL_VERSION;
    if (protoOk) {
      m_phase = welcome.phase;
      m_slots = welcome.table;
      m_game = welcome.game;
      m_streamConfig = welcome.streamConfig;
      config = welcome.streamConfig;
      phase = welcome.phase;
      hasGame = welcome.game.has_value();
      hostId = m_lobbyBackend.currentLobby().hostId;
      const auto it = m_peers.find(hostId);
      if (it != m_peers.end()) {
        it->second.admitted = true;
        hostLink = it->second.link;
      }
    }
  }
  if (!protoOk) {
    teardown(REJECT_PROTOCOL_MISMATCH, true);
    return;
  }
  if (m_events.slotsChanged) {
    m_events.slotsChanged();
  }
  if (hasGame && m_events.gameChanged) {
    m_events.gameChanged();
  }
  if (config && m_events.streamConfigReceived) {
    m_events.streamConfigReceived(*config);
  }
  if (m_events.phaseChanged) {
    m_events.phaseChanged(phase);
  }
  if (m_events.peerReady && hostLink) {
    m_events.peerReady(hostId, *hostLink);
  }
}

// --- helpers ---

void NetplaySession::broadcastSlots() {
  broadcastControl(SlotTableMessage{.table = slotTable()});
}

void NetplaySession::broadcastControl(const ControlMessage &message) {
  const auto text = encodeMessage(message);
  broadcastPacket(ChannelKind::Control,
                  std::span(reinterpret_cast<const uint8_t *>(text.data()),
                            text.size()));
}

void NetplaySession::sendControl(const PlayerId to,
                                 const ControlMessage &message) {
  const auto text = encodeMessage(message);
  IPeerLink *link = nullptr;
  {
    std::lock_guard lock(m_mutex);
    const auto it = m_peers.find(to);
    if (it != m_peers.end()) {
      link = it->second.link;
    }
  }
  if (link) {
    link->send(ChannelKind::Control,
               std::span(reinterpret_cast<const uint8_t *>(text.data()),
                         text.size()));
  }
}

Welcome NetplaySession::buildWelcome() const {
  return Welcome{.proto = PROTOCOL_VERSION,
                 .phase = m_phase,
                 .table = m_slots,
                 .game = m_game,
                 .streamConfig = m_streamConfig};
}

void NetplaySession::teardown(const std::string &reason,
                              const bool fireLobbyEnded) {
  bool wasInLobby = false;
  {
    std::lock_guard lock(m_mutex);
    wasInLobby = m_inLobby;
    m_inLobby = false;
    m_isHost = false;
    m_paused = false;
    m_phase = GamePhase::Idle;
    m_slots = SlotTable{};
    m_game.reset();
    m_streamConfig.reset();
    m_peers.clear();
    m_chat = ChatLog{};
  }
  if (!wasInLobby) {
    return;
  }
  m_transport.closeAll();
  m_lobbyBackend.leaveLobby();
  if (fireLobbyEnded && m_events.lobbyEnded) {
    m_events.lobbyEnded(reason);
  }
}

} // namespace firelight::netplay
