#pragma once

#include "transport.hpp"

#include <memory>
#include <string>

namespace firelight::netplay {

struct RtcTransportConfig {
  // Empty = host candidates only (LAN/loopback; used by tests).
  std::string stunServer = "stun:stun.l.google.com:19302";
  // Optional relay for symmetric-NAT pairs ("turn:user:pass@host:port").
  std::string turnServer;
};

// WebRTC data-channel transport (libdatachannel): one peer connection per
// member with four channels mapped from ChannelKind — Control/Video reliable
// ordered, Audio/Input unreliable unordered. Signaling is non-trickle: each
// side emits a single offer/answer payload once candidate gathering finishes.
// Events fire on libdatachannel's threads.
class RtcTransport final : public IPeerTransport {
public:
  explicit RtcTransport(RtcTransportConfig config = {});
  ~RtcTransport() override;

  void connectToPeer(PlayerId memberId, bool isOfferer) override;
  void handleSignal(PlayerId from, const std::string &payload) override;
  void closePeer(PlayerId memberId) override;
  void closeAll() override;

private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

} // namespace firelight::netplay
