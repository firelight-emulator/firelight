#include "../../../libs/firelight/netplay/tests/fake_lobby_backend.hpp"
#include "../../../libs/firelight/netplay/tests/fake_transport.hpp"
#include "../../../src/app/netplay/netplay_service.hpp"

#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/netplay/stream_packets.hpp>

#include <filesystem>
#include <gtest/gtest.h>

namespace firelight::netplay {

namespace {
struct Fixture : testing::Test {
  std::shared_ptr<FakeLobbyHub> lobbyHub = std::make_shared<FakeLobbyHub>();
  std::shared_ptr<FakeTransportHub> transportHub =
      std::make_shared<FakeTransportHub>();

  library::SqliteUserLibraryRepository libraryRepo{":memory:"};
  library::UserLibraryService library{
      libraryRepo, std::filesystem::temp_directory_path().string()};

  FakeLobbyBackend hostBackend{lobbyHub, PlayerIdentity{1, "Host"}};
  FakeTransport hostTransport{transportHub, 1};
  NetplayService host{hostBackend, hostTransport, library, "0.1.0"};

  FakeLobbyBackend guestBackend{lobbyHub, PlayerIdentity{2, "Guest"}};
  FakeTransport guestTransport{transportHub, 2};
  NetplayService guest{guestBackend, guestTransport, library, "0.1.0"};

  void hostUpAndJoin() {
    bool ok = false;
    host.hostLobby([&](const bool result, const std::string &) {
      ok = result;
    });
    ASSERT_TRUE(ok);
    ok = false;
    guest.joinLobby(host.session().joinCode(),
                    [&](const bool result, const std::string &) {
                      ok = result;
                    });
    ASSERT_TRUE(ok);
  }
};

using NetplayServiceTest = Fixture;
} // namespace

TEST_F(NetplayServiceTest, MembersAreAutoSeatedIntoSlots) {
  hostUpAndJoin();

  const auto table = host.session().slotTable();
  ASSERT_TRUE(table.slot(0).has_value());
  EXPECT_EQ(table.slot(0)->memberId, 1u);
  ASSERT_TRUE(table.slot(1).has_value());
  EXPECT_EQ(table.slot(1)->memberId, 2u);
  EXPECT_EQ(table.slot(1)->displayName, "Guest");

  // The guest sees the same seating
  EXPECT_EQ(guest.session().slotTable().occupiedCount(), 2);

  // A host-cleared slot stays cleared; auto-seating only covers new arrivals
  host.clearSlot(1);
  host.session().sendChat("nudge"); // any lobbyChanged won't re-seat
  EXPECT_FALSE(host.session().slotTable().slot(1).has_value());
}

TEST_F(NetplayServiceTest, ReadyCheckPhasesFlowToGuests) {
  hostUpAndJoin();

  // No game selected yet -> can't start
  EXPECT_FALSE(host.startGame());

  host.session().selectGame({.gameName = "Mario Kart", .contentHash = "mk"});
  EXPECT_TRUE(host.startGame());
  EXPECT_EQ(guest.session().phase(), GamePhase::Starting);

  // Guests can't start games
  EXPECT_FALSE(guest.startGame());

  guest.setReady(true);
  EXPECT_TRUE(host.session().slotTable().slot(1)->ready);

  host.confirmLaunch();
  EXPECT_EQ(guest.session().phase(), GamePhase::InGame);

  host.endGame();
  EXPECT_EQ(guest.session().phase(), GamePhase::Idle);
  EXPECT_TRUE(guest.session().inLobby());
  // Double-start guard: a second ready check works after the game ended
  EXPECT_TRUE(host.startGame());
}

TEST_F(NetplayServiceTest, RenamesReachSlotsAndMembersEverywhere) {
  hostUpAndJoin();

  guest.setPlayerName("Zelda");
  // The host's slot table carries the new name and broadcasts it
  EXPECT_EQ(host.session().slotTable().slot(1)->displayName, "Zelda");
  EXPECT_EQ(guest.session().slotTable().slot(1)->displayName, "Zelda");

  host.setPlayerName("Ganon");
  EXPECT_EQ(host.session().slotTable().slot(0)->displayName, "Ganon");
  EXPECT_EQ(guest.session().slotTable().slot(0)->displayName, "Ganon");
}

TEST_F(NetplayServiceTest, LateJoinerDuringGameIsSeated) {
  hostUpAndJoin();
  host.session().selectGame({.gameName = "Tetris", .contentHash = "t"});
  host.startGame();
  host.confirmLaunch();

  FakeLobbyBackend lateBackend{lobbyHub, PlayerIdentity{3, "Late"}};
  FakeTransport lateTransport{transportHub, 3};
  NetplayService late{lateBackend, lateTransport, library, "0.1.0"};
  bool ok = false;
  late.joinLobby(host.session().joinCode(),
                 [&](const bool result, const std::string &) { ok = result; });
  ASSERT_TRUE(ok);

  EXPECT_EQ(late.session().phase(), GamePhase::InGame);
  ASSERT_TRUE(host.session().slotTable().slot(2).has_value());
  EXPECT_EQ(host.session().slotTable().slot(2)->displayName, "Late");
}

TEST_F(NetplayServiceTest, GuestInputReachesHostRemotePad) {
  hostUpAndJoin();
  host.session().selectGame({.gameName = "Kart", .contentHash = "k"});
  host.startGame();
  guest.setReady(true);
  host.confirmLaunch();

  // While hosting a live game, port 1 answers with the guest's remote pad
  EXPECT_TRUE(host.retropadProvider().active());
  const auto pad = host.retropadProvider().getRetropadForPlayerIndex(1);
  ASSERT_NE(pad, nullptr);
  EXPECT_FALSE(pad->isButtonPressed(0, 1, libretro::IRetroPad::SouthFace));

  // The guest sends a pressed frame over the input channel
  input::InputFrame frame;
  frame.setButton(0, true); // SouthFace / RETRO_DEVICE_ID_JOYPAD_B
  frame.leftStickX = 12000;
  InputPacket packet;
  packet.seq = 1;
  packet.localPadIndex = 0;
  packet.frames.push_back(frame.serialize());
  guest.session().sendToHost(ChannelKind::Input, encodePacket(packet));

  EXPECT_TRUE(pad->isButtonPressed(0, 1, libretro::IRetroPad::SouthFace));
  EXPECT_EQ(pad->getLeftStickXPosition(0, 1), 12000);

  // Empty slots answer nothing while online; ending the game restores the
  // pass-through provider
  EXPECT_EQ(host.retropadProvider().getRetropadForPlayerIndex(5), nullptr);
  host.endGame();
  EXPECT_FALSE(host.retropadProvider().active());
}

TEST_F(NetplayServiceTest, StreamConfigReachesGuestReceiver) {
  hostUpAndJoin();
  host.session().selectGame({.gameName = "Kart", .contentHash = "k"});
  host.startGame();
  host.confirmLaunch();

  // The host announces a config mid-game (as the encoder would); the guest's
  // receiver takes it without a valid extradata blob being decodable — the
  // decoder rejects garbage extradata gracefully
  host.session().announceStreamConfig(StreamConfig{
      .extradataB64 = "!!!not-base64!!!", .width = 240, .height = 160});
  EXPECT_EQ(guest.session().streamConfig()->width, 240);
}

} // namespace firelight::netplay
