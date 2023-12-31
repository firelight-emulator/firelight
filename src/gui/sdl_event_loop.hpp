//
// Created by alexs on 1/6/2024.
//

#ifndef SDL_EVENT_LOOP_HPP
#define SDL_EVENT_LOOP_HPP
#include "controller_manager.hpp"

#include <qthread.h>

namespace Firelight {

class SdlEventLoop final : public QThread {
public:
  explicit SdlEventLoop(Input::ControllerManager *manager);
  void stop();

protected:
  void run() override;

private:
  bool m_running = true;
  Input::ControllerManager *m_controllerManager;
};

} // namespace Firelight

#endif // SDL_EVENT_LOOP_HPP
