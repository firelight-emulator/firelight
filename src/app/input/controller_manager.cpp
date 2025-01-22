#include "controller_manager.hpp"

#include <spdlog/spdlog.h>
#include <string>

#include "controller_icons.hpp"

namespace firelight::input {
  ControllerManager::ControllerManager(input::IControllerRepository &controllerRepository) : m_controllerRepository(
    controllerRepository) {
    m_settings = std::make_unique<QSettings>();

    m_prioritizeControllerOverKeyboard = m_settings->value("controllers/prioritizeControllerOverKeyboard", true).toBool();
  }

  void ControllerManager::setKeyboardRetropad(input::KeyboardInputHandler *keyboard) {
    m_controllers[0] = std::unique_ptr<SdlController>(keyboard);
    m_controllers[0]->setControllerProfile(m_controllerRepository.getControllerProfile("Keyboard", 0, 0, 0));
    m_numControllers++;
    emit controllerConnected(1, "Keyboard", ControllerIcons::sourceUrlFromType(KEYBOARD));
  }

  void ControllerManager::handleSDLControllerEvent(const SDL_Event &event) {
    switch (event.type) {
      case SDL_CONTROLLERDEVICEADDED: {
        openControllerWithDeviceIndex(event.cdevice.which);
        break;
      }
      case SDL_CONTROLLERDEVICEREMOVED: {
        const auto joystickInstanceId = event.cdevice.which;

        for (int i = 0; i < m_controllers.max_size(); ++i) {
          if (m_controllers[i] != nullptr &&
              m_controllers[i]->getInstanceId() == joystickInstanceId) {
            auto name = m_controllers[i]->getName();
            const auto type = m_controllers[i]->getType();
            m_controllers[i].reset();
            m_numControllers--;
            emit controllerDisconnected(i + 1, QString::fromStdString(name), ControllerIcons::sourceUrlFromType(type));
            spdlog::debug("We found it and we're unplugging it");
          }
        }

        break;
      }
      case SDL_CONTROLLERBUTTONDOWN: {
        const auto joystickInstanceId = event.cbutton.which;
        for (int i = 0; i < m_controllers.max_size(); ++i) {
          if (m_controllers[i] != nullptr &&
              m_controllers[i]->getInstanceId() == joystickInstanceId) {
            const auto button = event.cbutton.button;
            libretro::IRetroPad::Input virtualInput = libretro::IRetroPad::Input::Unknown;
            switch (button) {
              case SDL_CONTROLLER_BUTTON_A:
                virtualInput = libretro::IRetroPad::Input::SouthFace;
                break;
              case SDL_CONTROLLER_BUTTON_B:
                virtualInput = libretro::IRetroPad::Input::EastFace;
                break;
              case SDL_CONTROLLER_BUTTON_X:
                virtualInput = libretro::IRetroPad::Input::WestFace;
                break;
              case SDL_CONTROLLER_BUTTON_Y:
                virtualInput = libretro::IRetroPad::Input::NorthFace;
                break;
              case SDL_CONTROLLER_BUTTON_DPAD_UP:
                virtualInput = libretro::IRetroPad::Input::DpadUp;
                break;
              case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                virtualInput = libretro::IRetroPad::Input::DpadDown;
                break;
              case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                virtualInput = libretro::IRetroPad::Input::DpadLeft;
                break;
              case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                virtualInput = libretro::IRetroPad::Input::DpadRight;
                break;
              case SDL_CONTROLLER_BUTTON_START:
                virtualInput = libretro::IRetroPad::Input::Start;
                break;
              case SDL_CONTROLLER_BUTTON_BACK:
                virtualInput = libretro::IRetroPad::Input::Select;
                break;
              case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                virtualInput = libretro::IRetroPad::Input::LeftBumper;
                break;
              case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                virtualInput = libretro::IRetroPad::Input::RightBumper;
                break;
              case SDL_CONTROLLER_BUTTON_LEFTSTICK:
                virtualInput = libretro::IRetroPad::Input::L3;
                break;
              case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
                virtualInput = libretro::IRetroPad::Input::R3;
                break;
              default:
                break;
            }
            emit retropadInputStateChanged(i + 1, virtualInput, true);
            spdlog::debug("Player {} Button {} pressed", i + 1, button);
            emit buttonStateChanged(i + 1, button, true);
          }
        }
        break;
      }
      case SDL_CONTROLLERBUTTONUP: {
        const auto joystickInstanceId = event.cbutton.which;
        for (int i = 0; i < m_controllers.max_size(); ++i) {
          if (m_controllers[i] != nullptr &&
              m_controllers[i]->getInstanceId() == joystickInstanceId) {
            const auto button = event.cbutton.button;
            libretro::IRetroPad::Input virtualInput = libretro::IRetroPad::Input::Unknown;
            switch (button) {
              case SDL_CONTROLLER_BUTTON_A:
                virtualInput = libretro::IRetroPad::Input::SouthFace;
                break;
              case SDL_CONTROLLER_BUTTON_B:
                virtualInput = libretro::IRetroPad::Input::EastFace;
                break;
              case SDL_CONTROLLER_BUTTON_X:
                virtualInput = libretro::IRetroPad::Input::WestFace;
                break;
              case SDL_CONTROLLER_BUTTON_Y:
                virtualInput = libretro::IRetroPad::Input::NorthFace;
                break;
              case SDL_CONTROLLER_BUTTON_DPAD_UP:
                virtualInput = libretro::IRetroPad::Input::DpadUp;
                break;
              case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                virtualInput = libretro::IRetroPad::Input::DpadDown;
                break;
              case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                virtualInput = libretro::IRetroPad::Input::DpadLeft;
                break;
              case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                virtualInput = libretro::IRetroPad::Input::DpadRight;
                break;
              case SDL_CONTROLLER_BUTTON_START:
                virtualInput = libretro::IRetroPad::Input::Start;
                break;
              case SDL_CONTROLLER_BUTTON_BACK:
                virtualInput = libretro::IRetroPad::Input::Select;
                break;
              case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                virtualInput = libretro::IRetroPad::Input::LeftBumper;
                break;
              case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                virtualInput = libretro::IRetroPad::Input::RightBumper;
                break;
              case SDL_CONTROLLER_BUTTON_LEFTSTICK:
                virtualInput = libretro::IRetroPad::Input::L3;
                break;
              case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
                virtualInput = libretro::IRetroPad::Input::R3;
                break;
              default:
                break;
            }
            emit retropadInputStateChanged(i + 1, virtualInput, false);
            spdlog::debug("Player {} Button {} released", i + 1, button);
            emit buttonStateChanged(i + 1, button, false);
          }
        }
        break;
      }
      case SDL_CONTROLLERAXISMOTION: {
        for (int i = 0; i < m_controllers.max_size(); ++i) {
          if (m_controllers[i] != nullptr &&
              m_controllers[i]->getInstanceId() == event.caxis.which) {
            switch (event.caxis.axis) {
              case SDL_CONTROLLER_AXIS_LEFTX: {
                if (event.caxis.value < 8192 && event.caxis.value > -8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::LeftStickLeft, false);
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::LeftStickRight, false);
                  break;
                }
                if (event.caxis.value > 8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::LeftStickRight, true);
                  break;
                }
                if (event.caxis.value < -8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::LeftStickLeft, true);
                  break;
                }
                emit axisStateChanged(i + 1, event.caxis.axis, event.caxis.value);
                break;
              }
              case SDL_CONTROLLER_AXIS_LEFTY: {
                if (event.caxis.value < 8192 && event.caxis.value > -8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::LeftStickUp, false);
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::LeftStickDown, false);
                  break;
                }
                if (event.caxis.value > 8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::LeftStickDown, true);
                  break;
                }
                if (event.caxis.value < -8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::LeftStickUp, true);
                  break;
                }
                emit axisStateChanged(i + 1, event.caxis.axis, event.caxis.value);
                break;
              }
              case SDL_CONTROLLER_AXIS_RIGHTX: {
                if (event.caxis.value < 8192 && event.caxis.value > -8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::RightStickLeft, false);
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::RightStickRight, false);
                  break;
                }
                if (event.caxis.value > 8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::RightStickRight, true);
                  break;
                }
                if (event.caxis.value < -8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::RightStickLeft, true);
                  break;
                }
                emit axisStateChanged(i + 1, event.caxis.axis, event.caxis.value);
                break;
              }
              case SDL_CONTROLLER_AXIS_RIGHTY: {
                if (event.caxis.value < 8192 && event.caxis.value > -8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::RightStickUp, false);
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::RightStickDown, false);
                  break;
                }
                if (event.caxis.value > 8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::RightStickDown, true);
                  break;
                }
                if (event.caxis.value < -8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::RightStickUp, true);
                  break;
                }
                emit axisStateChanged(i + 1, event.caxis.axis, event.caxis.value);
                break;
              }
              case SDL_CONTROLLER_AXIS_TRIGGERLEFT: {
                if (event.caxis.value < 8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::LeftTrigger, false);
                  break;
                }

                emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::LeftTrigger, true);
                emit axisStateChanged(i + 1, event.caxis.axis, event.caxis.value);
                break;
              }
              case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: {
                if (event.caxis.value < 8192) {
                  emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::RightTrigger, false);
                  break;
                }

                emit retropadInputStateChanged(i + 1, libretro::IRetroPad::Input::RightTrigger, true);
                emit axisStateChanged(i + 1, event.caxis.axis, event.caxis.value);
                break;
              }
            }


            emit axisStateChanged(i + 1, event.caxis.axis, event.caxis.value);
          }
        }
        break;
      }
      default:
        // spdlog::debug("Got an unhandled SDL event {}", ev.type);
        break;
    }
  }

  void ControllerManager::refreshControllerList() {
    const auto numJoys = SDL_NumJoysticks();
    for (int i = 0; i < numJoys; ++i) {
      openControllerWithDeviceIndex(i);
    }
  }

  std::optional<libretro::IRetroPad *>
  ControllerManager::getRetropadForPlayerIndex(const int t_player, const int platformId) {
    const auto controller = getControllerForPlayerIndex(t_player);
    // if (t_player == 0 && !controller && m_keyboard) {
    //   return m_keyboard;
    // }

    if (controller) {
      (*controller)->
          setActiveMapping(m_controllerRepository.getInputMapping((*controller)->getProfileId(), platformId));
    }

    return controller;
  }

  void ControllerManager::openControllerWithDeviceIndex(int32_t t_deviceIndex) {
    // Index refers to N'th controller, so only shows order
    // InstanceID refers to a specific controller for the duration of its
    // session

    for (int i = 0; i < m_controllers.max_size(); ++i) {
      if (m_controllers[i] != nullptr &&
          m_controllers[i]->getDeviceIndex() == t_deviceIndex) {
        return;
      }
    }

    auto controller = SDL_GameControllerOpen(t_deviceIndex);
    if (controller == nullptr) {
      return;
    }

    // TODO: Check the repository for known controller types
    // Get the default profile for the controller type
    // If it doesn't exist, create a new one
    const auto profile = m_controllerRepository.getControllerProfile(
      SDL_GameControllerName(controller),
      SDL_GameControllerGetVendor(controller),
      SDL_GameControllerGetProduct(controller),
      SDL_GameControllerGetProductVersion(controller));

    // std::shared_ptr<input::ControllerProfile> profile;
    // if (info) {
    //   profile = m_controllerRepository.getControllerProfile(info->defaultProfileId);
    // } else {
    //   auto newInfo = input::ControllerInfo{};
    //   newInfo.name = SDL_GameControllerName(controller);
    //   newInfo.vendorId = SDL_GameControllerGetVendor(controller);
    //   newInfo.productId = SDL_GameControllerGetProduct(controller);
    //   newInfo.productVersion = SDL_GameControllerGetProductVersion(controller);
    //
    //   m_controllerRepository.addControllerInfo(newInfo);
    //   profile = m_controllerRepository.getControllerProfile(newInfo.defaultProfileId);
    // }

    // TODO: Check if any controllers have the same joystick id.

    for (int i = 0; i < m_controllers.max_size(); ++i) {
      if (m_prioritizeControllerOverKeyboard && m_controllers[i] != nullptr && m_controllers[i]->getType() == KEYBOARD) {
        for (int j = i; j < m_controllers.max_size(); ++j) {
          if (m_controllers[j] == nullptr) {
            m_controllers[j] = std::move(m_controllers[i]);
            m_controllers[j]->setPlayerIndex(j);
            m_controllers[i].reset();
            emit controllerOrderChanged();
            break;
          }
        }
      }

      if (m_controllers[i] == nullptr) {
        SDL_GameControllerSetPlayerIndex(controller, i);
        m_numControllers++;
        m_controllers[i] =
            std::make_unique<SdlController>(controller, t_deviceIndex);
        m_controllers[i]->setControllerProfile(profile);
        const auto name = m_controllers[i]->getName();
        const auto type = m_controllers[i]->getType();

        emit controllerConnected(i + 1, QString::fromStdString(name), ControllerIcons::sourceUrlFromType(type));
        break;
      }

      // if (m_controllers[i]->getType() == KEYBOARD) {
      //   for (int j = i; j < m_controllers.max_size(); ++j) {
      //     if (m_controllers[j] == nullptr) {
      //       m_controllers[j] = std::move(m_controllers[i]);
      //       m_controllers[j]->setPlayerIndex(j);
      //     }
      //   }
      // }
      // if (m_controllers[i] == nullptr) {
      //   if (m_keyboardPlayerIndex == i) {
      //     for (int j = 0; j < m_controllers.max_size(); ++j) {
      //       if (m_controllers[j] == nullptr) {
      //         m_keyboardPlayerIndex = j;
      //       }
      //     }
      //   }
      //   SDL_GameControllerSetPlayerIndex(controller, i);
      //
      //   m_numControllers++;
      //   m_controllers[i] =
      //       std::make_unique<SdlController>(controller, t_deviceIndex);
      //   m_controllers[i]->setControllerProfile(profile);
      //   const auto name = m_controllers[i]->getName();
      //   const auto type = m_controllers[i]->getType();
      //
      //   emit controllerConnected(i + 1, QString::fromStdString(name), ControllerIcons::sourceUrlFromType(type));
      //   break;
      // }
    }
  }

  std::optional<input::IGamepad *>
  ControllerManager::getControllerForPlayerIndex(const int t_player) const {
    if (m_controllers.at(t_player) != nullptr) {
      return {m_controllers.at(t_player).get()};
    }

    return {};
  }

  bool ControllerManager::prioritizeControllerOverKeyboard() const {
    return m_prioritizeControllerOverKeyboard;
  }

  void ControllerManager::updateControllerOrder(const QVariantMap &map) {
    std::unique_ptr<SdlController> tempCon[map.size()];

    auto emittedSignal = false;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
      const auto to = it.key().toInt();
      const auto from = it.value().toInt();
      if (from == to) {
        continue;
      }

      if (!emittedSignal) {
        emittedSignal = true;
        // emit layoutAboutToBeChanged();
      }

      tempCon[to] = std::move(m_controllers[from]);
      tempCon[to]->setPlayerIndex(-1);
    }

    for (auto i = 0; i < map.size(); ++i) {
      if (tempCon[i] != nullptr) {
        m_controllers[i] = std::move(tempCon[i]);
        m_controllers[i]->setPlayerIndex(i);
      }
    }

    // emit layoutChanged();
  }

  void ControllerManager::updateControllerOrder(const QVector<int> &order) {
    std::array<std::unique_ptr<SdlController>, 32> tempCon{};
    for (int i = 0; i < order.size(); i++) {
      if (!m_controllers[order[i]]) {
        continue;
      }

      tempCon[i] = std::move(m_controllers[order[i]]);
      tempCon[i]->setPlayerIndex(i);

      // if (order[i] == i) {
      //   printf("No change for %d\n", i);
      //   continue;
      // }

      // move controller from order[i] to i
    }

    m_controllers = std::move(tempCon);
    emit controllerOrderChanged();
  }

  bool ControllerManager::blockGamepadInput() const {
    return m_blockGamepadInput;
  }

  void ControllerManager::setBlockGamepadInput(const bool blockGamepadInput) {
    if (m_blockGamepadInput == blockGamepadInput) {
      return;
    }

    m_blockGamepadInput = blockGamepadInput;
    emit blockGamepadInputChanged();
  }
  void ControllerManager::setPrioritizeControllerOverKeyboard(
      const bool prioritizeControllerOverKeyboard) {
    if (m_prioritizeControllerOverKeyboard == prioritizeControllerOverKeyboard) {
      return;
    }

    m_settings->setValue("controllers/prioritizeControllerOverKeyboard", prioritizeControllerOverKeyboard);
    m_prioritizeControllerOverKeyboard = prioritizeControllerOverKeyboard;
    emit prioritizeControllerOverKeyboardChanged();
  }
  } // namespace firelight::input
