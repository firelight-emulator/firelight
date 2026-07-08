#include "fake_lobby_backend.hpp"
#include "fake_transport.hpp"

#include <firelight/netplay/protocol.hpp>
#include <firelight/netplay/session.hpp>

#include <gtest/gtest.h>

namespace firelight::netplay {

namespace {
// One simulated machine: identity + backend + transport + session.
struct Client {
  Client(const std::shared_ptr<FakeLobbyHub> &lobbyHub,
         const std::shared_ptr<FakeTransportHub> &transportHub,
         const PlayerId id, const std::string &name)
      : backend(lobbyHub, PlayerIdentity{id, name}),
        transport(transportHub, id), session(backend, transport, "0.1.0") {}

  FakeLobbyBackend backend;
  FakeTransport transport;
  NetplaySession session;
};

struct Fixture : testing::Test {
  std::shared_ptr<FakeLobbyHub> lobbyHub = std::make_shared<FakeLobbyHub>();
  std::shared_ptr<FakeTransportHub> transportHub =
      std::make_shared<FakeTransportHub>();

  Client host{lobbyHub, transportHub, 1, "Host"};
  Client guest{lobbyHub, transportHub, 2, "GuestOne"};
  Client guest2{lobbyHub, transportHub, 3, "GuestTwo"};

  void hostUp() {
    bool ok = false;
    host.session.hostLobby([&](const bool result, const std::string &) {
      ok = result;
    });
    ASSERT_TRUE(ok);
  }

