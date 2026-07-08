#pragma once

#include "remote_retropad.hpp"

#include <firelight/libretro/retropad_provider.hpp>
#include <firelight/netplay/session.hpp>

#include <atomic>
#include <map>
#include <memory>
#include <mutex>

namespace firelight::netplay {

// Maps libretro ports to lobby slots while the host runs an online game:
// slot N's occupant answers port N — the host through their local pads,
// remote members through their RemoteRetroPads. Inactive (or with no inner
// provider change needed), it's a pure pass-through.
class SlotMappedRetropadProvider final : public libretro::IRetropadProvider {
public:
  SlotMappedRetropadProvider(libretro::IRetropadProvider *localPads,
                             NetplaySession &session)
      : m_localPads(localPads), m_session(session) {}

  void setActive(const bool active) { m_active = active; }
  [[nodiscard]] bool active() const { return m_active.load(); }

  std::shared_ptr<RemoteRetroPad> padForMember(const PlayerId memberId) {
    std::lock_guard lock(m_padsMutex);
    auto &pad = m_pads[memberId];
    if (!pad) {
      pad = std::make_shared<RemoteRetroPad>();
    }
    return pad;
  }

  std::shared_ptr<libretro::IRetroPad>
  getRetropadForPlayerIndex(const int port) override {
    if (!m_active.load()) {
      return m_localPads ? m_localPads->getRetropadForPlayerIndex(port)
                         : nullptr;
    }
    const auto table = m_session.slotTable();
    const auto &slot = table.slot(port);
    if (!slot) {
      return nullptr;
    }
    if (slot->memberId == m_session.lobby().hostId) {
      return m_localPads
                 ? m_localPads->getRetropadForPlayerIndex(slot->localPadIndex)
                 : nullptr;
    }
    return padForMember(slot->memberId);
  }

private:
  libretro::IRetropadProvider *m_localPads = nullptr;
  NetplaySession &m_session;
  std::atomic<bool> m_active{false};
  std::mutex m_padsMutex;
  std::map<PlayerId, std::shared_ptr<RemoteRetroPad>> m_pads;
};

} // namespace firelight::netplay
