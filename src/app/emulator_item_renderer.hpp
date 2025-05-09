#pragma once

#include <QImage>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQueue>
#include <QQuickRhiItemRenderer>
#include <QSGRenderNode>
#include <QVideoFrameInput>
#include <firelight/libretro/video_data_receiver.hpp>
#include <qsgrendererinterface.h>
#include <rhi/qrhi.h>

#include "audio/audio_manager.hpp"
#include "libretro/core.hpp"
#include "libretro/core_configuration.hpp"
#include "manager_accessor.hpp"

#include <qchronotimer.h>
// #include "../later/video_decoder.h"
// #include "video_encoder.h"

class EmulatorItem;
class EmulatorItemRenderer : public QQuickRhiItemRenderer,
                             public QOpenGLFunctions,
                             public firelight::libretro::IVideoDataReceiver,
                             public firelight::ManagerAccessor {
public:
  explicit EmulatorItemRenderer(QSGRendererInterface::GraphicsApi api,
                                std::unique_ptr<libretro::Core> core);

  void onGeometryChanged(
      const std::function<void(int, int, float, double)> &callback) {
    m_geometryChangedCallback = callback;
  }

  void receive(const void *data, unsigned width, unsigned height,
               size_t pitch) override;

  retro_hw_context_type getPreferredHwRender() override;

  void getHwRenderContext(retro_hw_context_type &contextType,
                          unsigned int &major, unsigned int &minor) override;

  proc_address_t getProcAddress(const char *sym) override;

  void setResetContextFunc(context_reset_func contextResetFunc) override;

  void setDestroyContextFunc(context_destroy_func contextDestroyFunc) override;

  uintptr_t getCurrentFramebufferId() override;

  void setSystemAVInfo(retro_system_av_info *info) override;

  void setPixelFormat(retro_pixel_format *format) override;

  void setHwRenderContextNegotiationInterface(
      retro_hw_render_context_negotiation_interface *iface) override;

  void setHwRenderInterface(retro_hw_render_interface **iface) override;

  QByteArray m_gameData;
  QByteArray m_saveData;
  QString m_corePath;
  QString m_contentHash;
  int m_saveSlotNumber;
  int m_platformId;
  QString m_contentPath;
  bool m_gameReady;
  int m_playbackMultiplier = 1;

  enum EmulatorCommandType {
    ResetGame,
    WriteSaveFile,
    WriteRewindPoint,
    EmitRewindPoints,
    LoadRewindPoint,
    WriteSuspendPoint,
    LoadSuspendPoint,
    UndoLoadSuspendPoint,
    SetPlaybackMultiplier,
    RunFrame
  };

  struct EmulatorCommand {
    EmulatorCommandType type;
    int suspendPointIndex;
    int rewindPointIndex;
    int playbackMultiplier;
  };

  void submitCommand(EmulatorCommand command);

  void save(bool waitForFinish) const;

protected:
  ~EmulatorItemRenderer() override;

  void initialize(QRhiCommandBuffer *cb) override;

  void synchronize(QQuickRhiItem *item) override;

  void render(QRhiCommandBuffer *cb) override;

private:
  EmulatorItem *m_emulatorItem;
  // firelight::av::VideoEncoder *m_encoder = nullptr;
  // firelight::av::VideoDecoder *m_decoder = nullptr;
  const QSGRendererInterface::GraphicsApi m_graphicsApi;

  QRhiResourceUpdateBatch *m_currentUpdateBatch = nullptr;

  QQueue<EmulatorCommand> m_commandQueue;

  QImage m_overlayImage;
  QImage m_currentImage;

  SuspendPoint m_beforeLastLoadSuspendPoint;

  QList<QString> m_rewindImageUrls{};

  QElapsedTimer m_playSessionTimer;
  firelight::activity::PlaySession m_playSession{};

  bool m_quitting = false;
  bool m_shouldRunFrame = true;

  QElapsedTimer m_emulationTimer{};
  long m_emulationTimingTargetNs = 1000000;
  long m_totalTargetDeviation = 0;
  bool m_runNextFrame = false;

  QElapsedTimer m_renderCallTimer{};
  QList<int64_t> m_renderCallTimes;
  int64_t m_averageTimeBetweenRenderCalls = 0;

  bool m_measureTime = false;
  QThread m_emulationTimerThread{};
  QList<int64_t> m_emulationWorkTimeBuffer{};
  int64_t m_averageEmulationTime = 0;

  int m_frameNumber = 0;

  int m_currentWaitFrames = 0;
  int m_waitFrames = 0;

  QList<SuspendPoint> m_rewindPoints;

  std::unique_ptr<libretro::Core> m_core = nullptr;
  bool m_coreInitialized = false;

  std::function<void(int, int, float, double)> m_geometryChangedCallback =
      nullptr;

  // HW rendering members
  bool m_openGlInitialized = false;
  GLint m_currentFramebufferId = 0;
  std::function<void()> m_resetContextFunction = nullptr;
  std::function<void()> m_destroyContextFunction = nullptr;

  // Default according to libretro docs
  QImage::Format m_pixelFormat = QImage::Format_RGB16;

  bool m_paused = false;

  uint m_coreBaseWidth = 0;
  uint m_coreBaseHeight = 0;
  uint m_coreMaxWidth = 0;
  uint m_coreMaxHeight = 0;
  float m_coreAspectRatio = 0.0f;
  float m_calculatedAspectRatio = 0.0f;

  bool m_shouldSave = false;
};
