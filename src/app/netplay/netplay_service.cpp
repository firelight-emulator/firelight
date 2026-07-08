#include "netplay_service.hpp"

#include "../emulation/emulation_service.hpp"

#include <firelight/netplay/stream_packets.hpp>
#include <rcheevos/ra_client.hpp>
#include <spdlog/spdlog.h>

namespace firelight::netplay {

namespace {
std::string httpUrlOrEmpty(const std::string &url) {
  return url.starts_with("http://") || url.starts_with("https://") ? url : "";
}
} // namespace

NetplayService::NetplayService(
    ILobbyBackend &lobbyBackend, IPeerTransport &transport,
    library::UserLibraryService &library, std::string appVersion,
    achievements::RAClient *achievementClient,
    libretro::IRetropadProvider *localPads,
    std::function<std::shared_ptr<IAudioOutput>()> guestAudioFactory)
    : m_lobbyBackend(lobbyBackend), m_library(library),
      m_achievementClient(achievementClient),
      m_session(lobbyBackend, transport, std::move(appVersion)),
      m_retropadProvider(localPads, m_session),
      m_receiver(m_session, std::move(guestAudioFactory), localPads) {
  m_sender.setConfigReadyCallback([this](StreamConfig config) {
    m_session.announceStreamConfig(config);
  });
  SessionEvents events;
  events.lobbyChanged = [this] {
    autoAssignMembers();
    if (m_forward.lobbyChanged) {
      m_forward.lobbyChanged();
    }
  };
  events.slotsChanged = [this] {
    if (m_forward.slotsChanged) {
      m_forward.slotsChanged();
    }
  };
  events.gameChanged = [this] {
    if (m_forward.gameChanged) {
      m_forward.gameChanged();
    }
  };
  events.phaseChanged = [this](const GamePhase phase) {
    handlePhase(phase);
    if (m_forward.phaseChanged) {
      m_forward.phaseChanged(phase);
    }
  };
  events.chatMessageAdded = [this](const ChatMessage &message) {
    if (m_forward.chatMessageAdded) {
      m_forward.chatMessageAdded(message);
    }
  };
  events.streamConfigReceived = [this](const StreamConfig &config) {
    if (!m_session.isHost()) {
      m_receiver.configure(config);
    }
    if (m_forward.streamConfigReceived) {
      m_forward.streamConfigReceived(config);
    }
  };
  events.pauseChanged = [this](const bool paused) {
    if (m_forward.pauseChanged) {
      m_forward.pauseChanged(paused);
    }
  };
  events.peerReady = [this](const PlayerId id, IPeerLink &link) {
    m_sender.onPeerReady(id, link);
    if (m_forward.peerReady) {
      m_forward.peerReady(id, link);
    }
  };
  events.peerLost = [this](const PlayerId id) {
    m_sender.onPeerLost(id);
    if (m_forward.peerLost) {
      m_forward.peerLost(id);
    }
  };
  events.lobbyEnded = [this](const std::string &reason) {
    m_selectedEntryId = -1;
    m_sender.disarm();
    m_receiver.stopPlaying();
    m_retropadProvider.setActive(false);
    restoreHardcoreIfSuppressed();
    if (m_forward.lobbyEnded) {
      m_forward.lobbyEnded(reason);
    }
  };
  events.packetReceived = [this](const PlayerId id, const ChannelKind channel,
                                 std::span<const uint8_t> data) {
    switch (channel) {
    case ChannelKind::Input: {
      // Host: a guest's controller state for their slot.
      if (const auto packet = decodeInputPacket(data);
          packet && !packet->frames.empty()) {
        m_retropadProvider.padForMember(id)->applyFrame(
            packet->seq,
            input::InputFrame::deserialize(packet->frames.front()));
      }
      break;
    }
    case ChannelKind::Video:
      m_receiver.onVideoPacket(data);
      break;
    case ChannelKind::Audio:
      m_receiver.onAudioPacket(data);
      break;
    default:
      break;
    }
    if (m_forward.packetReceived) {
      m_forward.packetReceived(id, channel, data);
    }
  };
  m_session.setEvents(std::move(events));

  m_emulationStoppedConnection =
      EventDispatcher::instance().subscribe<emulation::EmulationStoppedEvent>(
          [this](const emulation::EmulationStoppedEvent &) {
            if (m_session.isHost() &&
                m_session.phase() == GamePhase::InGame) {
              m_session.endGame("finished");
            }
          });
}

void NetplayService::setEvents(SessionEvents events) {
  m_forward = std::move(events);
}

void NetplayService::signIn(std::function<void(bool)> done) {
  m_lobbyBackend.beginSignIn(std::move(done));
}

SignInState NetplayService::signInState() const {
  return m_lobbyBackend.signInState();
}

PlayerIdentity NetplayService::localIdentity() const {
  return m_lobbyBackend.localIdentity();
}

std::string NetplayService::providerName() const {
  return m_lobbyBackend.providerName();
}

void NetplayService::setPlayerName(const std::string &name) {
  m_lobbyBackend.setPreferredDisplayName(name);
}

void NetplayService::hostLobby(ILobbyBackend::ResultCallback done) {
  m_session.hostLobby(
      [this, done = std::move(done)](const bool ok, const std::string &error) {
        if (ok) {
          autoAssignMembers();
        }
        if (done) {
          done(ok, error);
        }
      });
}

void NetplayService::joinLobby(const std::string &joinCode,
                               ILobbyBackend::ResultCallback done) {
  m_session.joinLobby(joinCode, std::move(done));
}

void NetplayService::leaveLobby() { m_session.leaveLobby(); }

void NetplayService::assignSlot(const int slot, const PlayerId memberId,
                                const int localPadIndex) {
  m_session.assignSlot(slot, memberId, localPadIndex);
}

void NetplayService::clearSlot(const int slot) { m_session.clearSlot(slot); }

void NetplayService::setReady(const bool ready) { m_session.setReady(ready); }

void NetplayService::selectGameByEntryId(const int entryId) {
  const auto entry = m_library.getEntry(entryId);
  if (!entry) {
    spdlog::warn("[Netplay] no library entry with id {}", entryId);
    return;
  }
  m_selectedEntryId = entryId;
  m_session.selectGame(SessionDescriptor{
      .gameName = entry->displayName,
      .contentHash = entry->contentHash,
      .platformId = static_cast<int>(entry->platformId),
      .artUrl = httpUrlOrEmpty(entry->boxartFrontSourceUrl.empty()
                                   ? entry->icon1x1SourceUrl
                                   : entry->boxartFrontSourceUrl),
      .strategy = SyncStrategyKind::HostStream,
  });
}

void NetplayService::sendChat(const std::string &text) {
  m_session.sendChat(text);
}

bool NetplayService::startGame() {
  if (!m_session.isHost() || !m_session.game() ||
      m_session.phase() != GamePhase::Idle) {
    return false;
  }
  // The encoder announces its config once frames flow (announceStreamConfig).
  m_session.startGame(std::nullopt);
  return true;
}

void NetplayService::confirmLaunch() {
  if (!m_session.isHost() || m_session.phase() != GamePhase::Starting) {
    return;
  }
  // Remote inputs would trip RetroAchievements hardcore; suppress for the
  // duration of the online game.
  if (m_achievementClient &&
      m_session.slotTable().anyRemoteOccupant(m_session.lobby().hostId) &&
      m_achievementClient->hardcoreModeActive()) {
    m_achievementClient->setDefaultToHardcore(false);
    m_hardcoreSuppressed = true;
    spdlog::info("[Netplay] hardcore mode off for online play");
  }
  m_sender.arm();
  m_session.markStreamStarted();
}

void NetplayService::endGame(const std::string &reason) {
  m_sender.disarm();
  restoreHardcoreIfSuppressed();
  m_session.endGame(reason);
}

void NetplayService::restoreHardcoreIfSuppressed() {
  if (m_hardcoreSuppressed && m_achievementClient) {
    m_achievementClient->setDefaultToHardcore(true);
    m_hardcoreSuppressed = false;
  }
}

void NetplayService::handlePhase(const GamePhase phase) {
  const bool isHost = m_session.isHost();
  if (phase == GamePhase::InGame) {
    if (isHost) {
      m_retropadProvider.setActive(true);
    } else {
      const auto game = m_session.game();
      m_receiver.startPlaying(game ? game->platformId : 0);
    }
    return;
  }
  if (phase == GamePhase::Idle) {
    m_retropadProvider.setActive(false);
    m_receiver.stopPlaying();
  }
}

int NetplayService::selectedEntryId() const {
  if (m_selectedEntryId >= 0) {
    return m_selectedEntryId;
  }
  // Guests (and a host after restart) can resolve through the content hash.
  if (const auto game = m_session.game()) {
    if (const auto entry = m_library.getEntryWithContentHash(game->contentHash)) {
      return entry->id;
    }
  }
  return -1;
}

void NetplayService::autoAssignMembers() {
  if (!m_session.isHost()) {
    return;
  }
  const auto lobby = m_session.lobby();
  for (const auto &member : lobby.members) {
    const auto table = m_session.slotTable();
    if (!table.slotsFor(member.id).empty()) {
      continue;
    }
    for (int slot = 0; slot < MAX_SLOTS; ++slot) {
      if (!table.slot(slot).has_value()) {
        m_session.assignSlot(slot, member.id, 0);
        break;
      }
    }
  }
}

} // namespace firelight::netplay
