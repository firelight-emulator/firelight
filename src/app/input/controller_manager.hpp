//
// Created by alexs on 1/6/2024.
//

#ifndef CONTROLLER_MANAGER_HPP
#define CONTROLLER_MANAGER_HPP
#include "../libretro/retropad_provider.hpp"
#include "controller.hpp"
#include "keyboard_controller.hpp"

#include <QObject>
#include <SDL_events.h>

namespace Firelight::Input {

class ControllerManager final : public QObject,
                                public libretro::IRetropadProvider {
  Q_OBJECT

public:
  ControllerManager() = default;
  void handleSDLControllerEvent(const SDL_Event &event);
  void refreshControllerList();

  [[nodiscard]] std::optional<Controller *>
  getControllerForPlayer(int t_player) const;
  std::optional<libretro::IRetroPad *>
  getRetropadForPlayer(int t_player) override;

signals:
  void controllerConnected();
  void controllerDisconnected();

private:
  KeyboardController m_keyboardController{};
  std::array<std::unique_ptr<Controller>, 32> m_controllers{};

  void openControllerWithDeviceIndex(int32_t t_deviceIndex);
};

} // namespace Firelight::Input

#endif // CONTROLLER_MANAGER_HPP
