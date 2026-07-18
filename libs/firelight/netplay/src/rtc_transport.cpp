#include <firelight/netplay/rtc_transport.hpp>

#include <nlohmann/json.hpp>
#include <rtc/rtc.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <condition_variable>
#include <deque>
#include <map>
#include <mutex>
#include <thread>

namespace firelight::netplay {

namespace {
constexpr std::array<const char *, 4> CHANNEL_LABELS{"control", "video",
                                                     "audio", "input"};

std::optional<ChannelKind> channelForLabel(const std::string &label) {
  for (size_t i = 0; i < CHANNEL_LABELS.size(); ++i) {
    if (label == CHANNEL_LABELS[i]) {
      return static_cast<ChannelKind>(i);
    }
  }
  return std::nullopt;
}

rtc::DataChannelInit initFor(const ChannelKind kind) {
  rtc::DataChannelInit init;
  if (kind == ChannelKind::Audio || kind == ChannelKind::Input) {
    init.reliability.unordered = true;
    init.reliability.maxRetransmits = 0;
  }
  return init;
}
} // namespace

struct RtcTransport::Impl {
  class Link final : public IPeerLink {
  public:
    Link(Impl &owner, const PlayerId peerId) : m_owner(owner), m_peerId(peerId) {}

    void send(const ChannelKind channel,
              const std::span<const uint8_t> data) override {
      std::shared_ptr<rtc::DataChannel> target;
      {
        std::lock_guard lock(m_owner.mutex);
        const auto it = m_owner.peers.find(m_peerId);
        if (it == m_owner.peers.end()) {
          return;
        }
        target = it->second.channels[static_cast<size_t>(channel)];
      }
      if (target && target->isOpen()) {
        target->send(reinterpret_cast<const std::byte *>(data.data()),
                     data.size());
      }
    }

    [[nodiscard]] size_t bufferedBytes(const ChannelKind channel) const override {
      std::lock_guard lock(m_owner.mutex);
      const auto it = m_owner.peers.find(m_peerId);
      if (it == m_owner.peers.end()) {
        return 0;
      }
      const auto &target = it->second.channels[static_cast<size_t>(channel)];
      return target ? target->bufferedAmount() : 0;
    }

    [[nodiscard]] int roundTripMs() const override {
      std::lock_guard lock(m_owner.mutex);
      const auto it = m_owner.peers.find(m_peerId);
      if (it == m_owner.peers.end() || !it->second.connection) {
        return -1;
      }
      const auto rtt = it->second.connection->rtt();
      return rtt ? static_cast<int>(rtt->count()) : -1;
    }

  private:
    Impl &m_owner;
    PlayerId m_peerId;
  };

  struct Peer {
    std::shared_ptr<rtc::PeerConnection> connection;
    std::array<std::shared_ptr<rtc::DataChannel>, 4> channels;
    std::array<bool, 4> channelOpen{};
    std::unique_ptr<Link> link;
    bool announced = false;
    bool dead = false;
  };

  explicit Impl(RtcTransport &transport, RtcTransportConfig transportConfig)
      : owner(transport), config(std::move(transportConfig)) {
    worker = std::thread([this] { workerLoop(); });
  }

  ~Impl() {
    {
      std::lock_guard lock(taskMutex);
      running = false;
    }
    taskCv.notify_all();
    if (worker.joinable()) {
      worker.join();
    }
  }

  RtcTransport &owner;
  RtcTransportConfig config;
  mutable std::recursive_mutex mutex;
  std::map<PlayerId, Peer> peers;

  // libdatachannel fires callbacks while holding connection-internal locks
  // (with host-only candidates, even synchronously inside setRemoteDescription)
  // — calling back into the connection from a callback can deadlock. Work that
  // re-enters the connection is queued onto this worker instead
  std::mutex taskMutex;
  std::condition_variable taskCv;
  std::deque<std::function<void()>> tasks;
  std::thread worker;
  bool running = true;

  void post(std::function<void()> task) {
    {
      std::lock_guard lock(taskMutex);
      if (!running) {
        return;
      }
      tasks.push_back(std::move(task));
    }
    taskCv.notify_one();
  }

