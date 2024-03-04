#pragma once

#include "emulation_manager.hpp"
#include "libretro/core.hpp"
#include "manager_accessor.hpp"
#include <QOpenGLFunctions>
#include <QQuickFramebufferObject>

class EmulatorRenderer final : public QQuickFramebufferObject::Renderer,
                               public firelight::libretro::IVideoDataReceiver,
                               public QOpenGLFunctions,
                               public firelight::ManagerAccessor {
public:
  EmulatorRenderer();
  ~EmulatorRenderer() override;

  void receive(const void *data, unsigned width, unsigned height,
               size_t pitch) override;
  proc_address_t get_proc_address(const char *sym) override;
  void set_reset_context_func(context_reset_func) override;
  uintptr_t get_current_framebuffer_id() override;
  void set_system_av_info(retro_system_av_info *info) override;
  void save(bool waitForFinish = false);

protected:
  void synchronize(QQuickFramebufferObject *fbo) override;
  QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
  void render() override;

private:
  long long sessionStartTime = 0;
  long long sessionEndTime = 0;
  long long sessionDuration = 0;
  QElapsedTimer m_playtimeTimer;

  bool m_shouldLoadGame = false;
  bool m_shouldStartEmulation = false;
  bool m_shouldStopEmulation = false;

  bool m_running = false;
  bool m_paused = true;

  bool m_ranLastFrame = false;

  int m_entryId;
  QByteArray m_gameData;
  QByteArray m_saveData;
  QString m_corePath;

  bool m_shouldShutdown = false;
  bool running = false;
  Uint64 thisTick;
  Uint64 lastTick;
  int m_millisSinceLastSave{};
  bool usingHwRendering = false;
  retro_system_av_info *core_av_info_ = nullptr;
  std::unique_ptr<libretro::Core> core;
  LibEntry m_currentEntry;
  QOpenGLFramebufferObject *m_fbo = nullptr;
  context_reset_func reset_context = nullptr;
  double totalFrameWorkDurationMillis = 0;

  long long int frameCount = 0;
  int frameSkipRatio = 0;
  long numFrames = 0;
};