  void join(Client &client) {
    bool ok = false;
    client.session.joinLobby(host.session.joinCode(),
                             [&](const bool result, const std::string &) {
                               ok = result;
                             });
    ASSERT_TRUE(ok);
  }
};

using SessionTest = Fixture;
} // namespace

TEST_F(SessionTest, GuestJoinsAndIsAdmitted) {
  hostUp();
  join(guest);

  EXPECT_TRUE(host.session.isHost());
  EXPECT_FALSE(guest.session.isHost());
  EXPECT_EQ(host.session.connectedPeers(), std::vector<PlayerId>{2});
  EXPECT_EQ(guest.session.connectedPeers(), std::vector<PlayerId>{1});
  EXPECT_EQ(guest.session.phase(), GamePhase::Idle);
  EXPECT_EQ(guest.session.lobby().members.size(), 2u);
}

TEST_F(SessionTest, SlotAssignmentAndReadyPropagate) {
  hostUp();
  join(guest);

  host.session.assignSlot(0, 1, 0);
  host.session.assignSlot(1, 2, 0);

  auto guestSlots = guest.session.slotTable();
  ASSERT_TRUE(guestSlots.slot(0).has_value());
  EXPECT_EQ(guestSlots.slot(0)->displayName, "Host");
  ASSERT_TRUE(guestSlots.slot(1).has_value());
  EXPECT_EQ(guestSlots.slot(1)->displayName, "GuestOne");
  EXPECT_FALSE(guestSlots.slot(1)->ready);

  guest.session.setReady(true);
  EXPECT_TRUE(host.session.slotTable().slot(1)->ready);
  EXPECT_TRUE(guest.session.slotTable().slot(1)->ready);

  host.session.clearSlot(1);
  EXPECT_FALSE(guest.session.slotTable().slot(1).has_value());
}

TEST_F(SessionTest, GameLifecycleReachesGuests) {
  hostUp();
  join(guest);

  std::vector<GamePhase> guestPhases;
  std::optional<StreamConfig> guestConfig;
  SessionEvents events;
  events.phaseChanged = [&](const GamePhase phase) {
    guestPhases.push_back(phase);
  };
  events.streamConfigReceived = [&](const StreamConfig &config) {
    guestConfig = config;
  };
  guest.session.setEvents(std::move(events));

  host.session.selectGame({.gameName = "Mario Kart",
                           .contentHash = "abc",
                           .platformId = 4});
  ASSERT_TRUE(guest.session.game().has_value());
  EXPECT_EQ(guest.session.game()->gameName, "Mario Kart");

  host.session.startGame(std::nullopt);
  host.session.announceStreamConfig(StreamConfig{.width = 240, .height = 160});
  host.session.markStreamStarted();

  EXPECT_EQ(guest.session.phase(), GamePhase::InGame);
  ASSERT_TRUE(guestConfig.has_value());
  EXPECT_EQ(guestConfig->width, 240);
  EXPECT_EQ(guestPhases,
            (std::vector{GamePhase::Starting, GamePhase::InGame}));

  host.session.setPaused(true);
  EXPECT_TRUE(guest.session.isPaused());

  host.session.endGame("finished");
  EXPECT_EQ(guest.session.phase(), GamePhase::Idle);
  EXPECT_FALSE(guest.session.isPaused());
  EXPECT_TRUE(guest.session.inLobby());
  EXPECT_FALSE(guest.session.streamConfig().has_value());
}

TEST_F(SessionTest, JoinMidGameGetsFullSnapshot) {
  hostUp();
  join(guest);
  host.session.assignSlot(0, 1, 0);
  host.session.assignSlot(1, 2, 0);
  host.session.selectGame({.gameName = "Tetris", .contentHash = "t"});
  host.session.startGame(StreamConfig{.width = 320, .height = 240});
  host.session.markStreamStarted();

  join(guest2);

  EXPECT_EQ(guest2.session.phase(), GamePhase::InGame);
  ASSERT_TRUE(guest2.session.game().has_value());
  EXPECT_EQ(guest2.session.game()->gameName, "Tetris");
  ASSERT_TRUE(guest2.session.streamConfig().has_value());
  EXPECT_EQ(guest2.session.streamConfig()->width, 320);
  EXPECT_EQ(guest2.session.slotTable().occupiedCount(), 2);

  // Host can seat the newcomer mid-game.
  host.session.assignSlot(2, 3, 0);
  EXPECT_EQ(guest2.session.slotTable().slot(2)->displayName, "GuestTwo");
  EXPECT_EQ(guest.session.slotTable().slot(2)->displayName, "GuestTwo");
}

TEST_F(SessionTest, BackToBackGamesKeepLobbyIntact) {
  hostUp();
  join(guest);
  host.session.assignSlot(0, 1, 0);
  host.session.assignSlot(1, 2, 0);
  guest.session.setReady(true);

  host.session.startGame(StreamConfig{.width = 240});
  host.session.markStreamStarted();
  host.session.endGame("finished");

  // Ready flags reset for the next game; membership and slots survive.
  EXPECT_FALSE(host.session.slotTable().slot(1)->ready);
  EXPECT_EQ(guest.session.slotTable().occupiedCount(), 2);
  EXPECT_TRUE(guest.session.inLobby());

  host.session.selectGame({.gameName = "Mario Party", .contentHash = "mp"});
  guest.session.setReady(true);
  host.session.startGame(StreamConfig{.width = 640});
  host.session.markStreamStarted();

  EXPECT_EQ(guest.session.phase(), GamePhase::InGame);
  EXPECT_EQ(guest.session.game()->gameName, "Mario Party");
  EXPECT_EQ(guest.session.streamConfig()->width, 640);
}

TEST_F(SessionTest, ProtocolMismatchIsRejected) {
  hostUp();
  join(guest);

  // A raw client that speaks an old protocol version: uses the transport
  // directly and hand-crafts its Hello.
  FakeTransport rawTransport(transportHub, 99);
  std::string rejectCode;
  IPeerLink *rawLink = nullptr;
  TransportEvents rawEvents;
  rawEvents.peerConnected = [&](PlayerId, IPeerLink &link) {
    rawLink = &link;
  };
  rawEvents.messageReceived = [&](PlayerId, ChannelKind,
                                  std::span<const uint8_t> data) {
    const auto decoded =
        decodeMessage(std::string(data.begin(), data.end()));
    if (decoded) {
      if (const auto *reject = std::get_if<Reject>(&*decoded)) {
        rejectCode = reject->code;
      }
    }
  };
  rawTransport.setEvents(std::move(rawEvents));
  rawTransport.setSignalOut([&](const PlayerId to, const std::string &payload) {
    // Signaling normally rides the lobby; deliver directly for the raw peer.
    if (auto *hostTransport = &host.transport) {
      hostTransport->handleSignal(99, payload);
    }
    (void)to;
  });
  // The host's answer signal has nowhere to go (99 isn't a lobby member), so
  // complete the raw side's handshake by hand.
  rawTransport.connectToPeer(1, true);
  rawTransport.handleSignal(1, "answer");
  ASSERT_NE(rawLink, nullptr);

  const auto hello =
      encodeMessage(Hello{.proto = 999, .appVersion = "9", .memberId = 99});
  rawLink->send(ChannelKind::Control,
                std::span(reinterpret_cast<const uint8_t *>(hello.data()),
                          hello.size()));

  EXPECT_EQ(rejectCode, REJECT_PROTOCOL_MISMATCH);
  const auto peers = host.session.connectedPeers();
  EXPECT_EQ(std::count(peers.begin(), peers.end(), 99), 0);
}

TEST_F(SessionTest, ReconnectReplaysSnapshot) {
  hostUp();
  join(guest);
  host.session.assignSlot(1, 2, 0);
  host.session.selectGame({.gameName = "Zelda", .contentHash = "z"});
  host.session.startGame(StreamConfig{.width = 256});
  host.session.markStreamStarted();

  // Network blip: both ends lose the link, nobody leaves the lobby.
  guest.transport.dropLink(1);
  EXPECT_TRUE(guest.session.connectedPeers().empty());
  EXPECT_TRUE(host.session.connectedPeers().empty());
  EXPECT_TRUE(guest.session.inLobby());

  guest.session.reconnectToHost();

  EXPECT_EQ(guest.session.connectedPeers(), std::vector<PlayerId>{1});
  EXPECT_EQ(guest.session.phase(), GamePhase::InGame);
  EXPECT_EQ(guest.session.streamConfig()->width, 256);
  // The slot survived the drop, so the guest can keep playing.
  EXPECT_EQ(host.session.slotTable().slot(1)->displayName, "GuestOne");
}

TEST_F(SessionTest, MemberLeavingClearsTheirSlots) {
  hostUp();
  join(guest);
  join(guest2);
  host.session.assignSlot(1, 2, 0);
  host.session.assignSlot(2, 3, 0);

  guest.session.leaveLobby();

  EXPECT_FALSE(host.session.slotTable().slot(1).has_value());
  EXPECT_TRUE(host.session.slotTable().slot(2).has_value());
  EXPECT_FALSE(guest2.session.slotTable().slot(1).has_value());
  EXPECT_EQ(host.session.lobby().members.size(), 2u);
}

TEST_F(SessionTest, HostLeavingEndsLobbyForEveryone) {
  hostUp();
  join(guest);
  join(guest2);

  std::string endReason;
  SessionEvents events;
  events.lobbyEnded = [&](const std::string &reason) { endReason = reason; };
  guest.session.setEvents(std::move(events));

  host.session.leaveLobby();

  EXPECT_EQ(endReason, "lobby-closed");
  EXPECT_FALSE(guest.session.inLobby());
  EXPECT_FALSE(guest2.session.inLobby());
  EXPECT_FALSE(host.session.inLobby());
}

TEST_F(SessionTest, ChatReachesEveryoneIncludingSender) {
  hostUp();
  join(guest);
  join(guest2);

  host.session.sendChat("hello lobby");
  guest.session.sendChat("hi host");

  for (auto *client : {&host, &guest, &guest2}) {
    const auto messages = client->session.chatMessages();
    ASSERT_EQ(messages.size(), 2u);
    EXPECT_EQ(messages[0].senderName, "Host");
    EXPECT_EQ(messages[0].text, "hello lobby");
    EXPECT_EQ(messages[1].senderName, "GuestOne");
    EXPECT_EQ(messages[1].text, "hi host");
  }
}

TEST_F(SessionTest, PacketsBypassControlAndReachSink) {
  hostUp();
  join(guest);

  std::vector<uint8_t> received;
  ChannelKind receivedChannel{};
  SessionEvents events;
  events.packetReceived = [&](PlayerId, const ChannelKind channel,
                              std::span<const uint8_t> data) {
    receivedChannel = channel;
    received.assign(data.begin(), data.end());
  };
  host.session.setEvents(std::move(events));

  const std::vector<uint8_t> inputBytes{1, 2, 3, 4};
  guest.session.sendToHost(ChannelKind::Input, inputBytes);

  EXPECT_EQ(receivedChannel, ChannelKind::Input);
  EXPECT_EQ(received, inputBytes);

  // And host -> guests on the stream channels.
  std::vector<uint8_t> guestReceived;
  SessionEvents guestEvents;
  guestEvents.packetReceived = [&](PlayerId, ChannelKind,
                                   std::span<const uint8_t> data) {
    guestReceived.assign(data.begin(), data.end());
  };
  guest.session.setEvents(std::move(guestEvents));

  const std::vector<uint8_t> videoBytes{9, 9, 9};
  host.session.broadcastPacket(ChannelKind::Video, videoBytes);
  EXPECT_EQ(guestReceived, videoBytes);
}

} // namespace firelight::netplay
