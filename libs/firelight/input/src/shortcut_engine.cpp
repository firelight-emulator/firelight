#include <firelight/input/shortcut_engine.hpp>

#include <firelight/input/gamepad_profile.hpp>
#include <firelight/input/igamepad.hpp>
#include <firelight/input/shortcut_mapping.hpp>
#include <firelight/input/shortcut_registry.hpp>

#include "firelight/event_dispatcher.hpp"

#include <vector>

namespace firelight::input {

void ShortcutEngine::setContext(const int scope) {
  std::lock_guard lock(m_mutex);
  m_context = scope;
}
int ShortcutEngine::context() const {
  std::lock_guard lock(m_mutex);
  return m_context;
}

void ShortcutEngine::forgetDevice(IGamepad *device) {
  std::lock_guard lock(m_mutex);
  m_held.erase(device);
  m_satisfied.erase(device);
  m_holdActive.erase(device);
  m_toggleLatch.erase(device);
}

bool ShortcutEngine::isSourceSatisfied(IGamepad *device,
                                       const InputSource &source) const {
  if (source.type == SourceType::None) {
    return false;
  }
  const auto devIt = m_held.find(device);
  if (devIt == m_held.end()) {
    return false;
  }
  const auto &held = devIt->second;
  const auto isHeld = [&held](const int code) {
    const auto it = held.find(code);
    return it != held.end() && it->second;
  };

  if (!isHeld(source.code)) {
    return false;
  }
  for (const auto mod : source.modifiers) {
    if (!isHeld(mod)) {
      return false;
    }
  }
  return true;
}

void ShortcutEngine::onInput(const int playerIndex, IGamepad *device,
                             const int code, const bool pressed) {
  if (!device) {
    return;
  }

  // Detect edges under the lock, buffering events, then publish them after
  // releasing so a subscriber can't re-enter the engine and deadlock.
  std::vector<ShortcutEvent> events;
  {
    std::lock_guard lock(m_mutex);
    m_held[device][code] = pressed;

    const auto profile = device->getProfile();
    if (!profile) {
      return;
    }
    const auto shortcuts = profile->getShortcutMapping();
    if (!shortcuts) {
      return;
    }

    auto &registry = ShortcutRegistry::instance();

    for (const auto &[id, sources] : shortcuts->getAll()) {
      const auto *action = registry.getAction(id);
      if (!action) {
        continue;
      }

      bool satisfied = false;
      for (const auto &source : sources) {
        if (isSourceSatisfied(device, source)) {
          satisfied = true;
          break;
        }
      }

      bool &previous = m_satisfied[device][id];
      if (satisfied == previous) {
        continue; // no edge for this shortcut
      }
      const bool rising = satisfied && !previous;
      previous = satisfied;

      const bool inScope = (action->scope & m_context) != 0;

      switch (action->activation) {
      case ActivationType::Press:
        if (rising && inScope) {
          events.push_back(ShortcutEvent{.playerIndex = playerIndex,
                                         .id = id,
                                         .phase = ShortcutPhase::Started});
        }
        break;

      case ActivationType::Hold: {
        bool &active = m_holdActive[device][id];
        if (rising && inScope) {
          active = true;
          events.push_back(ShortcutEvent{.playerIndex = playerIndex,
                                         .id = id,
                                         .phase = ShortcutPhase::Started});
        } else if (!satisfied && active) {
          // Release a hold that we started, even if we've left its scope.
          active = false;
          events.push_back(ShortcutEvent{.playerIndex = playerIndex,
                                         .id = id,
                                         .phase = ShortcutPhase::Ended});
        }
        break;
      }

      case ActivationType::Toggle:
        if (rising && inScope) {
          bool &latch = m_toggleLatch[device][id];
          latch = !latch;
          events.push_back(ShortcutEvent{.playerIndex = playerIndex,
                                         .id = id,
                                         .phase = ShortcutPhase::Started,
                                         .toggledState = latch});
        }
        break;
      }
    }
  }

  auto &dispatcher = EventDispatcher::instance();
  for (const auto &event : events) {
    dispatcher.publish(event);
  }
}

} // namespace firelight::input
