//
// Created by alexs on 12/20/2023.
//

#ifndef FIRELIGHT_EMULATION_MANAGER_HPP
#define FIRELIGHT_EMULATION_MANAGER_HPP

#include "library/library_manager.hpp"
#include "libretro/core.hpp"
#include <QImage>
#include <QObject>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSGDynamicTexture>
#include <qopenglcontext.h>

static FL::Library::LibraryManager *library_manager_ = nullptr;

class EmulationManager : public QQuickItem,
                         public CoreVideoDataReceiver,
                         public QOpenGLFunctions {
  Q_OBJECT

  typedef uintptr_t (*get_framebuffer_func)();

public:
  static EmulationManager *getInstance();
  static void setLibraryManager(FL::Library::LibraryManager *manager);

  explicit EmulationManager(QQuickItem *parent = nullptr);
  static void registerInstance(EmulationManager *manager);

  void receive(const void *data, unsigned int width, unsigned int height,
               size_t pitch) override;
  proc_address_t get_proc_address(const char *sym) override;
  void set_reset_context_func(context_reset_func reset) override;
  uintptr_t get_current_framebuffer_id() override;

  std::unique_ptr<QImage> getImage();

public slots:

  void initialize();
  void loadLibraryEntry(int entryId);
  void runOneFrame();
  void pause();
  void resume();

private:
  int current_lib_entry_id_ = -1;
  std::unique_ptr<QSGDynamicTexture> gameTexture = nullptr;
  std::unique_ptr<QOpenGLFramebufferObject> gameFbo = nullptr;
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
  int numSkipFrames = 0;
  int currentSkippedFrames = 0;
};

#endif // FIRELIGHT_EMULATION_MANAGER_HPP
