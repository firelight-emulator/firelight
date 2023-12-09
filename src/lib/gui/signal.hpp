//
// Created by alexs on 11/13/2023.
//

#ifndef FIRELIGHT_SIGNAL_HPP
#define FIRELIGHT_SIGNAL_HPP

#include <functional>
namespace FL::GUI {

template <typename... Args> class Signal {
public:
  using Slot = std::function<void(Args...)>;

  void connect(const Slot &slot) { slots.push_back(slot); }

  void emit(Args... args) {
    for (const auto &slot : slots) {
      slot(args...);
    }
  }

private:
  std::vector<Slot> slots;
};

} // namespace FL::GUI

#endif // FIRELIGHT_SIGNAL_HPP
