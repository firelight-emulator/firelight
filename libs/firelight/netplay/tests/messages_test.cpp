#include <firelight/netplay/messages.hpp>
#include <firelight/netplay/protocol.hpp>

#include <gtest/gtest.h>

namespace firelight::netplay {

TEST(MessagesTest, HelloRoundTrips) {
  const Hello hello{.proto = PROTOCOL_VERSION,
                    .appVersion = "1.2.3",
                    .memberId = 42};
  const auto decoded = decodeMessage(encodeMessage(hello));
  ASSERT_TRUE(decoded.has_value());
  const auto *out = std::get_if<Hello>(&*decoded);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->proto, PROTOCOL_VERSION);
  EXPECT_EQ(out->appVersion, "1.2.3");
  EXPECT_EQ(out->memberId, 42u);
}

TEST(MessagesTest, WelcomeCarriesFullSnapshot) {
  Welcome welcome;
  welcome.proto = PROTOCOL_VERSION;
  welcome.phase = GamePhase::InGame;
  welcome.table.assign(0, SlotAssignment{.memberId = 1,
                                         .localPadIndex = 0,
                                         .displayName = "Host",
                                         .ready = true});
  welcome.table.assign(3, SlotAssignment{.memberId = 2,
                                         .localPadIndex = 1,
                                         .displayName = "Guest"});
  welcome.game = SessionDescriptor{.gameName = "Mario Kart",
                                   .contentHash = "abc123",
                                   .platformId = 4,
                                   .artUrl = "http://art",
                                   .strategy = SyncStrategyKind::HostStream};
  welcome.streamConfig = StreamConfig{.extradataB64 = "c3BzcHBz",
                                      .width = 240,
                                      .height = 160,
                                      .fps = 60};

  const auto decoded = decodeMessage(encodeMessage(welcome));
  ASSERT_TRUE(decoded.has_value());
  const auto *out = std::get_if<Welcome>(&*decoded);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->phase, GamePhase::InGame);
  ASSERT_TRUE(out->table.slot(0).has_value());
  EXPECT_EQ(out->table.slot(0)->displayName, "Host");
  EXPECT_TRUE(out->table.slot(0)->ready);
  ASSERT_TRUE(out->table.slot(3).has_value());
  EXPECT_EQ(out->table.slot(3)->localPadIndex, 1);
  EXPECT_FALSE(out->table.slot(1).has_value());
  ASSERT_TRUE(out->game.has_value());
  EXPECT_EQ(out->game->contentHash, "abc123");
  ASSERT_TRUE(out->streamConfig.has_value());
  EXPECT_EQ(out->streamConfig->width, 240);
}

TEST(MessagesTest, AllSimpleTypesRoundTrip) {
  EXPECT_TRUE(std::holds_alternative<Reject>(
      *decodeMessage(encodeMessage(Reject{.code = "x", .message = "y"}))));
  EXPECT_TRUE(std::holds_alternative<ReadyState>(
      *decodeMessage(encodeMessage(ReadyState{.ready = true}))));
  EXPECT_TRUE(std::holds_alternative<SessionStarting>(
      *decodeMessage(encodeMessage(SessionStarting{}))));
  EXPECT_TRUE(std::holds_alternative<StreamStart>(
      *decodeMessage(encodeMessage(StreamStart{}))));
  EXPECT_TRUE(std::holds_alternative<PauseState>(
      *decodeMessage(encodeMessage(PauseState{.paused = true}))));
  EXPECT_TRUE(std::holds_alternative<SessionEnded>(
      *decodeMessage(encodeMessage(SessionEnded{.reason = "done"}))));
  EXPECT_TRUE(std::holds_alternative<Ping>(
      *decodeMessage(encodeMessage(Ping{.t = 123}))));
  EXPECT_TRUE(std::holds_alternative<Pong>(
      *decodeMessage(encodeMessage(Pong{.t = 123}))));
  EXPECT_TRUE(std::holds_alternative<GameSelected>(*decodeMessage(
      encodeMessage(GameSelected{.game = {.gameName = "Tetris"}}))));
  EXPECT_TRUE(std::holds_alternative<StreamConfig>(
      *decodeMessage(encodeMessage(StreamConfig{.width = 320}))));
}

TEST(MessagesTest, GarbageIsRejected) {
  EXPECT_FALSE(decodeMessage("not json").has_value());
  EXPECT_FALSE(decodeMessage("{}").has_value());
  EXPECT_FALSE(decodeMessage(R"({"type":"unknown"})").has_value());
  EXPECT_FALSE(decodeMessage("[1,2,3]").has_value());
}

TEST(MessagesTest, JoinCodeShape) {
  const auto code = generateJoinCode();
  EXPECT_EQ(code.size(), 12u);
  EXPECT_TRUE(code.starts_with("FL-"));
  EXPECT_EQ(code[7], '-');
  EXPECT_NE(code, generateJoinCode());
}

} // namespace firelight::netplay
