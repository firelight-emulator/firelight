#include <firelight/input/sdl_input_service.hpp>

#include "firelight/event_dispatcher.hpp"

#include <SDL_hints.h>
#include <spdlog/spdlog.h>

namespace firelight::input {
// Axis magnitude past which a stick counts as a directional press for menu
// navigation (25% of the int16 range).
constexpr int kNavStickThreshold = 8192;
SDLInputService::SDLInputService(IControllerRepository &gamepadRepository)
    : m_gamepadRepository(gamepadRepository) {
  SDL_SetHint(SDL_HINT_APP_NAME, "Firelight");
  SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");

  if (SDL_Init(m_sdlServices) < 0) {
    spdlog::error("SDL could not initialize! SDL_Error: {}", SDL_GetError());
  }

  SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");

  // Feed keyboard key events into the shortcut engine (the keyboard is treated
  // like any other device).
  m_keyboardKeyConnection =
      EventDispatcher::instance().subscribe<KeyboardKeyEvent>(
          [this](const KeyboardKeyEvent &event) {
            if (m_keyboard) {
              m_shortcutEngine.onInput(m_keyboard->getPlayerIndex(),
                                       m_keyboard.get(), event.key,
                                       event.pressed);
            }
          });
}

void SDLInputService::setShortcutContext(const int scope) {
  m_shortcutEngine.setContext(scope);
}

SDLInputService::~SDLInputService() {
  if (m_running) {
    stop();
  }
}

std::shared_ptr<GamepadProfile> SDLInputService::resolveProfileForGamepad(
    const std::shared_ptr<IGamepad> &gamepad) {
  std::shared_ptr<GamepadProfile> profile;

  // A per-game override (if active) wins over the device's stored default.
  if (m_gameProfileOverride.has_value()) {
    profile = m_gamepadRepository.getProfile(*m_gameProfileOverride);
  }

  if (!profile) {
    const auto info =
        m_gamepadRepository.getDeviceInfo(gamepad->getDeviceIdentifier());
    if (info) {
      profile = m_gamepadRepository.getProfile(info->profileId);
    } else {
      const auto name = "Default " + gamepad->getName() + " Profile";
      profile = m_gamepadRepository.createProfile(name);
      m_gamepadRepository.updateDeviceInfo(
          gamepad->getDeviceIdentifier(),
          DeviceInfo{gamepad->getName(), profile->getId()});
    }
  }

  // Flag keyboard profiles for the UI (key labels vs button labels). Derived
  // from the device type, so the keyboard is not a separate code path.
  if (profile && gamepad->getDeviceType() == DeviceType::Keyboard) {
    profile->setIsKeyboardProfile(true);
  }
  return profile;
}

void SDLInputService::publishConnected(
    const std::shared_ptr<IGamepad> &gamepad) {
  EventDispatcher::instance().publish(GamepadConnectedEvent{gamepad});
}

void SDLInputService::publishDisconnected(const int playerIndex) {
  EventDispatcher::instance().publish(GamepadDisconnectedEvent{playerIndex});
}

void SDLInputService::assignToSlot(const int slot,
                                   const std::shared_ptr<IGamepad> &gamepad) {
  m_playerSlots[slot] = gamepad;
  gamepad->setPlayerIndex(slot);
  spdlog::info("Assigned {} to player number {}", gamepad->getName(), slot + 1);
}

void SDLInputService::assignPlayerSlot(
    const std::shared_ptr<IGamepad> &gamepad) {
  const int nextSlot = getNextAvailablePlayerIndex();

  // When gamepads are preferred, a new gamepad bumps the keyboard out of the
  // earliest slot it occupies.
  if (gamepad->getDeviceType() == DeviceType::Gamepad &&
      m_preferGamepadOverKeyboard && m_keyboard) {
    for (int i = 0; i < MAX_PLAYERS; ++i) {
      const auto it = m_playerSlots.find(i);
      if (it == m_playerSlots.end() || it->second == nullptr) {
        assignToSlot(i, gamepad);
        return;
      }
      if (it->second == m_keyboard) {
        if (nextSlot != -1) {
          moveGamepadToPlayerIndex(i, nextSlot);
        }
        assignToSlot(i, gamepad);
        return;
      }
    }
    gamepad->setPlayerIndex(-1);
    return;
  }

  if (nextSlot != -1) {
    assignToSlot(nextSlot, gamepad);
  } else {
    gamepad->setPlayerIndex(-1);
  }
}

int SDLInputService::addGamepad(std::shared_ptr<IGamepad> gamepad) {
  gamepad->setProfile(resolveProfileForGamepad(gamepad));
  m_gamepads.emplace_back(gamepad);
  assignPlayerSlot(gamepad);
  publishConnected(gamepad);
  return gamepad->getPlayerIndex();
}

void SDLInputService::reapplyDeviceProfiles() {
  for (const auto &gamepad : m_gamepads) {
    if (gamepad) {
      gamepad->setProfile(resolveProfileForGamepad(gamepad));
    }
  }
}

void SDLInputService::promoteDeviceToPlayerOne(
    const std::shared_ptr<IGamepad> &gamepad) {
  const int current = gamepad->getPlayerIndex();
  if (current == 0) {
    return; // already player one
  }

  const auto occupant = getPlayerGamepad(0);
  assignToSlot(0, gamepad);

  // Move whoever held player one into the promoted device's old slot.
  if (current >= 0) {
    if (occupant) {
      assignToSlot(current, occupant);
    } else {
      m_playerSlots.erase(current);
    }
  } else if (occupant) {
    // The promoted device had no slot; give the old occupant the next free one.
    const int free = getNextAvailablePlayerIndex();
    if (free != -1) {
      assignToSlot(free, occupant);
    } else {
      occupant->setPlayerIndex(-1);
    }
  }

  EventDispatcher::instance().publish(GamepadOrderChangedEvent{});
}

void SDLInputService::applyGameContext(std::optional<std::string> contentHash,
                                       const int platformId) {
  // 1. Per-game profile override.
  m_gameProfileOverride =
      contentHash.has_value()
          ? m_gamepadRepository.getGameProfileOverride(*contentHash)
          : std::nullopt;
  reapplyDeviceProfiles();

  // 2. Preferred controller type for this platform: if a connected controller
  //    matches, promote it to player one.
  if (platformId >= 0) {
    const auto preferred =
        m_gamepadRepository.getPlatformPreferredType(platformId);
    if (preferred.has_value()) {
      for (const auto &gamepad : m_gamepads) {
        if (gamepad && static_cast<int>(gamepad->getType()) == *preferred) {
          promoteDeviceToPlayerOne(gamepad);
          break;
        }
      }
    }
  }
}

void SDLInputService::clearGameContext() {
  m_gameProfileOverride = std::nullopt;
  reapplyDeviceProfiles();
}

bool SDLInputService::removeGamepadByInstanceId(int instanceId) {
  for (auto it = m_gamepads.begin(); it != m_gamepads.end(); ++it) {
    if (!*it || (*it)->getInstanceId() != instanceId) {
      continue;
    }
    const auto gamepad = *it;
    spdlog::info("Removing gamepad: {}", instanceId);
    m_shortcutEngine.forgetDevice(gamepad.get());

    for (int i = 0; i < MAX_PLAYERS; ++i) {
      if (m_playerSlots.contains(i) && m_playerSlots[i] == gamepad) {
        m_playerSlots.erase(i);
      }
    }

    publishDisconnected(gamepad->getPlayerIndex());
    gamepad->setPlayerIndex(-1);
    gamepad->disconnect();
    m_gamepads.erase(it);
    return true;
  }

  return true;
}

bool SDLInputService::removeGamepadByPlayerIndex(int playerIndex) {
  const auto gamepad = getPlayerGamepad(playerIndex);
  if (!gamepad) {
    return true;
  }
  return removeGamepadByInstanceId(gamepad->getInstanceId());
}

std::vector<std::shared_ptr<IGamepad>> SDLInputService::listGamepads() {
  std::vector<std::shared_ptr<IGamepad>> connectedGamepads;
  for (const auto &gamepad : m_gamepads) {
    connectedGamepads.emplace_back(gamepad);
  }

  if (m_keyboard) {
    connectedGamepads.emplace_back(m_keyboard);
  }

  return connectedGamepads;
}

std::shared_ptr<IGamepad>
SDLInputService::getPlayerGamepad(const int playerIndex) {
  if (m_playerSlots.contains(playerIndex)) {
    return m_playerSlots[playerIndex];
  }

  return {};
}

std::optional<libretro::IRetroPad *>
SDLInputService::getRetropadForPlayerIndex(const int t_player) {
  const auto gamepad = getPlayerGamepad(t_player);
  return gamepad
             ? std::optional(static_cast<libretro::IRetroPad *>(gamepad.get()))
             : std::nullopt;
}

std::pair<int16_t, int16_t> SDLInputService::getPointerPosition() const {
  return {m_mouseX, m_mouseY};
}
bool SDLInputService::isPressed() const { return m_mousePressed; }
void SDLInputService::updateMouseState(double x, double y, bool mousePressed) {
  m_mouseX = x * 32767;
  m_mouseY = y * 32767;
  m_mousePressed = mousePressed;
}
void SDLInputService::updateMousePressed(bool mousePressed) {
  m_mousePressed = mousePressed;
}

void SDLInputService::run() {
  spdlog::info("Starting SDL Input Service...");
  while (m_running) {
    SDL_Event ev;
    while (SDL_WaitEvent(&ev)) {
      switch (ev.type) {
      case SDL_CONTROLLERDEVICEADDED:
        openSdlGamepad(ev.cdevice.which);
        break;
      case SDL_CONTROLLERDEVICEREMOVED:
        removeGamepadByInstanceId(ev.cdevice.which);
        break;
      case SDL_CONTROLLERAXISMOTION: {
        const auto joystickInstanceId = ev.cbutton.which;
        std::shared_ptr<IGamepad> gamepad;
        for (const auto &g : m_gamepads) {
          if (g && g->getInstanceId() == joystickInstanceId) {
            gamepad = g;
            break;
          }
        }

        if (!gamepad) {
          break;
        }

        auto index = gamepad->getPlayerIndex();

        switch (ev.caxis.axis) {
        case SDL_CONTROLLER_AXIS_LEFTX: {
          if (ev.caxis.value < kNavStickThreshold && ev.caxis.value > -kNavStickThreshold) {
            if (m_gamepadLastStates[index][LeftStickLeft]) {
              m_gamepadLastStates[index][LeftStickLeft] = false;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = LeftStickLeft,
                  .pressed = false,
              });
            }

            if (m_gamepadLastStates[index][LeftStickRight]) {
              m_gamepadLastStates[index][LeftStickRight] = false;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = LeftStickRight,
                  .pressed = false,
              });
            }
            break;
          }
          if (ev.caxis.value > kNavStickThreshold) {
            if (!m_gamepadLastStates[index][LeftStickRight]) {
              m_gamepadLastStates[index][LeftStickRight] = true;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = LeftStickRight,
                  .pressed = true,
              });
            }
            break;
          }
          if (ev.caxis.value < -kNavStickThreshold) {
            if (!m_gamepadLastStates[index][LeftStickLeft]) {
              m_gamepadLastStates[index][LeftStickLeft] = true;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = LeftStickLeft,
                  .pressed = true,
              });
            }
            break;
          }
          break;
        }
        case SDL_CONTROLLER_AXIS_LEFTY: {
          if (ev.caxis.value < kNavStickThreshold && ev.caxis.value > -kNavStickThreshold) {
            if (m_gamepadLastStates[index][LeftStickUp]) {
              m_gamepadLastStates[index][LeftStickUp] = false;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = LeftStickUp,
                  .pressed = false,
              });
            }

            if (m_gamepadLastStates[index][LeftStickDown]) {
              m_gamepadLastStates[index][LeftStickDown] = false;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = LeftStickDown,
                  .pressed = false,
              });
            }
            break;
          }
          if (ev.caxis.value > kNavStickThreshold) {
            if (!m_gamepadLastStates[index][LeftStickDown]) {
              m_gamepadLastStates[index][LeftStickDown] = true;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = LeftStickDown,
                  .pressed = true,
              });
            }
            break;
          }
          if (ev.caxis.value < -kNavStickThreshold) {
            if (!m_gamepadLastStates[index][LeftStickUp]) {
              m_gamepadLastStates[index][LeftStickUp] = true;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = LeftStickUp,
                  .pressed = true,
              });
            }
            break;
          }
          break;
        }
        case SDL_CONTROLLER_AXIS_RIGHTX: {
          if (ev.caxis.value < kNavStickThreshold && ev.caxis.value > -kNavStickThreshold) {
            if (m_gamepadLastStates[index][RightStickLeft]) {
              m_gamepadLastStates[index][RightStickLeft] = false;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = RightStickLeft,
                  .pressed = false,
              });
            }

            if (m_gamepadLastStates[index][RightStickRight]) {
              m_gamepadLastStates[index][RightStickRight] = false;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = RightStickRight,
                  .pressed = false,
              });
            }
            break;
          }
          if (ev.caxis.value > kNavStickThreshold) {
            if (!m_gamepadLastStates[index][RightStickRight]) {
              m_gamepadLastStates[index][RightStickRight] = true;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = RightStickRight,
                  .pressed = true,
              });
            }
            break;
          }
          if (ev.caxis.value < -kNavStickThreshold) {
            if (!m_gamepadLastStates[index][RightStickLeft]) {
              m_gamepadLastStates[index][RightStickLeft] = true;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = RightStickLeft,
                  .pressed = true,
              });
            }
            break;
          }
          break;
        }
        case SDL_CONTROLLER_AXIS_RIGHTY: {
          if (ev.caxis.value < kNavStickThreshold && ev.caxis.value > -kNavStickThreshold) {
            if (m_gamepadLastStates[index][RightStickUp]) {
              m_gamepadLastStates[index][RightStickUp] = false;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = RightStickUp,
                  .pressed = false,
              });
            }

            if (m_gamepadLastStates[index][RightStickDown]) {
              m_gamepadLastStates[index][RightStickDown] = false;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = RightStickDown,
                  .pressed = false,
              });
            }
            break;
          }
          if (ev.caxis.value > kNavStickThreshold) {
            if (!m_gamepadLastStates[index][RightStickDown]) {
              m_gamepadLastStates[index][RightStickDown] = true;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = RightStickDown,
                  .pressed = true,
              });
            }
            break;
          }
          if (ev.caxis.value < -kNavStickThreshold) {
            if (!m_gamepadLastStates[index][RightStickUp]) {
              m_gamepadLastStates[index][RightStickUp] = true;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = RightStickUp,
                  .pressed = true,
              });
            }
            break;
          }
          break;
        }
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT: {
          if (ev.caxis.value < kNavStickThreshold) {
            if (m_gamepadLastStates[index][LeftTrigger]) {
              m_gamepadLastStates[index][LeftTrigger] = false;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = LeftTrigger,
                  .pressed = false,
              });
              break;
            }

            break;
          }

          if (!m_gamepadLastStates[index][LeftTrigger]) {
            m_gamepadLastStates[index][LeftTrigger] = true;
            EventDispatcher::instance().publish(GamepadInputEvent{
                .playerIndex = index,
                .input = LeftTrigger,
                .pressed = true,
            });
          }

          break;
        }
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: {
          if (ev.caxis.value < kNavStickThreshold) {
            if (m_gamepadLastStates[index][RightTrigger]) {
              m_gamepadLastStates[index][RightTrigger] = false;
              EventDispatcher::instance().publish(GamepadInputEvent{
                  .playerIndex = index,
                  .input = RightTrigger,
                  .pressed = false,
              });
              break;
            }

            break;
          }

          if (!m_gamepadLastStates[index][RightTrigger]) {
            m_gamepadLastStates[index][RightTrigger] = true;
            EventDispatcher::instance().publish(GamepadInputEvent{
                .playerIndex = index,
                .input = RightTrigger,
                .pressed = true,
            });
          }

          break;
        }
        }
        break;
      }
      case SDL_CONTROLLERBUTTONUP: {
        const auto joystickInstanceId = ev.cbutton.which;
        std::shared_ptr<IGamepad> gamepad;
        for (const auto &g : m_gamepads) {
          if (g && g->getInstanceId() == joystickInstanceId) {
            gamepad = g;
            break;
          }
        }

        if (!gamepad) {
          break;
        }

        const auto input = sdlToGamepadInputs.contains(ev.cbutton.button)
                               ? sdlToGamepadInputs.at(ev.cbutton.button)
                               : None;

        m_shortcutEngine.onInput(gamepad->getPlayerIndex(), gamepad.get(),
                                 static_cast<int>(input), false);

        EventDispatcher::instance().publish(GamepadInputEvent{
            .playerIndex = gamepad->getPlayerIndex(),
            .input = input,
            .pressed = false,
        });

        break;
      }
      case SDL_CONTROLLERBUTTONDOWN: {
        const auto joystickInstanceId = ev.cbutton.which;
        std::shared_ptr<IGamepad> gamepad;
        for (const auto &g : m_gamepads) {
          if (g && g->getInstanceId() == joystickInstanceId) {
            gamepad = g;
            break;
          }
        }

        if (!gamepad) {
          break;
        }

        const auto input = sdlToGamepadInputs.contains(ev.cbutton.button)
                               ? sdlToGamepadInputs.at(ev.cbutton.button)
                               : None;

        m_shortcutEngine.onInput(gamepad->getPlayerIndex(), gamepad.get(),
                                 static_cast<int>(input), true);

        EventDispatcher::instance().publish(GamepadInputEvent{
            .playerIndex = gamepad->getPlayerIndex(),
            .input = input,
            .pressed = true,
        });

        break;
      }
      case SDL_KEYDOWN:
      case SDL_KEYUP:
      case SDL_JOYAXISMOTION:
      case SDL_JOYBUTTONUP:
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYDEVICEADDED:
      case SDL_JOYDEVICEREMOVED:
        spdlog::debug("Ignoring event type: {}", ev.type);
        break;
      case SDL_QUIT:
        spdlog::info("Quitting SDL Input Service");
        SDL_QuitSubSystem(m_sdlServices);
        SDL_Quit();
        return;
      default:
        spdlog::debug("Got an unhandled SDL event {}", ev.type);
        break;
      }
    }
  }

  SDL_QuitSubSystem(m_sdlServices);
  SDL_Quit();

  spdlog::info("Stopping SDL Input Service...");
}
void SDLInputService::stop() {
  m_running = false;
  SDL_Event quitEvent;
  quitEvent.type = SDL_QUIT;
  SDL_PushEvent(&quitEvent);
}

