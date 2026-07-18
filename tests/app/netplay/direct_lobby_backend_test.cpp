#include "../../../src/app/netplay/direct_lobby_backend.hpp"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <gtest/gtest.h>

namespace firelight::netplay {

namespace {
// Pumps the Qt event loop until the condition holds or the timeout passes
template <typename Predicate>
bool pumpUntil(Predicate predicate, const int timeoutMs = 5000) {
  QElapsedTimer timer;
  timer.start();
  while (!predicate()) {
    if (timer.elapsed() > timeoutMs) {
      return false;
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
  }
  return true;
}

struct RecordedEvents {
  std::vector<LobbyMember> joined;
  std::vector<PlayerId> left;
  std::vector<std::pair<std::string, std::string>> chat; // name, text
  std::vector<std::pair<PlayerId, std::string>> signals_;
  int lobbyClosed = 0;

  LobbyEvents wire() {
    LobbyEvents events;
    events.memberJoined = [this](const LobbyMember &m) {
      joined.push_back(m);
    };
    events.memberLeft = [this](const PlayerId id) { left.push_back(id); };
    events.chatReceived = [this](PlayerId, const std::string &name,
                                 const std::string &text) {
      chat.emplace_back(name, text);
    };
    events.signalReceived = [this](const PlayerId from,
                                   const std::string &payload) {
      signals_.emplace_back(from, payload);
    };
    events.lobbyClosed = [this] { lobbyClosed++; };
    return events;
  }
};
} // namespace

TEST(DirectLobbyBackendTest, HostGuestJoinChatSignalLeave) {
  DirectLobbyBackend host;
  host.setPreferredDisplayName("Alice");
  RecordedEvents hostEvents;
  host.setEvents(hostEvents.wire());

  bool hostOk = false;
  host.createLobby("ignored", [&](const bool ok, const std::string &) {
    hostOk = ok;
  });
  ASSERT_TRUE(hostOk);
  EXPECT_TRUE(host.currentLobby().joined);
  EXPECT_EQ(host.currentLobby().hostId, 1u);
  EXPECT_NE(host.currentLobby().joinCode.find(":47774"), std::string::npos);

  DirectLobbyBackend guest;
  guest.setPreferredDisplayName("Bob");
  RecordedEvents guestEvents;
  guest.setEvents(guestEvents.wire());

  bool guestDone = false;
  bool guestOk = false;
  guest.joinLobby("127.0.0.1", [&](const bool ok, const std::string &) {
    guestDone = true;
    guestOk = ok;
  });
  ASSERT_TRUE(pumpUntil([&] { return guestDone; }));
  ASSERT_TRUE(guestOk);
  ASSERT_TRUE(pumpUntil([&] { return !hostEvents.joined.empty(); }));

  const auto guestId = guest.currentLobby().self.id;
  EXPECT_GT(guestId, 1u);
  EXPECT_EQ(guest.currentLobby().hostId, 1u);
  EXPECT_EQ(guest.currentLobby().members.size(), 2u);
  EXPECT_EQ(host.currentLobby().members.size(), 2u);
  EXPECT_EQ(hostEvents.joined[0].id, guestId);
  EXPECT_EQ(hostEvents.joined[0].displayName, "Bob");
  EXPECT_EQ(guest.currentLobby().members[0].displayName, "Alice");
  EXPECT_EQ(host.localIdentity().displayName, "Alice");

  guest.sendChat("hi from guest");
  ASSERT_TRUE(pumpUntil([&] { return !hostEvents.chat.empty(); }));
  EXPECT_EQ(hostEvents.chat[0].second, "hi from guest");

  host.sendChat("hi from host");
  ASSERT_TRUE(pumpUntil([&] { return !guestEvents.chat.empty(); }));
  EXPECT_EQ(guestEvents.chat[0].second, "hi from host");

  guest.sendSignal(1, "offer-payload");
  ASSERT_TRUE(pumpUntil([&] { return !hostEvents.signals_.empty(); }));
  EXPECT_EQ(hostEvents.signals_[0].first, guestId);
  EXPECT_EQ(hostEvents.signals_[0].second, "offer-payload");

  host.sendSignal(guestId, "answer-payload");
  ASSERT_TRUE(pumpUntil([&] { return !guestEvents.signals_.empty(); }));
  EXPECT_EQ(guestEvents.signals_[0].first, 1u);
  EXPECT_EQ(guestEvents.signals_[0].second, "answer-payload");

  guest.leaveLobby();
  ASSERT_TRUE(pumpUntil([&] { return !hostEvents.left.empty(); }));
  EXPECT_EQ(hostEvents.left[0], guestId);
  EXPECT_EQ(host.currentLobby().members.size(), 1u);

  host.leaveLobby();
  pumpUntil([&] { return !host.currentLobby().joined; });
}

TEST(DirectLobbyBackendTest, HostClosingEndsLobbyForGuests) {
  DirectLobbyBackend host;
  RecordedEvents hostEvents;
  host.setEvents(hostEvents.wire());
  host.createLobby("ignored", nullptr);

  DirectLobbyBackend guest;
  RecordedEvents guestEvents;
  guest.setEvents(guestEvents.wire());

  bool joined = false;
  guest.joinLobby("127.0.0.1:47774",
                  [&](const bool ok, const std::string &) { joined = ok; });
  ASSERT_TRUE(pumpUntil([&] { return joined; }));

  host.leaveLobby();
  ASSERT_TRUE(pumpUntil([&] { return guestEvents.lobbyClosed > 0; }));
  EXPECT_FALSE(guest.currentLobby().joined);
}

TEST(DirectLobbyBackendTest, JoinFailsWhenNobodyIsListening) {
  DirectLobbyBackend guest;
  bool done = false;
  bool ok = true;
  std::string error;
  guest.joinLobby("127.0.0.1:47799",
                  [&](const bool result, const std::string &message) {
                    done = true;
                    ok = result;
                    error = message;
                  });
  ASSERT_TRUE(pumpUntil([&] { return done; }, 15000));
  EXPECT_FALSE(ok);
  EXPECT_FALSE(error.empty());
  EXPECT_FALSE(guest.currentLobby().joined);
}

} // namespace firelight::netplay
