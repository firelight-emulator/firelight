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
  m_running = false;
  SDL_Event quitEvent;
  quitEvent.type = SDL_QUIT;
  SDL_PushEvent(&quitEvent);
  SDL_QuitSubSystem(m_sdlServices);
  SDL_Quit();
}

std::vector<std::shared_ptr<IGamepad>> SDLInputService::listGamepads() {
  std::vector<std::shared_ptr<IGamepad>> gamepads(m_gamepads.size());
  for (const auto &gamepad : m_gamepads) {
    if (gamepad) {
      gamepads.emplace_back(gamepad);
    }
  }

  return gamepads;
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

void SDLInputService::run() {
  spdlog::info("Starting SDL Input Service...");
  while (m_running) {
    SDL_Event ev;
    while (SDL_WaitEvent(&ev)) {
      switch (ev.type) {
      case SDL_CONTROLLERDEVICEADDED:
        addGamepad(ev.cdevice.which);
        break;
      case SDL_CONTROLLERDEVICEREMOVED:
        removeGamepad(ev.cdevice.which);
        break;
      case SDL_CONTROLLERAXISMOTION: {
        const auto joystickInstanceId = ev.cbutton.which;
        std::shared_ptr<SdlController> gamepad;
        for (const auto &g : m_gamepads) {
          if (g->getInstanceId() == joystickInstanceId) {
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
        std::shared_ptr<SdlController> gamepad;
        for (const auto &g : m_gamepads) {
          if (g->getInstanceId() == joystickInstanceId) {
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
        std::shared_ptr<SdlController> gamepad;
        for (const auto &g : m_gamepads) {
          if (g->getInstanceId() == joystickInstanceId) {
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
        return;
      default:
        spdlog::debug("Got an unhandled SDL event {}", ev.type);
        break;
      }
    }
  }
  spdlog::info("Stopping SDL Input Service...");
}

void SDLInputService::changeGamepadOrder(
    const std::map<int, int> &oldToNewIndex) {
  std::map<int, std::shared_ptr<SdlController>> newPlayerSlots;

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

  for (const auto &[newIndex, gamepad] : newPlayerSlots) {
    if (gamepad) {
      m_playerSlots[newIndex] = gamepad;
    } else {
      m_playerSlots.erase(newIndex);
    }
  }

  EventDispatcher::instance().publish(GamepadOrderChangedEvent{});
}

void SDLInputService::addGamepad(const int deviceIndex) {
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

  auto controller = std::make_shared<SdlController>(gameController);

  // Retrieve the gamepad profile and assign it to the gamepad
  const auto info =
      m_gamepadRepository.getDeviceInfo(controller->getDeviceIdentifier());
  if (!info) {
    const auto name = "Default " + controller->getName() + " Profile";
    const auto profile = m_gamepadRepository.createProfile(name);
    const auto deviceInfo = DeviceInfo{controller->getName(), profile->getId()};

    m_gamepadRepository.updateDeviceInfo(controller->getDeviceIdentifier(),
                                         deviceInfo);

    controller->setProfile(profile);
  } else {
    const auto profile = m_gamepadRepository.getProfile(info->profileId);
    controller->setProfile(profile);
  }

  m_gamepads.emplace_back(controller);
  for (int i = 0; i < MAX_PLAYERS; ++i) {
    if (!m_playerSlots.contains(i) || m_playerSlots[i] == nullptr) {
      m_playerSlots[i] = controller;
      controller->setPlayerIndex(i);
      spdlog::info("Assigned {} to player number {}", controller->getName(),
                   i + 1);

      EventDispatcher::instance().publish(GamepadConnectedEvent{controller});
      return;
    }
  }

  controller->setPlayerIndex(-1);
  EventDispatcher::instance().publish(GamepadConnectedEvent{controller});

  spdlog::info("Player slots full; not assigning gamepad to player slot");
}

void SDLInputService::removeGamepad(int joystickInstanceId) {
  for (auto it = m_gamepads.begin(); it != m_gamepads.end(); ++it) {
    if (*it && (*it)->getInstanceId() == joystickInstanceId) {
      spdlog::info("Removing gamepad: {}", joystickInstanceId);
      for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (m_playerSlots.contains(i) &&
            m_playerSlots[i]->getInstanceId() == joystickInstanceId) {
          m_playerSlots.erase(i);
          spdlog::info("Removed player slot for player {}", i + 1);
        }
      }

      EventDispatcher::instance().publish(GamepadDisconnectedEvent{*it});

      SDL_GameControllerClose((*it)->getSDLController());
      it->reset();
      m_gamepads.erase(it);
      return;
    }
  }
}

} // namespace firelight::input