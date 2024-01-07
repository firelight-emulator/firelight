//
// Created by alexs on 1/6/2024.
//

#ifndef CONTROLLER_MANAGER_HPP
#define CONTROLLER_MANAGER_HPP
#include "../app/libretro/retropad_provider.hpp"
#include "src/app/controller/controller.hpp"

#include <QObject>
#include <SDL_events.h>

namespace firelight::input {

class ControllerManager final : public QObject,
                                public libretro::IRetropadProvider {
  Q_OBJECT

public:
  ControllerManager() = default;
  void handleSDLControllerEvent(const SDL_Event &event);
  void refreshControllerList();

  std::optional<Controller *> getControllerForPlayer(int t_player) const;
  std::optional<libretro::IRetroPad *>
  getRetropadForPlayer(int t_player) override;

signals:
  void controllerConnected();
  void controllerDisconnected();

private:
  std::array<std::unique_ptr<Controller>, 32> m_controllers{};

  void openControllerWithDeviceIndex(int32_t t_deviceIndex);
};

} // namespace firelight::input

#endif // CONTROLLER_MANAGER_HPP
