#pragma once

#include "controller_manager.hpp"

#include <qthread.h>

namespace firelight {
  class SdlEventLoop final : public QThread {
  public:
    explicit SdlEventLoop(QObject *window, Input::ControllerManager *manager);

    ~SdlEventLoop() override;

    void stopProcessing();

  protected:
    void run() override;

  private:
    void processEvents() const;

    QObject *m_window;
    bool m_running = true;
    Input::ControllerManager *m_controllerManager;
  };
} // namespace firelight
