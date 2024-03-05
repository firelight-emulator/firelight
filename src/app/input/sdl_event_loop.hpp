#pragma once

#include "controller_manager.hpp"

#include <qthread.h>

namespace firelight {

class SdlEventLoop final : public QThread {
public:
  explicit SdlEventLoop(Input::ControllerManager *manager);
  ~SdlEventLoop() override;
  void stopProcessing();

protected:
  void run() override;

private:
  void processEvents() const;
  bool m_running = true;
  Input::ControllerManager *m_controllerManager;
};

} // namespace firelight
