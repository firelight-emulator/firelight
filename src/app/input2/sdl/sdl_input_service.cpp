#include "sdl_input_service.hpp"

#include "event_dispatcher.hpp"

#include <SDL_hints.h>
#include <spdlog/spdlog.h>

namespace firelight::input {
SDLInputService::SDLInputService(IControllerRepository &gamepadRepository)
    : m_gamepadRepository(gamepadRepository) {
  SDL_SetHint(SDL_HINT_APP_NAME, "Firelight");
  SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");

  if (SDL_Init(m_sdlServices) < 0) {
    spdlog::error("SDL could not initialize! SDL_Error: {}", SDL_GetError());
  }

  SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
}

SDLInputService::~SDLInputService() {
  if (m_running) {
    stop();
  }
}

int SDLInputService::addGamepad(std::shared_ptr<IGamepad> gamepad) {
  const auto info =
      m_gamepadRepository.getDeviceInfo(gamepad->getDeviceIdentifier());
  if (!info) {
    const auto name = "Default " + gamepad->getName() + " Profile";
    const auto profile = m_gamepadRepository.createProfile(name);
    const auto deviceInfo = DeviceInfo{gamepad->getName(), profile->getId()};

    m_gamepadRepository.updateDeviceInfo(gamepad->getDeviceIdentifier(),
                                         deviceInfo);

    gamepad->setProfile(profile);
  } else {
    const auto profile = m_gamepadRepository.getProfile(info->profileId);
    gamepad->setProfile(profile);
  }

  m_gamepads.emplace_back(gamepad);
  auto nextSlot = getNextAvailablePlayerIndex();
  if (nextSlot == -1) {
    gamepad->setPlayerIndex(-1);
    EventDispatcher::instance().publish(GamepadConnectedEvent{gamepad});
    return -1;
  }

  if (!m_preferGamepadOverKeyboard || m_keyboard == nullptr) {
    m_playerSlots[nextSlot] = gamepad;
    gamepad->setPlayerIndex(nextSlot);
    spdlog::info("Assigned {} to player number {}", gamepad->getName(),
                 nextSlot + 1);

    EventDispatcher::instance().publish(GamepadConnectedEvent{gamepad});
    return gamepad->getPlayerIndex();
  }

  // Find the keyboard and move it to next available slot
  for (int i = 0; i < MAX_PLAYERS; ++i) {
    if (!m_playerSlots.contains(i) || m_playerSlots[i] == nullptr) {
      m_playerSlots[i] = gamepad;
      gamepad->setPlayerIndex(i);
      spdlog::info("Assigned {} to player number {}", gamepad->getName(),
                   i + 1);

      EventDispatcher::instance().publish(GamepadConnectedEvent{gamepad});
      return gamepad->getPlayerIndex();
    }

    if (m_playerSlots.contains(i) && m_playerSlots[i] == m_keyboard) {
      moveGamepadToPlayerIndex(i, nextSlot);

      m_playerSlots[i] = gamepad;
      gamepad->setPlayerIndex(i);
      spdlog::info("Assigned {} to player number {}", gamepad->getName(),
                   i + 1);

      EventDispatcher::instance().publish(GamepadConnectedEvent{gamepad});
      return gamepad->getPlayerIndex();
    }
  }

  return -1;
}

bool SDLInputService::removeGamepadByInstanceId(int instanceId) {
  for (auto it = m_gamepads.begin(); it != m_gamepads.end(); ++it) {
    if (*it && (*it)->getInstanceId() == instanceId) {
      spdlog::info("Removing gamepad: {}", instanceId);
      for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (m_playerSlots.contains(i) && m_playerSlots[i] == *it) {
          m_playerSlots.erase(i);
          spdlog::info("Removed player slot for player {}", i + 1);
        }
      }

      EventDispatcher::instance().publish(
          GamepadDisconnectedEvent{(*it)->getPlayerIndex()});

      (*it)->setPlayerIndex(-1);
      (*it)->disconnect();
      m_gamepads.erase(it);
      it->reset();
      return true;
    }
  }

  return true;
}

