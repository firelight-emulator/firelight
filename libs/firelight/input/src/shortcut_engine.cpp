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

  // TODO
  // Entering or leaving gameplay drops every mask. A ScopeAlways action firing
  // in a menu suppresses an input nothing is reading, and that mask would
  // otherwise still be there when the game came back
  for (const auto &[device, held] : m_held) {
    device->suppressor().clear();
  }
}
int ShortcutEngine::context() const {
  std::lock_guard lock(m_mutex);
  return m_context;
}

bool ShortcutEngine::hotkeysEnabled(IGamepad *device) const {
  std::lock_guard lock(m_mutex);
  const auto it = m_hotkeysDisabled.find(device);
  return it == m_hotkeysDisabled.end() || !it->second;
}

void ShortcutEngine::setHotkeysEnabled(IGamepad *device, const bool enabled) {
  std::lock_guard lock(m_mutex);
  m_hotkeysDisabled[device] = !enabled;
  if (!enabled) {
    // Anything this device was withholding goes back to the game immediately;
    // handing the game its inputs is the entire point of turning these off
    device->suppressor().clear();
  }
}

void ShortcutEngine::forgetDevice(IGamepad *device) {
  std::lock_guard lock(m_mutex);
  device->suppressor().clear();
  m_held.erase(device);
  m_satisfied.erase(device);
  m_holdActive.erase(device);
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
  // releasing so a subscriber can't re-enter the engine and deadlock
  std::vector<ShortcutEvent> events;
  std::vector<HotkeysToggledEvent> toggles;
  {
    std::lock_guard lock(m_mutex);
    m_held[device][code] = pressed;

    // The game gets this input back the moment it's physically let go, whether
    // or not the shortcut it fired is still active
    if (!pressed) {
      device->suppressor().release(code);
    }

    const auto profile = device->getProfile();
    if (!profile) {
      return;
    }
    const auto shortcuts = profile->getShortcutMapping();
    if (!shortcuts) {
      return;
    }

    auto &registry = ShortcutRegistry::instance();

    const bool disabled = m_hotkeysDisabled[device];

    for (const auto &[id, sources] : shortcuts->getAll()) {
      const auto *action = registry.getAction(id);
      if (!action) {
        continue;
      }
      // While a device's hotkeys are off, every input belongs to the game —
      // except the one that turns them back on, or there'd be no way back
      if (disabled && id != TOGGLE_HOTKEYS_ID) {
        continue;
      }

      bool satisfied = false;
      int satisfiedBy = -1;
      for (const auto &source : sources) {
        if (isSourceSatisfied(device, source)) {
          satisfied = true;
          satisfiedBy = source.code;
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

      // TODO
      // Withhold the input that fired this from the game, until it is physically
      // released. Done here, under the lock and before the event is published,
      // because a subscriber runs a queued hop later — by which time the core
      // would already have sampled the button for a frame or more
      //
      // Only the trigger, never the modifiers: a modifier was already down when
      // the combo completed, so masking it now would hand the core a release it
      // never got, and unmasking would hand it a second press. That leak is why
      // a modifier belongs on a button the game rarely reads
      if (rising && inScope) {
        device->suppressor().suppress(satisfiedBy);
      }

      switch (action->activation) {
      case ActivationType::Press:
        if (rising && inScope) {
          // Handled here rather than downstream because it is the one action
          // about the device itself, and the device is only known here
          if (id == TOGGLE_HOTKEYS_ID) {
            m_hotkeysDisabled[device] = !disabled;
            if (!disabled) {
              device->suppressor().clear();
            }
            toggles.push_back(HotkeysToggledEvent{.playerIndex = playerIndex,
                                                  .enabled = disabled});
          }
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
          // Release a hold that we started, even if we've left its scope
          active = false;
          events.push_back(ShortcutEvent{.playerIndex = playerIndex,
                                         .id = id,
                                         .phase = ShortcutPhase::Ended});
        }
        break;
      }
      }
    }
  }

  auto &dispatcher = EventDispatcher::instance();
  for (const auto &toggle : toggles) {
    EventDispatcher::instance().publish(toggle);
  }
  for (const auto &event : events) {
    dispatcher.publish(event);
  }
}

} // namespace firelight::input
