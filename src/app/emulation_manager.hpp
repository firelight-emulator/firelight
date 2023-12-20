//
// Created by alexs on 12/20/2023.
//

#ifndef FIRELIGHT_EMULATION_MANAGER_HPP
#define FIRELIGHT_EMULATION_MANAGER_HPP

#include "libretro/core.hpp"
#include <QImage>
#include <QObject>

class EmulationManager : public QObject, public CoreVideoDataReceiver {
  Q_OBJECT

public:
  static EmulationManager *getInstance();
  void initialize();
  void receive(const void *data, unsigned int width, unsigned int height,
               size_t pitch) override;
  QImage *getImage();

public slots:
  void runOneFrame();

private:
  //  FL::GameLoopMetrics loopMetrics;
  Uint64 frameBegin;
  Uint64 frameEnd;
  double frameDiff;
  float framerate = 60; // Shouldn't need a default, but eh.
  int dumb = 0;
  void *m_data;
  unsigned m_width;
  unsigned m_height;
  size_t m_pitch;
  boolean running = false;
  Uint64 thisTick;
  Uint64 lastTick;
  std::unique_ptr<libretro::Core> core;
  FL::Input::ControllerManager conManager;
  double totalFrameWorkDurationMillis = 0;
  long numFrames = 0;
};

#endif // FIRELIGHT_EMULATION_MANAGER_HPP
