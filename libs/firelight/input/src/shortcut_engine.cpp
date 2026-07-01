#include <firelight/input/shortcut_engine.hpp>

#include <firelight/input/gamepad_profile.hpp>
#include <firelight/input/igamepad.hpp>
#include <firelight/input/shortcut_mapping.hpp>
#include <firelight/input/shortcut_registry.hpp>

#include "firelight/event_dispatcher.hpp"

namespace firelight::input {

void ShortcutEngine::setContext(const int scope) { m_context = scope; }
int ShortcutEngine::context() const { return m_context; }

void ShortcutEngine::forgetDevice(IGamepad *device) {
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
  auto &dispatcher = EventDispatcher::instance();

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
        dispatcher.publish(ShortcutEvent{.playerIndex = playerIndex,
                                         .id = id,
                                         .phase = ShortcutPhase::Started});
      }
      break;

    case ActivationType::Hold: {
      bool &active = m_holdActive[device][id];
      if (rising && inScope) {
        active = true;
        dispatcher.publish(ShortcutEvent{.playerIndex = playerIndex,
                                         .id = id,
                                         .phase = ShortcutPhase::Started});
      } else if (!satisfied && active) {
        // Release a hold that we started, even if we've since left its scope.
        active = false;
        dispatcher.publish(ShortcutEvent{.playerIndex = playerIndex,
                                         .id = id,
                                         .phase = ShortcutPhase::Ended});
      }
      break;
    }

    case ActivationType::Toggle:
      if (rising && inScope) {
        bool &latch = m_toggleLatch[device][id];
        latch = !latch;
        dispatcher.publish(ShortcutEvent{.playerIndex = playerIndex,
                                         .id = id,
                                         .phase = ShortcutPhase::Started,
                                         .toggledState = latch});
      }
      break;
    }
  }
}

} // namespace firelight::input
