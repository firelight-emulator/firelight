#pragma once

#include <array>
#include <atomic>

namespace firelight::input {

// The inputs a device is currently withholding from the running game.
//
// When a shortcut fires, the input that triggered it is suppressed until it is
// physically released, so the combo that fast-forwards doesn't also press a
// button in the game. Only the trigger is withheld, never the modifiers: a
// modifier was already down when the combo completed, so masking it would hand
// the core a release it never got, and unmasking would hand it a second press.
//
// Written by the shortcut engine on the input thread; read by the core's input
// callback on the render thread, per port per button per frame. Lock-free
// because that read is the hottest path in the input system, and a mutex there
// would contend thousands of times a second over a collection that is nearly
// always empty. Four slots because a combo can only withhold its one trigger,
// and no one holds more than a handful of shortcuts at once.
class InputSuppressor {
public:
  void suppress(const int code) {
    if (isSuppressed(code)) {
      return;
    }
    for (auto &slot : m_codes) {
      int empty = EMPTY;
      if (slot.compare_exchange_strong(empty, code, std::memory_order_relaxed)) {
        return;
      }
    }
    // More than four at once: drop it rather than grow. The input reaches the
    // game, which is the same as not having suppression at all.
  }

  void release(const int code) {
    for (auto &slot : m_codes) {
      int held = code;
      slot.compare_exchange_strong(held, EMPTY, std::memory_order_relaxed);
    }
  }

  [[nodiscard]] bool isSuppressed(const int code) const {
    for (const auto &slot : m_codes) {
      if (slot.load(std::memory_order_relaxed) == code) {
        return true;
      }
    }
    return false;
  }

  void clear() {
    for (auto &slot : m_codes) {
      slot.store(EMPTY, std::memory_order_relaxed);
    }
  }

private:
  // Not a code any device reports, so it reads as an empty slot.
  static constexpr int EMPTY = -1;

  std::array<std::atomic<int>, 4> m_codes{EMPTY, EMPTY, EMPTY, EMPTY};
};

} // namespace firelight::input