void SDLInputService::changeGamepadOrder(
    const std::map<int, int> &oldToNewIndex) {
  std::map<int, std::shared_ptr<IGamepad>> newPlayerSlots;

  for (const auto &[oldIndex, newIndex] : oldToNewIndex) {
    if (m_playerSlots.contains(oldIndex)) {
      const auto gamepad = m_playerSlots[oldIndex];
      m_playerSlots.erase(oldIndex);
      newPlayerSlots[newIndex] = gamepad;
      if (gamepad) {
        gamepad->setPlayerIndex(newIndex);
        spdlog::info("Changed player slot for {} to {}", gamepad->getName(),
                     newIndex + 1);
      }
    }
  }

  for (auto i = 0; i < MAX_PLAYERS; ++i) {
    auto gamepad = newPlayerSlots.contains(i) ? newPlayerSlots[i] : nullptr;
    if (gamepad) {
      m_playerSlots[i] = gamepad;
    } else {
      m_playerSlots.erase(i);
    }
  }

  EventDispatcher::instance().publish(GamepadOrderChangedEvent{});
}

bool SDLInputService::preferGamepadOverKeyboard() const {
  return m_preferGamepadOverKeyboard;
}

void SDLInputService::setPreferGamepadOverKeyboard(const bool prefer) {
  m_preferGamepadOverKeyboard = prefer;
}

