#include "core_input_router.hpp"

#include "libretro/libretro.h"

namespace firelight::libretro {

int CoreInputRouter::getPortInputClass(const unsigned port) const {
  const auto it = m_portInputClass.find(port);
  return it != m_portInputClass.end()
           ? it->second
           : static_cast<int>(firelight::input::GamepadInputClass::Joypad);
}

void CoreInputRouter::pollInput() {
  // Glide the shared cursor from a gamepad stick on any Mouse/Light-Gun port
  if (m_pointerInputProvider && m_retropadProvider) {
    namespace fi = firelight::input;
    for (const auto &[port, deviceClass]: m_portInputClass) {
      if (deviceClass != static_cast<int>(fi::GamepadInputClass::Mouse) &&
          deviceClass != static_cast<int>(fi::GamepadInputClass::Lightgun)) {
        continue;
      }
      // Fall back to player 0 so one controller still aims a gun the game
      // auto-placed elsewhere (e.g. Duck Hunt's Zapper on port 2)
      auto pad =
          m_retropadProvider->getRetropadForPlayerIndex(static_cast<int>(port));
      if (!pad) {
        pad = m_retropadProvider->getRetropadForPlayerIndex(0);
      }
      if (!pad) {
        continue;
      }
      const auto stickX = pad->getLeftStickXPosition(m_platformId, 1);
      const auto stickY = pad->getLeftStickYPosition(m_platformId, 1);
      m_pointerInputProvider->nudgeCursor(
        cursorGlideDelta(stickX, m_analogPointerSpeed),
        cursorGlideDelta(stickY, m_analogPointerSpeed));
      break;
    }
  }

  // getRelativeMotion consumes the delta, so read it once for the whole frame
  if (m_pointerInputProvider) {
    m_frameMouseDelta = m_pointerInputProvider->getRelativeMotion();
  } else {
    m_frameMouseDelta = {0, 0};
  }

  for (int port = 0; port < MAX_INPUT_PORTS; ++port) {
    const auto pad =
        m_retropadProvider
          ? m_retropadProvider->getRetropadForPlayerIndex(port)
          : nullptr;
    if (pad) {
      m_portFrames[port] =
          firelight::input::captureJoypadFrame(*pad, m_platformId, 1);
      m_portActive[port] = true;
    } else {
      m_portActive[port] = false;
    }
  }
}

int16_t CoreInputRouter::readInputState(const unsigned port,
                                        const unsigned device,
                                        const unsigned index,
                                        const unsigned id) const {
  if (device == RETRO_DEVICE_POINTER) {
    const auto pointerProvider = m_pointerInputProvider;
    if (pointerProvider == nullptr) {
      return 0;
    }

    if (id == RETRO_DEVICE_ID_POINTER_X) {
      return pointerProvider->getPointerPosition().first;
    }
    if (id == RETRO_DEVICE_ID_POINTER_Y) {
      return pointerProvider->getPointerPosition().second;
    }
    if (id == RETRO_DEVICE_ID_POINTER_PRESSED) {
      return pointerProvider->isPressed();
    }
  }

  // Branch on the device the core actually queries (masked), not what we set:
  // some cores query LIGHTGUN ids for a MOUSE-advertised device (FCEUmm Zapper)
  const auto deviceClass = device & RETRO_DEVICE_MASK;
  if (deviceClass == RETRO_DEVICE_MOUSE ||
      deviceClass == RETRO_DEVICE_LIGHTGUN) {
    namespace fi = firelight::input;
    // Inert unless the physical mouse (toggle) or the gamepad (port set to this
    // device type) is allowed to drive it
    const int selectedClass = getPortInputClass(port);
    const bool gamepadAllowed =
        (deviceClass == RETRO_DEVICE_MOUSE &&
         selectedClass == static_cast<int>(fi::GamepadInputClass::Mouse)) ||
        (deviceClass == RETRO_DEVICE_LIGHTGUN &&
         selectedClass == static_cast<int>(fi::GamepadInputClass::Lightgun));
    const bool mouseAllowed = m_mouseControlsPointerDevices;
    if (!mouseAllowed && !gamepadAllowed) {
      return 0;
    }
    const auto pointer = m_pointerInputProvider;
    const auto provider = m_retropadProvider;
    const auto pad = provider
                       ? provider->getRetropadForPlayerIndex(port)
                       : nullptr;
    const int mapClass = static_cast<int>(
      deviceClass == RETRO_DEVICE_MOUSE
        ? fi::GamepadInputClass::Mouse
        : fi::GamepadInputClass::Lightgun);
    const auto mapped = [&](const fi::GamepadInput vi) -> bool {
      return gamepadAllowed && pad &&
             pad->isVirtualInputActive(m_platformId, mapClass,
                                       static_cast<int>(vi));
    };
    const auto mouseBtn = [&](const int btn) -> bool {
      return mouseAllowed && pointer && pointer->isMouseButtonPressed(btn);
    };

    if (deviceClass == RETRO_DEVICE_MOUSE) {
      switch (id) {
        case RETRO_DEVICE_ID_MOUSE_X:
          return m_frameMouseDelta.first;
        case RETRO_DEVICE_ID_MOUSE_Y:
          return m_frameMouseDelta.second;
        case RETRO_DEVICE_ID_MOUSE_LEFT:
          return mouseBtn(RETRO_DEVICE_ID_MOUSE_LEFT) || mapped(fi::MouseLeft);
        case RETRO_DEVICE_ID_MOUSE_RIGHT:
          return mouseBtn(RETRO_DEVICE_ID_MOUSE_RIGHT) || mapped(fi::MouseRight);
        case RETRO_DEVICE_ID_MOUSE_MIDDLE:
          return mouseBtn(RETRO_DEVICE_ID_MOUSE_MIDDLE) ||
                 mapped(fi::MouseMiddle);
        default:
          return 0;
      }
    }

    // RETRO_DEVICE_LIGHTGUN
    switch (id) {
      case RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X:
        return pointer ? pointer->getPointerPosition().first : 0;
      case RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y:
        return pointer ? pointer->getPointerPosition().second : 0;
      case RETRO_DEVICE_ID_LIGHTGUN_X: // deprecated relative (snes9x)
        return m_frameMouseDelta.first;
      case RETRO_DEVICE_ID_LIGHTGUN_Y:
        return m_frameMouseDelta.second;
      case RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN:
        // Only the mouse can be off-screen; a gamepad-driven cursor never is
        return mouseAllowed && pointer && pointer->isPointerOffscreen();
      case RETRO_DEVICE_ID_LIGHTGUN_TRIGGER:
        return (mouseAllowed && pointer && pointer->isPressed()) ||
               mapped(fi::LightgunTrigger);
      case RETRO_DEVICE_ID_LIGHTGUN_RELOAD:
        // Right-click reloads (shoot off-screen) by convention
        return mouseBtn(RETRO_DEVICE_ID_MOUSE_RIGHT) || mapped(fi::LightgunReload);
      case RETRO_DEVICE_ID_LIGHTGUN_AUX_A:
        return mapped(fi::LightgunAuxA);
      case RETRO_DEVICE_ID_LIGHTGUN_AUX_B:
        return mapped(fi::LightgunAuxB);
      case RETRO_DEVICE_ID_LIGHTGUN_AUX_C:
        return mapped(fi::LightgunAuxC);
      case RETRO_DEVICE_ID_LIGHTGUN_START:
        return mapped(fi::LightgunStart);
      case RETRO_DEVICE_ID_LIGHTGUN_SELECT:
        return mapped(fi::LightgunSelect);
      case RETRO_DEVICE_ID_LIGHTGUN_DPAD_UP:
        return mapped(fi::LightgunDpadUp);
      case RETRO_DEVICE_ID_LIGHTGUN_DPAD_DOWN:
        return mapped(fi::LightgunDpadDown);
      case RETRO_DEVICE_ID_LIGHTGUN_DPAD_LEFT:
        return mapped(fi::LightgunDpadLeft);
      case RETRO_DEVICE_ID_LIGHTGUN_DPAD_RIGHT:
        return mapped(fi::LightgunDpadRight);
      default:
        return 0;
    }
  }

  // Joypad reads come from the per-frame snapshot (pollInput)
  if (port >= static_cast<unsigned>(MAX_INPUT_PORTS) || !m_portActive[port]) {
    return 0;
  }
  const auto &frame = m_portFrames[port];

  if (device == RETRO_DEVICE_ANALOG) {
    if (index == RETRO_DEVICE_INDEX_ANALOG_LEFT) {
      if (id == RETRO_DEVICE_ID_ANALOG_X) {
        return frame.leftStickX;
      }
      if (id == RETRO_DEVICE_ID_ANALOG_Y) {
        return frame.leftStickY;
      }
    } else if (index == RETRO_DEVICE_INDEX_ANALOG_RIGHT) {
      if (id == RETRO_DEVICE_ID_ANALOG_X) {
        return frame.rightStickX;
      }
      if (id == RETRO_DEVICE_ID_ANALOG_Y) {
        return frame.rightStickY;
      }
    }
  } else if (device == RETRO_DEVICE_JOYPAD) {
    if (id == RETRO_DEVICE_ID_JOYPAD_MASK) {
      return frame.buttonMask();
    }
    return frame.button(id) ? 1 : 0;
  }

  return 0;
}

} // namespace firelight::libretro
