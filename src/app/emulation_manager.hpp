//
// Created by alexs on 12/20/2023.
//

#ifndef FIRELIGHT_EMULATION_MANAGER_HPP
#define FIRELIGHT_EMULATION_MANAGER_HPP

#include "../gui/QLibraryManager.hpp"
#include "library/library_manager.hpp"
#include "libretro/core.hpp"
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QQuickItem>
#include <QSGDynamicTexture>

static QLibraryManager *library_manager_ = nullptr;

class EmulationManager : public QQuickItem,
                         public IVideoDataReceiver,
                         public QOpenGLFunctions {
  Q_OBJECT

  typedef uintptr_t (*get_framebuffer_func)();

public:
  static EmulationManager *getInstance();
  static void setLibraryManager(QLibraryManager *manager);

  explicit EmulationManager(QQuickItem *parent = nullptr);
  static void registerInstance(EmulationManager *manager);

  void receive(const void *data, unsigned int width, unsigned int height,
               size_t pitch) override;
  proc_address_t get_proc_address(const char *sym) override;
  void set_reset_context_func(context_reset_func reset) override;
  uintptr_t get_current_framebuffer_id() override;
  void set_system_av_info(retro_system_av_info *info) override;

public slots:
  void initialize(int entryId);
  void runOneFrame();
  void pause();
  void resume();

protected:
  QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;

private:
  retro_system_av_info *core_av_info_;
  bool glInitialized = false;
  QSGTexture *gameTexture = nullptr;
  bool usingHwRendering = false;
  std::unique_ptr<QOpenGLFramebufferObject> gameFbo = nullptr;
  context_reset_func reset_context = nullptr;
  bool running = false;
  Uint64 thisTick;
  Uint64 lastTick;
  std::unique_ptr<libretro::Core> core;
  FL::Input::ControllerManager conManager;
  double totalFrameWorkDurationMillis = 0;

  long long int frameCount = 0;
  int frameSkipRatio = 0;
  long numFrames = 0;
};

#endif // FIRELIGHT_EMULATION_MANAGER_HPP
