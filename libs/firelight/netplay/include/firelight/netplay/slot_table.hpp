#pragma once

#include "protocol.hpp"

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace firelight::netplay {

// One player slot (P1..P8). localPadIndex is which of the member's local
// controllers drives this slot, so one member can hold several slots
struct SlotAssignment {
  PlayerId memberId = 0;
  int localPadIndex = 0;
  std::string displayName;
  bool ready = false;

  bool operator==(const SlotAssignment &) const = default;
};

class SlotTable {
public:
  [[nodiscard]] const std::optional<SlotAssignment> &slot(int index) const;
  bool assign(int index, SlotAssignment assignment);
  void clear(int index);
  void clearMember(PlayerId memberId);
  // Ready is a per-member flag; applies to every slot they hold
  void setReady(PlayerId memberId, bool ready);
  void renameMember(PlayerId memberId, const std::string &name);

  [[nodiscard]] std::vector<int> slotsFor(PlayerId memberId) const;
  [[nodiscard]] bool anyRemoteOccupant(PlayerId hostId) const;
  [[nodiscard]] int occupiedCount() const;

  // Named all() rather than slots(): Qt defines `slots` as a macro, and this
  // header is included from Qt code
  [[nodiscard]] const std::array<std::optional<SlotAssignment>, MAX_SLOTS> &
  all() const {
    return m_slots;
  }

  bool operator==(const SlotTable &) const = default;

private:
  std::array<std::optional<SlotAssignment>, MAX_SLOTS> m_slots{};
  static std::optional<SlotAssignment> INVALID_SLOT;
};

} // namespace firelight::netplay
