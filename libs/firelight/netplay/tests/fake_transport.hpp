#pragma once

#include <firelight/netplay/transport.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace firelight::netplay {

class FakeTransport;

// Wires FakeTransports together by PlayerId, delivering everything
// synchronously — the fake stand-in for the network.
struct FakeTransportHub {
  std::map<PlayerId, FakeTransport *> transports;
};

class FakeTransport final : public IPeerTransport {
public:
  FakeTransport(std::shared_ptr<FakeTransportHub> hub, const PlayerId selfId)
      : m_hub(std::move(hub)), m_selfId(selfId) {
    m_hub->transports[selfId] = this;
  }

  ~FakeTransport() override { m_hub->transports.erase(m_selfId); }

  void connectToPeer(const PlayerId memberId, const bool isOfferer) override {
    if (isOfferer && m_signalOut) {
      m_signalOut(memberId, "offer");
    }
  }

  void handleSignal(const PlayerId from, const std::string &payload) override {
    if (payload == "offer") {
      // Establish before answering: peerConnected must precede any
      // messageReceived from that peer (the ordering real transports give).
      establishWith(from);
      if (m_signalOut) {
        m_signalOut(from, "answer");
      }
    } else if (payload == "answer") {
      establishWith(from);
    }
  }

  void closePeer(const PlayerId memberId) override {
    if (!m_links.contains(memberId)) {
      return;
    }
    m_links.erase(memberId);
    if (m_events.peerDisconnected) {
      m_events.peerDisconnected(memberId);
    }
    if (auto *other = peerTransport(memberId)) {
      other->remoteClosed(m_selfId);
    }
  }

  void closeAll() override {
    const auto links = m_links; // copy: closePeer mutates
    for (const auto &[id, link] : links) {
      closePeer(id);
    }
  }

  // Drops both ends without any lobby change — a network blip.
  void dropLink(const PlayerId memberId) { closePeer(memberId); }

private:
  class FakeLink final : public IPeerLink {
  public:
    FakeLink(FakeTransport &owner, const PlayerId remoteId)
        : m_owner(owner), m_remoteId(remoteId) {}

    void send(const ChannelKind channel,
              std::span<const uint8_t> data) override {
      if (auto *remote = m_owner.peerTransport(m_remoteId)) {
        if (remote->m_events.messageReceived) {
          remote->m_events.messageReceived(m_owner.m_selfId, channel, data);
        }
      }
    }
    [[nodiscard]] size_t bufferedBytes(ChannelKind) const override {
      return 0;
    }
    [[nodiscard]] int roundTripMs() const override { return 1; }

  private:
    FakeTransport &m_owner;
    PlayerId m_remoteId;
  };

  FakeTransport *peerTransport(const PlayerId id) const {
    const auto it = m_hub->transports.find(id);
    return it == m_hub->transports.end() ? nullptr : it->second;
  }

  void establishWith(const PlayerId remoteId) {
    if (m_links.contains(remoteId)) {
      return;
    }
    m_links[remoteId] = std::make_shared<FakeLink>(*this, remoteId);
    if (m_events.peerConnected) {
      m_events.peerConnected(remoteId, *m_links[remoteId]);
    }
  }

  void remoteClosed(const PlayerId remoteId) {
    if (!m_links.contains(remoteId)) {
      return;
    }
    m_links.erase(remoteId);
    if (m_events.peerDisconnected) {
      m_events.peerDisconnected(remoteId);
    }
  }

  std::shared_ptr<FakeTransportHub> m_hub;
  PlayerId m_selfId;
  std::map<PlayerId, std::shared_ptr<FakeLink>> m_links;
};

} // namespace firelight::netplay
