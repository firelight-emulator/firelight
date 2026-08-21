#include <firelight/netplay/rtc_transport.hpp>

#include <chrono>
#include <condition_variable>
#include <gtest/gtest.h>
#include <mutex>

namespace firelight::netplay {

namespace {
// Real WebRTC over loopback: two transports whose signaling is wired directly
struct Loopback {
  Loopback() {
    RtcTransportConfig config;
    config.stunServer = ""; // host candidates only, no network dependency
    first = std::make_unique<RtcTransport>(config);
    second = std::make_unique<RtcTransport>(config);

    first->setSignalOut([this](PlayerId, const std::string &payload) { second->handleSignal(1, payload); });
    second->setSignalOut([this](PlayerId, const std::string &payload) { first->handleSignal(2, payload); });
  }

  std::unique_ptr<RtcTransport> first;
  std::unique_ptr<RtcTransport> second;
};

template <typename Predicate>
bool waitFor(std::mutex &mutex, std::condition_variable &cv, Predicate predicate, const int seconds = 15) {
  std::unique_lock lock(mutex);
  return cv.wait_for(lock, std::chrono::seconds(seconds), predicate);
}
} // namespace

// Disabled by default: two WebRTC endpoints in ONE process share usrsctp's
// process-global state, and its synchronous loopback delivery can re-enter
// libdatachannel on the same thread (read->write lock upgrade -> EDEADLK
// abort, roughly 1 run in 3). Real deployments always have one endpoint per
// process, so this topology exists only here. Run it on demand with
// --gtest_also_run_disabled_tests; the definitive check is the two-machine
// manual smoke
TEST(RtcTransportTest, DISABLED_LoopbackConnectExchangeAndClose) {
  Loopback loop;

  std::mutex mutex;
  std::condition_variable cv;
  IPeerLink *firstLink = nullptr;
  IPeerLink *secondLink = nullptr;
  int disconnects = 0;
  std::map<ChannelKind, std::vector<uint8_t>> firstReceived;
  std::map<ChannelKind, std::vector<uint8_t>> secondReceived;

  TransportEvents firstEvents;
  firstEvents.peerConnected = [&](PlayerId, IPeerLink &link) {
    std::lock_guard lock(mutex);
    firstLink = &link;
    cv.notify_all();
  };
  firstEvents.peerDisconnected = [&](PlayerId) {
    std::lock_guard lock(mutex);
    disconnects++;
    cv.notify_all();
  };
  firstEvents.messageReceived = [&](PlayerId, const ChannelKind channel, std::span<const uint8_t> data) {
    std::lock_guard lock(mutex);
    firstReceived[channel].assign(data.begin(), data.end());
    cv.notify_all();
  };
  loop.first->setEvents(std::move(firstEvents));

  TransportEvents secondEvents;
  secondEvents.peerConnected = [&](PlayerId, IPeerLink &link) {
    std::lock_guard lock(mutex);
    secondLink = &link;
    cv.notify_all();
  };
  secondEvents.messageReceived = [&](PlayerId, const ChannelKind channel, std::span<const uint8_t> data) {
    std::lock_guard lock(mutex);
    secondReceived[channel].assign(data.begin(), data.end());
    cv.notify_all();
  };
  loop.second->setEvents(std::move(secondEvents));

  // Transport 1 offers toward member 2
  loop.first->connectToPeer(2, true);

  ASSERT_TRUE(waitFor(mutex, cv, [&] { return firstLink && secondLink; })) << "peers did not connect";

  // Exchange one payload per channel in each direction
  for (const auto kind : {ChannelKind::Control, ChannelKind::Video, ChannelKind::Audio, ChannelKind::Input}) {
    const std::vector<uint8_t> toSecond{static_cast<uint8_t>(kind), 1, 2, 3};
    const std::vector<uint8_t> toFirst{static_cast<uint8_t>(kind), 9, 8, 7};
    firstLink->send(kind, toSecond);
    secondLink->send(kind, toFirst);
  }

  ASSERT_TRUE(waitFor(mutex, cv, [&] { return firstReceived.size() == 4 && secondReceived.size() == 4; }))
      << "messages did not arrive on all channels";

  EXPECT_EQ(secondReceived[ChannelKind::Video],
            (std::vector<uint8_t>{static_cast<uint8_t>(ChannelKind::Video), 1, 2, 3}));
  EXPECT_EQ(firstReceived[ChannelKind::Input],
            (std::vector<uint8_t>{static_cast<uint8_t>(ChannelKind::Input), 9, 8, 7}));
  EXPECT_GE(firstLink->roundTripMs(), -1);

  // Closing one end reports the drop on the other
  loop.second->closeAll();
  EXPECT_TRUE(waitFor(mutex, cv, [&] { return disconnects >= 1; })) << "disconnect was not reported";
}

} // namespace firelight::netplay
