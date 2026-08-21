#include <firelight/netplay/slot_table.hpp>

namespace firelight::netplay {

std::optional<SlotAssignment> SlotTable::INVALID_SLOT;

const std::optional<SlotAssignment> &SlotTable::slot(const int index) const {
  if (index < 0 || index >= MAX_SLOTS) {
    return INVALID_SLOT;
  }
  return m_slots[index];
}

bool SlotTable::assign(const int index, SlotAssignment assignment) {
  if (index < 0 || index >= MAX_SLOTS) {
    return false;
  }
  m_slots[index] = std::move(assignment);
  return true;
}

void SlotTable::clear(const int index) {
  if (index >= 0 && index < MAX_SLOTS) {
    m_slots[index].reset();
  }
}

void SlotTable::clearMember(const PlayerId memberId) {
  for (auto &slot : m_slots) {
    if (slot && slot->memberId == memberId) {
      slot.reset();
    }
  }
}

void SlotTable::setReady(const PlayerId memberId, const bool ready) {
  for (auto &slot : m_slots) {
    if (slot && slot->memberId == memberId) {
      slot->ready = ready;
    }
  }
}

void SlotTable::renameMember(const PlayerId memberId, const std::string &name) {
  for (auto &slot : m_slots) {
    if (slot && slot->memberId == memberId) {
      slot->displayName = name;
    }
  }
}

std::vector<int> SlotTable::slotsFor(const PlayerId memberId) const {
  std::vector<int> result;
  for (int i = 0; i < MAX_SLOTS; ++i) {
    if (m_slots[i] && m_slots[i]->memberId == memberId) {
      result.push_back(i);
    }
  }
  return result;
}

bool SlotTable::anyRemoteOccupant(const PlayerId hostId) const {
  for (const auto &slot : m_slots) {
    if (slot && slot->memberId != hostId) {
      return true;
    }
  }
  return false;
}

int SlotTable::occupiedCount() const {
  int count = 0;
  for (const auto &slot : m_slots) {
    if (slot) {
      ++count;
    }
  }
  return count;
}

} // namespace firelight::netplay
