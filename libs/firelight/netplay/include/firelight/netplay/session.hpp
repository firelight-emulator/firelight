#pragma once

#include "chat_log.hpp"
#include "lobby_backend.hpp"
#include "messages.hpp"
#include "session_descriptor.hpp"
#include "session_phase.hpp"
#include "slot_table.hpp"
#include "transport.hpp"

#include <map>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace firelight::netplay {

// Everything the session reports outward. Callbacks fire on backend/transport
// threads with no session lock held; the app layer marshals to the GUI thread.
// packetReceived is the hot path (video/audio/input bytes for the stream
// runtimes) and bypasses the session lock, so set events once, before
// hostLobby/joinLobby.
struct SessionEvents {
  std::function<void()> lobbyChanged;
  std::function<void()> slotsChanged;
  std::function<void()> gameChanged;
  std::function<void(GamePhase)> phaseChanged;
  std::function<void(const ChatMessage &)> chatMessageAdded;
  std::function<void(const StreamConfig &)> streamConfigReceived;
  std::function<void(bool paused)> pauseChanged;
  // Control channel is open and the Hello/Welcome exchange succeeded.
  std::function<void(PlayerId, IPeerLink &)> peerReady;
  std::function<void(PlayerId)> peerLost;
  std::function<void(const std::string &reason)> lobbyEnded;
  std::function<void(PlayerId, ChannelKind, std::span<const uint8_t>)>
      packetReceived;
};

// The persistent lobby: membership, slots, chat, and peer links live here from
// lobby creation until the host closes it. Games start and end inside it
// (GamePhase); ending a game returns the lobby to Idle with everything intact.
// Thread-safe: backend and transport callbacks arrive on their own threads.
class NetplaySession {
public:
  NetplaySession(ILobbyBackend &lobbyBackend, IPeerTransport &transport,
                 std::string appVersion);
  // Detaches from the backend/transport and closes peers so their threads
  // can't call into a destroyed session.
  ~NetplaySession();

  void setEvents(SessionEvents events);

  // --- host ---
  void hostLobby(ILobbyBackend::ResultCallback done);
  void assignSlot(int slot, PlayerId memberId, int localPadIndex);
  void clearSlot(int slot);
  void selectGame(SessionDescriptor game);
  // Starting -> broadcast SessionStarting (+ StreamConfig when provided).
  void startGame(std::optional<StreamConfig> streamConfig);
  // The encoder's parameters are only known once frames flow; the host
  // announces them as soon as the stream is live (late joiners get them via
  // Welcome).
  void announceStreamConfig(const StreamConfig &config);
  // InGame -> broadcast StreamStart (host calls once the first IDR is queued).
  void markStreamStarted();
  void setPaused(bool paused);
  // Back to Idle; the lobby survives.
  void endGame(const std::string &reason);

  // --- guest ---
  void joinLobby(const std::string &joinCode,
                 ILobbyBackend::ResultCallback done);
  void setReady(bool ready);
  // Re-establish the peer link after a drop; still a lobby member, so the
  // Hello/Welcome exchange replays the full state snapshot.
  void reconnectToHost();

  // --- both ---
  void leaveLobby();
  void sendChat(const std::string &text);

  // Send helpers for the stream runtimes.
  void sendToMember(PlayerId memberId, ChannelKind channel,
                    std::span<const uint8_t> data);
  void broadcastPacket(ChannelKind channel, std::span<const uint8_t> data);
  void sendToHost(ChannelKind channel, std::span<const uint8_t> data);

  // --- state reads (copies, safe from any thread) ---
  [[nodiscard]] bool isHost() const;
  [[nodiscard]] bool inLobby() const;
  [[nodiscard]] GamePhase phase() const;
  [[nodiscard]] SlotTable slotTable() const;
  [[nodiscard]] std::optional<SessionDescriptor> game() const;
  [[nodiscard]] std::optional<StreamConfig> streamConfig() const;
  [[nodiscard]] std::string joinCode() const;
  [[nodiscard]] LobbyInfo lobby() const;
  [[nodiscard]] std::deque<ChatMessage> chatMessages() const;
  [[nodiscard]] bool isPaused() const;
  [[nodiscard]] std::vector<PlayerId> connectedPeers() const;

private:
  void wireBackendEvents();
  void wireTransportEvents();

  void handleControlMessage(PlayerId from, const std::string &text);
  void handleHello(PlayerId from, const Hello &hello);
  void handleWelcome(const Welcome &welcome);

  // Host: push the current slot table to every admitted peer.
  void broadcastSlots();
  void broadcastControl(const ControlMessage &message);
  void sendControl(PlayerId to, const ControlMessage &message);

  [[nodiscard]] Welcome buildWelcome() const; // caller holds m_mutex
  void teardown(const std::string &reason, bool fireLobbyEnded);

  ILobbyBackend &m_lobbyBackend;
  IPeerTransport &m_transport;
  std::string m_appVersion;
  SessionEvents m_events;

  mutable std::mutex m_mutex;
  bool m_isHost = false;
  bool m_inLobby = false;
  bool m_paused = false;
  GamePhase m_phase = GamePhase::Idle;
  SlotTable m_slots;
  std::optional<SessionDescriptor> m_game;
  std::optional<StreamConfig> m_streamConfig;
  ChatLog m_chat;

  struct Peer {
    IPeerLink *link = nullptr;
    bool admitted = false; // Hello/Welcome exchange completed
  };
  std::map<PlayerId, Peer> m_peers;
};

} // namespace firelight::netplay