  void workerLoop() {
    while (true) {
      std::function<void()> task;
      {
        std::unique_lock lock(taskMutex);
        taskCv.wait(lock, [this] { return !running || !tasks.empty(); });
        if (!running && tasks.empty()) {
          return;
        }
        task = std::move(tasks.front());
        tasks.pop_front();
      }
      task();
    }
  }

  rtc::Configuration rtcConfig() const {
    rtc::Configuration configuration;
    if (!config.stunServer.empty()) {
      configuration.iceServers.emplace_back(config.stunServer);
    }
    if (!config.turnServer.empty()) {
      configuration.iceServers.emplace_back(config.turnServer);
    }
    return configuration;
  }

  // Caller holds mutex. Creates the peer connection + shared handlers
  Peer &createPeer(const PlayerId memberId) {
    auto &peer = peers[memberId];
    peer = Peer{};
    peer.connection = std::make_shared<rtc::PeerConnection>(rtcConfig());
    peer.link = std::make_unique<Link>(*this, memberId);

    peer.connection->onGatheringStateChange(
        [this, memberId](const rtc::PeerConnection::GatheringState state) {
          if (state != rtc::PeerConnection::GatheringState::Complete) {
            return;
          }
          post([this, memberId] { sendLocalDescription(memberId); });
        });

    peer.connection->onStateChange(
        [this, memberId](const rtc::PeerConnection::State state) {
          if (state != rtc::PeerConnection::State::Disconnected &&
              state != rtc::PeerConnection::State::Failed &&
              state != rtc::PeerConnection::State::Closed) {
            return;
          }
          bool fire = false;
          {
            std::lock_guard lock(mutex);
            const auto it = peers.find(memberId);
            if (it != peers.end() && it->second.announced &&
                !it->second.dead) {
              it->second.dead = true;
              fire = true;
            }
          }
          if (fire && owner.m_events.peerDisconnected) {
            owner.m_events.peerDisconnected(memberId);
          }
        });

    peer.connection->onDataChannel(
        [this, memberId](std::shared_ptr<rtc::DataChannel> channel) {
          const auto kind = channelForLabel(channel->label());
          if (!kind) {
            return;
          }
          adoptChannel(memberId, *kind, std::move(channel));
        });

    return peer;
  }

  void adoptChannel(const PlayerId memberId, const ChannelKind kind,
                    std::shared_ptr<rtc::DataChannel> channel) {
    {
      std::lock_guard lock(mutex);
      const auto it = peers.find(memberId);
      if (it == peers.end()) {
        return;
      }
      it->second.channels[static_cast<size_t>(kind)] = channel;
    }

    channel->onOpen([this, memberId, kind] { channelOpened(memberId, kind); });

    channel->onMessage([this, memberId, kind](rtc::message_variant message) {
      if (!owner.m_events.messageReceived) {
        return;
      }
      if (std::holds_alternative<rtc::binary>(message)) {
        const auto &binary = std::get<rtc::binary>(message);
        owner.m_events.messageReceived(
            memberId, kind,
            std::span(reinterpret_cast<const uint8_t *>(binary.data()),
                      binary.size()));
      } else {
        const auto &text = std::get<std::string>(message);
        owner.m_events.messageReceived(
            memberId, kind,
            std::span(reinterpret_cast<const uint8_t *>(text.data()),
                      text.size()));
      }
    });

    // A channel that was already open before the handler was set (possible
    // with a fast in-process connection) never fires onOpen again
    if (channel->isOpen()) {
      channelOpened(memberId, kind);
    }
  }

  void sendLocalDescription(const PlayerId memberId) {
    std::optional<rtc::Description> description;
    {
      std::lock_guard lock(mutex);
      const auto it = peers.find(memberId);
      if (it == peers.end() || !it->second.connection) {
        return;
      }
      description = it->second.connection->localDescription();
    }
    if (description && owner.m_signalOut) {
      const nlohmann::json payload = {{"type", description->typeString()},
                                      {"sdp", std::string(*description)}};
      owner.m_signalOut(memberId, payload.dump());
    }
  }

