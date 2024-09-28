//
// Created by alexs on 1/6/2024.
//

#include "controller_manager.hpp"
#include "../../gui/PlatformInputListModel.hpp"

#include <spdlog/spdlog.h>
#include <string>

namespace firelight::Input {
  void ControllerManager::setKeyboardRetropad(libretro::IRetroPad *keyboard) {
    m_keyboard = keyboard;
  }

  void ControllerManager::handleSDLControllerEvent(const SDL_Event &event) {
    switch (event.type) {
      case SDL_CONTROLLERDEVICEADDED: {
        openControllerWithDeviceIndex(event.cdevice.which);
        break;
      }
      case SDL_CONTROLLERDEVICEREMOVED: {
        spdlog::info("Controller disconnected");
        const auto joystickInstanceId = event.cdevice.which;

        for (int i = 0; i < m_controllers.max_size(); ++i) {
          if (m_controllers[i] != nullptr &&
              m_controllers[i]->getInstanceId() == joystickInstanceId) {
            m_controllers[i].reset();
            m_numControllers--;
            emit controllerDisconnected();
            spdlog::debug("We found it and we're unplugging it");
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
  ControllerManager::getRetropadForPlayerIndex(const int t_player) {
    const auto controller = getControllerForPlayer(t_player);
    if (!controller && m_keyboard) {
      return m_keyboard;
    }

    return std::nullopt;
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

    // TODO: Check if any controllers have the same joystick id.

    for (int i = 0; i < m_controllers.max_size(); ++i) {
      if (m_controllers[i] == nullptr) {
        SDL_GameControllerSetPlayerIndex(controller, i);

        m_numControllers++;
        m_controllers[i] =
            std::make_unique<Controller>(controller, t_deviceIndex);
        m_controllers[i]->setControllerProfile(std::make_shared<input::ControllerProfile>());
        printf("ADDING CONTROLLER\n");
        break;
      }
    }

    spdlog::info("Controller device added with device index {}", t_deviceIndex);
    emit controllerConnected();
  }

  std::optional<Controller *>
  ControllerManager::getControllerForPlayer(const int t_player) const {
    if (m_controllers.at(t_player) != nullptr) {
      return {m_controllers.at(t_player).get()};
    }

    return {};
  }

  void ControllerManager::updateControllerOrder(const QVariantMap &map) {
    std::unique_ptr<Controller> tempCon[map.size()];

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
    std::array<std::unique_ptr<Controller>, 32> tempCon{};
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

  QAbstractListModel *ControllerManager::getPlatformInputModel(int platformId) {
    if (platformId == 0) {
      return new PlatformInputListModel(this);
    }

    return new PlatformInputListModel(this);
  }
} // namespace firelight::Input
