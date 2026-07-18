#pragma once

#include "protocol.hpp"

#include <cstdint>
#include <functional>
#include <span>
#include <string>

namespace firelight::netplay {

enum class ChannelKind : uint8_t { Control = 0, Video = 1, Audio = 2, Input = 3 };

// One established peer connection. Control/Video are reliable+ordered,
// Audio/Input are unreliable — implementations map ChannelKind accordingly
class IPeerLink {
public:
  virtual ~IPeerLink() = default;

  virtual void send(ChannelKind channel, std::span<const uint8_t> data) = 0;
  // Bytes queued but not yet handed to the network — the backpressure probe
  [[nodiscard]] virtual size_t bufferedBytes(ChannelKind channel) const = 0;
  [[nodiscard]] virtual int roundTripMs() const = 0;
};

// Ordering contract: peerConnected fires before any messageReceived from that
// peer, and no messageReceived fires after its peerDisconnected
struct TransportEvents {
  std::function<void(PlayerId, IPeerLink &)> peerConnected;
  std::function<void(PlayerId)> peerDisconnected;
  std::function<void(PlayerId, ChannelKind, std::span<const uint8_t>)>
      messageReceived;
};

// Peer-to-peer data plane. Connection handshakes ride the lobby backend's
// signaling channel: the transport emits payloads via signalOut, and inbound
// payloads are delivered to handleSignal.
class IPeerTransport {
public:
  virtual ~IPeerTransport() = default;

  // Guest side calls with isOfferer=true toward the host; the host side
  // accepts inbound offers via handleSignal.
  virtual void connectToPeer(PlayerId memberId, bool isOfferer) = 0;
  virtual void handleSignal(PlayerId from, const std::string &payload) = 0;
  virtual void closePeer(PlayerId memberId) = 0;
  virtual void closeAll() = 0;

  void setEvents(TransportEvents events) { m_events = std::move(events); }
  void setSignalOut(
      std::function<void(PlayerId to, const std::string &payload)> out) {
    m_signalOut = std::move(out);
  }

protected:
  TransportEvents m_events;
  std::function<void(PlayerId, const std::string &)> m_signalOut;
};

} // namespace firelight::netplay