  void channelOpened(const PlayerId memberId, const ChannelKind kind) {
    bool announce = false;
    {
      std::lock_guard lock(mutex);
      const auto it = peers.find(memberId);
      if (it == peers.end()) {
        return;
      }
      auto &peer = it->second;
      peer.channelOpen[static_cast<size_t>(kind)] = true;
      const bool allOpen =
          std::all_of(peer.channelOpen.begin(), peer.channelOpen.end(),
                      [](const bool open) { return open; });
      announce = allOpen && !peer.announced;
      if (announce) {
        peer.announced = true;
      }
    }
    if (!announce) {
      return;
    }
    // The consumer's first sends (Hello etc.) happen inside peerConnected;
    // fire it off-callback so those sends don't re-enter channel internals
    post([this, memberId] {
      IPeerLink *link = nullptr;
      {
        std::lock_guard lock(mutex);
        const auto it = peers.find(memberId);
        if (it == peers.end() || !it->second.announced) {
          return;
        }
        link = it->second.link.get();
      }
      if (owner.m_events.peerConnected && link) {
        owner.m_events.peerConnected(memberId, *link);
      }
    });
  }
};

RtcTransport::RtcTransport(RtcTransportConfig config)
    : m_impl(std::make_unique<Impl>(*this, std::move(config))) {}

RtcTransport::~RtcTransport() { closeAll(); }

void RtcTransport::connectToPeer(const PlayerId memberId,
                                 const bool isOfferer) {
  if (!isOfferer) {
    return; // the answering side reacts to the offer in handleSignal
  }
  std::array<std::shared_ptr<rtc::DataChannel>, 4> created;
  {
    std::lock_guard lock(m_impl->mutex);
    auto &peer = m_impl->createPeer(memberId);
    // Creating the channels triggers negotiation + candidate gathering
    for (size_t i = 0; i < CHANNEL_LABELS.size(); ++i) {
      created[i] = peer.connection->createDataChannel(
          CHANNEL_LABELS[i], initFor(static_cast<ChannelKind>(i)));
    }
  }
  for (size_t i = 0; i < created.size(); ++i) {
    m_impl->adoptChannel(memberId, static_cast<ChannelKind>(i),
                         std::move(created[i]));
  }
}

void RtcTransport::handleSignal(const PlayerId from,
                                const std::string &payload) {
  const auto parsed = nlohmann::json::parse(payload, nullptr, false);
  if (parsed.is_discarded() || !parsed.is_object()) {
    spdlog::warn("[Netplay] undecodable signal payload from {}", from);
    return;
  }
  const auto type = parsed.value("type", "");
  const auto sdp = parsed.value("sdp", "");
  if (sdp.empty()) {
    return;
  }

  try {
    std::lock_guard lock(m_impl->mutex);
    if (type == "offer") {
      auto &peer = m_impl->createPeer(from);
      peer.connection->setRemoteDescription(rtc::Description(sdp, type));
    } else if (type == "answer") {
      const auto it = m_impl->peers.find(from);
      if (it != m_impl->peers.end() && it->second.connection) {
        it->second.connection->setRemoteDescription(rtc::Description(sdp, type));
      }
    }
  } catch (const std::exception &e) {
    spdlog::warn("[Netplay] failed to apply {} from {}: {}", type, from,
                 e.what());
  }
}

void RtcTransport::closePeer(const PlayerId memberId) {
  Impl::Peer removed;
  {
    std::lock_guard lock(m_impl->mutex);
    const auto it = m_impl->peers.find(memberId);
    if (it == m_impl->peers.end()) {
      return;
    }
    removed = std::move(it->second);
    m_impl->peers.erase(it);
  }
  const bool announce = removed.announced && !removed.dead;
  if (removed.connection) {
    removed.connection->close();
  }
  if (announce && m_events.peerDisconnected) {
    m_events.peerDisconnected(memberId);
  }
}

void RtcTransport::closeAll() {
  std::vector<PlayerId> ids;
  {
    std::lock_guard lock(m_impl->mutex);
    for (const auto &[id, peer] : m_impl->peers) {
      ids.push_back(id);
    }
  }
  for (const auto id : ids) {
    closePeer(id);
  }
}

} // namespace firelight::netplay
