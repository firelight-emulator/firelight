<!-- SCAFFOLD: the prose in this file was seeded from an automated read of the code so you can rewrite it in your own voice (see activity/ and achievements/ for the tone). The "How it works" bullets are the load-bearing facts that currently live only in comments — fold them into prose, then trim the inline comments. Delete this line when done. -->
# Firelight Netplay Module
Self-contained static lib implementing Firelight's peer-to-peer netplay: a persistent, host-authoritative lobby (membership, P1-P8 slots, chat, game selection, pause/phase) layered over a pluggable lobby/signaling backend and a WebRTC data-channel transport. It carries a host-streamed audio/video/input session between peers and is deliberately Qt-free (opaque byte packets, JSON control messages).

## How it works

---

**Entry point:** NetplaySession

<!-- Load-bearing facts (thread rules, invariants, protocol ordering, gotchas). Rewrite as prose. -->
- Host is authoritative. Guests speak first: on control-channel open a guest sends Hello; the host validates proto==PROTOCOL_VERSION, marks the peer admitted, and replies Welcome. Mismatch -> Reject{protocol-mismatch} + closePeer. (session.cpp handleHello / peerConnected comment 'Guests speak first; the host admits on Hello.')
- Welcome is a FULL state snapshot (phase, slot table, game, stream config). A fresh join, a mid-game join, and a reconnect all recover from this one message — reconnectToHost just replays the handshake. (messages.hpp Welcome comment)
- SessionEvents fire on backend/transport threads with NO session lock held; the app marshals to the GUI thread. packetReceived is the hot path and bypasses the session lock, so events MUST be set once before hostLobby/joinLobby. (session.hpp SessionEvents comment)
- libdatachannel fires callbacks while holding connection-internal locks — even synchronously inside setRemoteDescription when only host candidates exist — so re-entering the connection from a callback can deadlock. RtcTransport queues any re-entrant work (sendLocalDescription, and firing peerConnected because the consumer's first Hello send re-enters) onto its own worker thread via post(). (rtc_transport.cpp Impl comment + channelOpened comment)
- Signaling is non-trickle: each side emits a single offer/answer payload once ICE gathering reaches Complete (onGatheringStateChange). connectToPeer(isOfferer=false) is a no-op — the answering side reacts to the offer in handleSignal. (rtc_transport.hpp + connectToPeer/handleSignal)
- A data channel that is already open before its onMessage/onOpen handler is attached never fires onOpen again (happens with fast in-process/loopback connections); adoptChannel explicitly re-checks isOpen() and calls channelOpened. peerConnected is announced only once all four channels are open. (rtc_transport.cpp adoptChannel/channelOpened)
- TransportEvents ordering contract: peerConnected fires before any messageReceived from that peer, and no messageReceived fires after peerDisconnected. peerDisconnected fires at most once per peer (guarded by Peer.announced && !Peer.dead so close+state-change don't double-fire). (transport.hpp + rtc_transport.cpp onStateChange/closePeer)
- ChannelKind numeric order is load-bearing: Control=0/Video=1/Audio=2/Input=3 index the transport's channel arrays and CHANNEL_LABELS. Control+Video are reliable+ordered; Audio+Input are unreliable+unordered (maxRetransmits=0). (transport.hpp + rtc_transport.cpp initFor)
- Headers avoid the identifier `slots` because Qt #defines it as a macro and these headers are included from Qt code: SlotTable's accessor is all() and Welcome's field is `table`. Renaming them will break the Qt build. (slot_table.hpp / messages.hpp comments)
- RETROPAD_FRAME_BYTES=10 is the serialized retropad frame size, kept as opaque bytes so netplay doesn't depend on the input module; the app glue static_asserts the two constants agree. Changing one silently desyncs input unless the assert is kept. (stream_packets.hpp)
- InputPacket frames are newest-first and include a few older duplicates for loss tolerance; the host applies the highest unseen seq. displayedPtsMs is the video pts the guest was watching (reserved for host-side rollback, unused in v1). (stream_packets.hpp)
- ILobbyBackend.chatReceived fires only for REMOTE senders — the session appends the local user's own chat in sendChat. memberRenamed fires for every rename including the local user. Double-handling local chat if this contract is misread. (lobby_backend.hpp + session.cpp)
- GamePhase lives inside a persistent lobby: the lobby exists from creation until the host closes it; startGame drives Idle->Starting (broadcast SessionStarting, plus StreamConfig if known), markStreamStarted drives Starting->InGame (broadcast StreamStart, host calls once the first IDR is queued), endGame returns to Idle and clears ready flags/stream config while the lobby survives. (session_phase.hpp + session.cpp)
- StreamConfig is announced late: encoder parameters are only known once frames flow, so the host calls announceStreamConfig once the stream is live; late joiners instead receive it inside Welcome. (session.hpp announceStreamConfig)
- The session stores NON-OWNING IPeerLink* in m_peers; the transport owns link lifetimes. The destructor detaches backend/transport events, clears signalOut, then closeAll() — in that order — so transport threads cannot call back into a half-destroyed session. teardown() clears all state and only fires lobbyEnded if it was actually in a lobby. (session.hpp/session.cpp ~NetplaySession, teardown)
- Join codes are FL-XXXX-XXXX in Crockford base32 (alphabet excludes I, L, O, U) so they are unambiguous read aloud. SignalReassembler caps abandoned partials at 32 (clears wholesale) to bound memory. (protocol.cpp / signal_chunker.cpp)
- SignalChunker/SignalReassembler and base64 are standalone helpers for backend implementations that must fit SDP under a provider message-size cap; RtcTransport itself emits whole SDP payloads via signalOut and does not chunk. They are not on the NetplaySession/RtcTransport call path. (signal_chunker.hpp/.cpp)

## Architecture

---

```mermaid
classDiagram
direction TB

class NetplaySession {
  +hostLobby(cb)
  +joinLobby(code, cb)
  +startGame(streamConfig)
  +broadcastPacket(channel, data)
  +setEvents(SessionEvents)
  -m_isHost bool
  -m_phase GamePhase
}
class ILobbyBackend {
  <<interface>>
  +createLobby(code, cb)
  +joinLobby(code, cb)
  +sendSignal(to, payload)
  +currentLobby() LobbyInfo
}
class IPeerTransport {
  <<interface>>
  +connectToPeer(id, isOfferer)
  +handleSignal(from, payload)
  +closePeer(id)
}
class IPeerLink {
  <<interface>>
  +send(channel, data)
  +bufferedBytes(channel) size_t
  +roundTripMs() int
}
class RtcTransport {
  +connectToPeer(id, isOfferer)
  +handleSignal(from, payload)
  -m_impl unique_ptr~Impl~
}
class SessionEvents {
  +phaseChanged(GamePhase)
  +peerReady(id, IPeerLink)
  +packetReceived(id, ChannelKind, bytes)
}
class SlotTable {
  +assign(i, SlotAssignment) bool
  +setReady(memberId, ready)
  +all() array
}
class SlotAssignment {
  +memberId PlayerId
  +localPadIndex int
  +ready bool
}
class ChatLog {
  +append(ChatMessage)
  +entries() deque
}
class SessionDescriptor {
  +gameName string
  +contentHash string
  +strategy SyncStrategyKind
}
class StreamConfig {
  +codec string
  +width int
  +height int
}
class ControlMessage {
  <<variant>>
  Hello Welcome Reject
  SlotTableMessage GameSelected
  ReadyState SessionStarting
  StreamConfig StreamStart
  PauseState SessionEnded
  Ping Pong
}
class ChannelKind {
  <<enumeration>>
  Control
  Video
  Audio
  Input
}
class GamePhase {
  <<enumeration>>
  Idle
  Starting
  InGame
}

IPeerTransport <|-- RtcTransport
NetplaySession --> ILobbyBackend : uses (ref)
NetplaySession --> IPeerTransport : uses (ref)
NetplaySession --> IPeerLink : ptr per peer
NetplaySession *-- SlotTable : owns
NetplaySession *-- ChatLog : owns
NetplaySession *-- SessionEvents : owns
NetplaySession o-- SessionDescriptor : selected game
NetplaySession o-- StreamConfig : announced
NetplaySession ..> ControlMessage : encode/decode
RtcTransport ..> IPeerLink : creates (Impl::Link)
SlotTable *-- SlotAssignment : P1..P8
IPeerLink --> ChannelKind : lanes
NetplaySession --> GamePhase : tracks

%% Omitted for readability: LobbyEvents/TransportEvents (callback structs held by the interfaces), LobbyInfo/LobbyMember/PlayerIdentity, Welcome/Hello/Reject internals, stream_packets (Video/Audio/InputPacket + encode/decode), SignalChunker/SignalReassembler, base64/generateJoinCode, RtcTransportConfig, SyncStrategyKind/SignInState enums, and RtcTransport::Impl/Impl::Peer internals.
```

Entrypoint NetplaySession confirmed correct: it is the orchestrator constructed with ILobbyBackend& + IPeerTransport& (session.hpp:47) and owns SlotTable/ChatLog/SessionEvents by value, holds optional SessionDescriptor/StreamConfig, and keeps a per-peer IPeerLink* map. All claimed relationships verified against headers: RtcTransport final:IPeerTransport (rtc_transport.hpp:22); inner Impl::Link final:IPeerLink creates the link (rtc_transport.cpp:41); SlotTable holds array<optional<SlotAssignment>,MAX_SLOTS> (slot_table.hpp:47); ControlMessage is a 13-alternative std::variant with encode/decode + std::visit dispatch (messages.hpp:87, session.cpp:531). No wrong-kind or wrong-direction relationships found. No KEY type is missing from the render given the 8-14 type guideline; genuinely omitted types (event structs, lobby DTOs, stream_packets, signal chunker, base64, enums) are all captured in the trailing %% comment. composition vs aggregation split is sound: mandatory value members (SlotTable/ChatLog/SessionEvents) as *--, optional value members (SessionDescriptor/StreamConfig) as o--.

## Data Structures

---

### NetplaySession _(class)_
The entrypoint and orchestrator. Owns lobby+game state (isHost, phase, slots, chat, selected game, stream config, peer map) and drives the host-authoritative Hello/Welcome protocol. Wires ILobbyBackend and IPeerTransport callbacks together, routes control JSON, and forwards opaque Video/Audio/Input packets to the app via SessionEvents.

### ILobbyBackend _(interface)_
The single seam for lobbies, chat, connection signaling AND identity/sign-in. Implementations (Discord today; LAN or custom server later) keep their tokens and sign-in flows internal. Fires LobbyEvents (member join/leave/rename, remote chat, inbound signal). Note: chatReceived fires only for REMOTE senders.

### IPeerTransport _(interface)_
The peer-to-peer data plane. Handshakes ride the backend's signaling channel: it emits payloads via a signalOut callback and receives inbound payloads via handleSignal. Fires TransportEvents (peerConnected/peerDisconnected/messageReceived).

### IPeerLink _(interface)_
One established peer connection. Control/Video are reliable+ordered, Audio/Input unreliable. bufferedBytes is the backpressure probe. Non-owning pointers to these are stored in the session's peer map; the transport owns their lifetime.

### RtcTransport _(class)_
The only IPeerTransport implementation: WebRTC data channels via libdatachannel, one PeerConnection per member with four channels mapped from ChannelKind. Non-trickle signaling (one offer/answer once gathering completes). Pimpl; internal Impl owns a worker thread + peers map, Impl::Peer holds the rtc::PeerConnection and channels, Impl::Link is the IPeerLink.

### SessionEvents _(struct)_
Everything the session reports outward as std::function callbacks: lobbyChanged, slotsChanged, phaseChanged, chatMessageAdded, peerReady/peerLost, streamConfigReceived, and the hot-path packetReceived. Set once before hosting/joining.

### LobbyEvents _(struct)_
Callbacks an ILobbyBackend fires as the lobby changes (memberJoined/Left/Renamed, chatReceived for remotes only, signalReceived, lobbyClosed). Held by the base interface, set by the session in wireBackendEvents.

### TransportEvents _(struct)_
Callbacks an IPeerTransport fires (peerConnected, peerDisconnected, messageReceived) with a documented ordering contract. Held by the base interface, set by the session in wireTransportEvents.

### SlotTable _(class)_
Fixed array of MAX_SLOTS(8) optional SlotAssignments (P1-P8). Ready is a per-member flag applied to every slot a member holds. Public accessor is named all() (not slots()) because Qt defines `slots` as a macro and this header is included from Qt code.

### SlotAssignment _(struct)_
One player slot: which member owns it, which of that member's local controllers (localPadIndex) drives it, display name, and ready flag. One member can hold several slots.

### ChatLog _(class)_
Bounded (default 500) deque of ChatMessage; oldest dropped on overflow. Owned by the session.

### SessionDescriptor _(struct)_
The game the lobby selected. contentHash/platformId identify content (lockstep verifies later); gameName/artUrl are for display so guests without the content still see what's playing. Carries a SyncStrategyKind.

### StreamConfig _(struct)_
Everything a guest's decoder needs to start mid-stream (codec, extradataB64, width/height/fps, audio codec/sampleRate/channels, inputTickHz). Announced once encoder params are known; late joiners get it via Welcome. Also a ControlMessage variant.

### ControlMessage _(typedef)_
std::variant of all control-channel messages (JSON on the wire): Hello, Welcome, Reject, SlotTableMessage, GameSelected, ReadyState, SessionStarting, StreamConfig, StreamStart, PauseState, SessionEnded, Ping, Pong. encodeMessage/decodeMessage convert to/from JSON text.

### Welcome _(struct)_
Full state snapshot the host sends whenever a member's control channel opens — fresh join, mid-game join, and reconnect all recover from this single message. Contains phase, SlotTable, optional game, optional StreamConfig.

### VideoPacket / AudioPacket / InputPacket _(struct)_
Binary little-endian packets for the Video/Audio/Input channels (opaque bytes at the session boundary). InputPacket frames are newest-first with a few older duplicates for loss tolerance; host applies the highest unseen seq. displayedPtsMs is reserved for host-side rollback (unused in v1).

### SignalReassembler / SignalChunk _(class)_
Utility to fit SDP signaling under a lobby provider's per-message size cap: base64-encode, split into indexed chunks, and reassemble by (sender, signalId). Standalone helper for backend implementations; not referenced by NetplaySession or RtcTransport directly (RtcTransport emits whole SDP payloads via signalOut).

### GamePhase _(enum)_
Sub-state of the game running inside a lobby: Idle, Starting, InGame. The lobby itself has no phases — it lives from creation until the host closes it; games start/end within it and endGame returns to Idle intact.

### ChannelKind _(enum)_
The four data-channel lanes (Control=0, Video=1, Audio=2, Input=3). Control/Video reliable+ordered; Audio/Input unreliable+unordered. The numeric values index RtcTransport's channel arrays and CHANNEL_LABELS.

### SyncStrategyKind _(enum)_
How gameplay stays in sync: HostStream (host runs the game and streams AV, guests send inputs — the v1 path) or Lockstep (every machine runs the content and exchanges per-frame inputs — later).

### protocol / base64 / generateJoinCode _(free-functions)_
protocol.hpp: PlayerId=uint64_t (opaque, never interpreted), PROTOCOL_VERSION=1, MAX_SLOTS=8, reject codes, and generateJoinCode() (FL-XXXX-XXXX, Crockford base32 minus I/L/O/U for read-aloud safety). base64.hpp: header-only encode/decode used by the signal chunker.