void SDLInputService::setKeyboard(std::shared_ptr<IGamepad> keyboard) {
  m_keyboard = std::move(keyboard);
  // The keyboard uses the same profile-resolution and slot-assignment path as
  // any other device (its DeviceType is Keyboard).
  m_keyboard->setProfile(resolveProfileForGamepad(m_keyboard));
  assignPlayerSlot(m_keyboard);
  publishConnected(m_keyboard);
}

void SDLInputService::openSdlGamepad(const int deviceIndex) {
  const auto gameController = SDL_GameControllerOpen(deviceIndex);

  if (!gameController) {
    spdlog::error("Failed to open game controller: {}", SDL_GetError());
    return;
  }

  const auto joystick = SDL_GameControllerGetJoystick(gameController);
  auto joystickInstanceId = SDL_JoystickInstanceID(joystick);

  for (const auto &gamepad : m_gamepads) {
    if (gamepad && gamepad->getInstanceId() == joystickInstanceId) {
      spdlog::debug("Gamepad already exists: {}", joystickInstanceId);
      return;
    }
  }

  addGamepad(std::make_shared<SdlController>(gameController));
}

int SDLInputService::getNextAvailablePlayerIndex() const {
  for (int i = 0; i < MAX_PLAYERS; ++i) {
    if (!m_playerSlots.contains(i) || m_playerSlots.at(i) == nullptr) {
      return i;
    }
  }

  return -1;
}

bool SDLInputService::moveGamepadToPlayerIndex(int oldIndex, int newIndex) {
  if (oldIndex == newIndex || !m_playerSlots.contains(oldIndex) ||
      (m_playerSlots.contains(newIndex) &&
       m_playerSlots[newIndex] != nullptr)) {
    spdlog::warn("Cannot move gamepad from {} "
                 "to {}: invalid indices",
                 oldIndex, newIndex);
    return false;
  }

  m_playerSlots[newIndex] = m_playerSlots[oldIndex];
  m_playerSlots[newIndex]->setPlayerIndex(newIndex);
  m_playerSlots.erase(oldIndex);

  spdlog::info("Moved {} from player slot {} to {}",
               m_playerSlots[newIndex]->getName(), oldIndex + 1, newIndex + 1);
  return true;
}

} // namespace firelight::input
