//
// Created by alexs on 1/6/2024.
//

#include "controller_manager.hpp"

#include <spdlog/spdlog.h>

namespace firelight::input {

void ControllerManager::handleSDLControllerEvent(const SDL_Event &event) {
  switch (event.type) {
  case SDL_CONTROLLERDEVICEADDED: {
    spdlog::info("Controller connected");
    auto joystickIndex = event.cdevice.which;
    spdlog::debug("Controller device added with joystick id {}", joystickIndex);

    // Index refers to N'th controller, so only shows order
    // InstanceID refers to a specific controller for the duration of its
    // session
    auto controller = SDL_GameControllerOpen(joystickIndex);
    if (controller == nullptr) {
      return;
    }

    for (int i = 0; i < m_controllers.max_size(); ++i) {
      if (m_controllers[i] == nullptr) {
        m_controllers[i] =
            std::make_unique<Controller>(controller, joystickIndex);
        break;
      }
    }

    emit controllerConnected();
    break;
  }
  case SDL_CONTROLLERDEVICEREMOVED:
    spdlog::info("Controller disconnected");
    emit controllerDisconnected();
    break;
  default:
    // spdlog::debug("Got an unhandled SDL event {}", ev.type);
    break;
  }
}

std::optional<libretro::IRetroPad *>
ControllerManager::getRetropadForPlayer(const int t_player) {
  return getControllerForPlayer(t_player);
}

std::optional<Controller *>
ControllerManager::getControllerForPlayer(const int t_player) const {
  if (m_controllers.at(t_player) != nullptr) {
    return {m_controllers.at(t_player).get()};
  }

  return {};
}
} // namespace firelight::input