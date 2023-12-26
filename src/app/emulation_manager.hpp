//
// Created by alexs on 12/20/2023.
//

#ifndef FIRELIGHT_EMULATION_MANAGER_HPP
#define FIRELIGHT_EMULATION_MANAGER_HPP

#include "libretro/FramebufferHandleProvider.hpp"
#include "libretro/core.hpp"
#include <QImage>
#include <QObject>
#include <QOpenGLFunctions>
#include <QQuickWindow>
#include <qopenglcontext.h>

class EmulationManager : public QObject,
                         public CoreVideoDataReceiver,
                         public QOpenGLFunctions {
  Q_OBJECT

  typedef uintptr_t (*get_framebuffer_func)();

public:
  static EmulationManager *getInstance();

  void receive(const void *data, unsigned int width, unsigned int height,
               size_t pitch) override;
  proc_address_t get_proc_address(const char *sym) override;
  void set_reset_context_func(context_reset_func reset) override;
  uintptr_t get_current_framebuffer_id() override;

  void set_framebuffer_thing(FramebufferHandleProvider *provider);

  std::unique_ptr<QImage> getImage();

  void setWindow(QQuickWindow *window);

public slots:
  void initialize();
  void runOneFrame();

private:
  GLuint otherTex;
  GLuint framebufferobject;
  GLuint renderBuffer;
  QQuickWindow *m_window = nullptr;
  FramebufferHandleProvider *framebufferProvider = nullptr;
  context_reset_func reset_context = nullptr;
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