bool SDLInputService::removeGamepadByPlayerIndex(int playerIndex) {
  if (!m_playerSlots.contains(playerIndex) ||
      m_playerSlots[playerIndex] == nullptr) {
    return true;
  }

  auto gamepad = m_playerSlots[playerIndex];
  m_playerSlots.erase(playerIndex);

  for (auto it = m_gamepads.begin(); it != m_gamepads.end(); ++it) {
    if (*it == gamepad) {
      spdlog::info("Removing gamepad: {}", gamepad->getInstanceId());
      EventDispatcher::instance().publish(
          GamepadDisconnectedEvent{gamepad->getPlayerIndex()});

      (*it)->setPlayerIndex(-1);
      (*it)->disconnect();
      m_gamepads.erase(it);
      it->reset();
      return true;
    }
  }

  return true;
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

std::shared_ptr<GamepadProfile> SDLInputService::getProfile(const int id) {
  return m_gamepadRepository.getProfile(id);
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
          if (ev.caxis.value < 8192 && ev.caxis.value > -8192) {
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
          if (ev.caxis.value > 8192) {
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
          if (ev.caxis.value < -8192) {
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
          if (ev.caxis.value < 8192 && ev.caxis.value > -8192) {
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
          if (ev.caxis.value > 8192) {
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
          if (ev.caxis.value < -8192) {
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
          if (ev.caxis.value < 8192 && ev.caxis.value > -8192) {
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
          if (ev.caxis.value > 8192) {
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
          if (ev.caxis.value < -8192) {
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
          if (ev.caxis.value < 8192 && ev.caxis.value > -8192) {
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
          if (ev.caxis.value > 8192) {
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
          if (ev.caxis.value < -8192) {
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
          if (ev.caxis.value < 8192) {
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
          if (ev.caxis.value < 8192) {
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

        for (const auto shortcut : gamepad->getToggledShortcuts(input)) {
          EventDispatcher::instance().publish(ShortcutToggledEvent{
              .playerIndex = gamepad->getPlayerIndex(), .shortcut = shortcut});
        }

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

  auto keyboardDeviceIdentifier = DeviceIdentifier{
      .deviceName = "Keyboard",
      .vendorId = -1,
      .productId = -1,
      .productVersion = -1,
  };

  const auto info = m_gamepadRepository.getDeviceInfo(keyboardDeviceIdentifier);
  if (!info) {
    const auto name = "Default Keyboard Profile";
    const auto profile = m_gamepadRepository.createProfile(name);
    const auto deviceInfo = DeviceInfo{"Keyboard", profile->getId()};
    profile->setIsKeyboardProfile(true);

    m_gamepadRepository.updateDeviceInfo(keyboardDeviceIdentifier, deviceInfo);

    m_keyboard->setProfile(profile);
  } else {
    const auto profile = m_gamepadRepository.getProfile(info->profileId);
    profile->setIsKeyboardProfile(true);
    m_keyboard->setProfile(profile);
  }

  for (int i = 0; i < MAX_PLAYERS; ++i) {
    if (!m_playerSlots.contains(i) || m_playerSlots[i] == nullptr) {
      m_playerSlots[i] = m_keyboard;
      m_keyboard->setPlayerIndex(i);
      spdlog::info("Assigned {} to player number {}", m_keyboard->getName(),
                   i + 1);

      EventDispatcher::instance().publish(GamepadConnectedEvent{m_keyboard});
      return;
    }
  }

  m_keyboard->setPlayerIndex(-1);
  EventDispatcher::instance().publish(GamepadConnectedEvent{m_keyboard});
}

void SDLInputService::openSdlGamepad(const int deviceIndex) {
  const auto gameController = SDL_GameControllerOpen(deviceIndex);

  if (!gameController) {
    spdlog::error("Failed to open game controller: {}", SDL_GetError());
    return;
  }

  const auto joystick = SDL_GameControllerGetJoystick(gameController);
  auto joystickInstanceId = SDL_JoystickInstanceID(joystick);

  // Make sure the gamepad is not already registered
  for (const auto &gamepad : m_gamepads) {
    if (gamepad && gamepad->getInstanceId() == joystickInstanceId) {
      spdlog::debug("Gamepad already exists: {}", joystickInstanceId);
      return;
    }
  }

  addGamepad(std::make_shared<SdlController>(gameController));

  // Retrieve the gamepad profile and assign it to the gamepad
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