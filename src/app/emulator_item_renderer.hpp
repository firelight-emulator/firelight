#pragma once

#include <QElapsedTimer>
#include <QImage>
#include <firelight/activity/activity_log.hpp>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQueue>
#include <QQuickRhiItemRenderer>
#include <QSGRenderNode>
#include <QVideoFrameInput>
#include <firelight/libretro/video_data_receiver.hpp>
#include <firelight/tas/movie_input_provider.hpp> // TAS replay: input_frame + provider
#include <mutex>
#include <qsgrendererinterface.h>
#include <rhi/qrhi.h>

#include "audio/audio_manager.hpp"
#include "emulation/emulator_instance.hpp"
#include "libretro/core.hpp"
#include "libretro/core_configuration.hpp"

#include <libretro/libretro_vulkan.h>
#include <memory>
#include <qchronotimer.h>

#include "emulator_vulkan_renderer.hpp"

class EmulatorItem;

namespace firelight {
  namespace achievements {
    class RAClient;
  }

  namespace gui {
    class GameImageProvider;
  }

  namespace saves {
    class ISaveManager;
  }

  namespace media {
    class MediaService;
    class ClipRecorder;
  }
} // namespace firelight

// Threading: created, used, and destroyed on the QML render thread — Qt drives
// initialize()/synchronize()/render() there, which is also where the
// EmulatorCommand queue (incl. RunFrame -> EmulatorInstance) is drained.
// submitCommand() enqueues from the GUI and pacing threads (the QQueue itself is
// not synchronized).
class EmulatorItemRenderer : public QQuickRhiItemRenderer,
                             public QOpenGLFunctions,
                             public firelight::libretro::IVideoDataReceiver {
public:
  EmulatorItemRenderer(
    QSGRendererInterface::GraphicsApi api, QWindow *window,
    firelight::emulation::EmulatorInstance *emulatorInstance,
    firelight::activity::IActivityLog *activityLog,
    firelight::achievements::RAClient *achievementManager,
    firelight::gui::GameImageProvider *gameImageProvider,
    firelight::saves::ISaveManager *saveManager,
    firelight::media::MediaService *mediaService);

  void setHwRenderInterface(retro_hw_render_callback *iface) override;

  void onGeometryChanged(
    const std::function<void(int, int, float, double)> &callback) {
    m_geometryChangedCallback = callback;
  }

  void receive(const void *data, unsigned width, unsigned height,
               size_t pitch) override;

  retro_hw_context_type getPreferredHwRender() override;

  void setSystemAVInfo(retro_system_av_info *info) override;

  void setPixelFormat(retro_pixel_format *format) override;

  void setScreenRotation(unsigned rotation) override;

  void setHwRenderContextNegotiationInterface(
    retro_hw_render_context_negotiation_interface *iface) override;

  void getHwRenderInterface(retro_hw_render_interface **iface) override;

  QByteArray m_gameData;
  QByteArray m_saveData;
  QString m_corePath;
  QString m_contentHash;
  int m_saveSlotNumber;
  QString m_contentPath;
  float m_playbackMultiplier = 1;

  enum EmulatorCommandType {
    WriteRewindPoint,
    EmitRewindPoints,
    LoadRewindPoint,
    WriteSuspendPoint,
    LoadSuspendPoint,
    UndoLoadSuspendPoint,
    SetPlaybackMultiplier,
    CaptureScreenshot,
    CaptureVideoClip,
    RunFrame,
    // TAS: advance exactly one frame even while paused (the render-thread
    // primitive the TAS Studio's frame-step drives). See render()/synchronize().
    TasStepFrame,
    // TAS: start/stop capturing each emulated frame's input into a movie (emitted
    // to the GUI via EmulatorItem::tasFrameRecorded).
    TasStartRecording,
    TasStopRecording,
    // TAS: restore the recording's anchor state and play the recorded movie back
    // through the core (a movie provider drives input; the live controller is
    // suppressed). TasStopReplay ends it early and restores live control.
    TasStartReplay,
    TasStopReplay
  };

  struct EmulatorCommand {
    EmulatorCommandType type;
    int suspendPointIndex;
    int rewindPointIndex;
    float playbackMultiplier;
  };

  void submitCommand(EmulatorCommand command);

protected:
  ~EmulatorItemRenderer() override;

  void initialize(QRhiCommandBuffer *cb) override;

  void synchronize(QQuickRhiItem *item) override;

  void render(QRhiCommandBuffer *cb) override;

private:
  QWindow *m_window = nullptr;
  EmulatorItem *m_emulatorItem = nullptr;
  const QSGRendererInterface::GraphicsApi m_graphicsApi;
  firelight::emulation::EmulatorInstance *m_emulatorInstance;

  // Services, injected by EmulatorItem (which is the ServiceAccessor). The
  // renderer is one level removed from QML, so it takes its dependencies rather
  // than reaching into the locator itself.
  firelight::activity::IActivityLog *m_activityLog;
  firelight::achievements::RAClient *m_achievementManager;
  firelight::gui::GameImageProvider *m_gameImageProvider;
  firelight::saves::ISaveManager *m_saveManager;
  firelight::media::MediaService *m_mediaService;

  // Instant-replay recorder: fed the software-rendered frames in receive(); its
  // rolling window is snapshotted + muxed to mp4 on CaptureVideoClip. Software
  // cores only — HW (Vulkan) cores don't deliver pixels to receive().
  std::unique_ptr<firelight::media::ClipRecorder> m_clipRecorder;
  double m_clipFps = 60.0;
  int m_clipWidth = 0;
  int m_clipHeight = 0;
  int64_t m_clipFrameIndex = 0;

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
  // TAS single-step: when set, render() runs exactly one frame even though the
  // item is paused, then clears the flag (back to the held pause image).
  bool m_tasStepPending = false;
  // TAS recording: while set, each emulated frame's input is captured and emitted
  // to the GUI as tasFrameRecorded(frameIndex, buttons).
  bool m_tasRecording = false;
  uint64_t m_tasRecordFrame = 0;
  // The recorded movie kept render-side (a copy of what was emitted to the GUI) so
  // replay can drive it without shipping the log back across the thread boundary.
  std::vector<firelight::input::InputFrame> m_tasRecordedMovie;
  // The core state at the instant recording started; replay restores it first so the
  // recorded inputs reproduce the same run.
  std::vector<uint8_t> m_tasAnchorState;
  // TAS replay: while set, m_tasMovieProvider drives the core's input and its cursor
  // advances one frame per emulated frame until the movie is exhausted.
  bool m_tasReplaying = false;
  std::unique_ptr<firelight::tas::MovieInputProvider> m_tasMovieProvider;

  QThread m_emulatorThread;
  QChronoTimer m_emulatorTimer{};

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

  std::function<void(int, int, float, double)> m_geometryChangedCallback = nullptr;

  // ── HW render callbacks (all APIs) ──────────────────────────────────────
  std::function<void()> m_resetContextFunction = nullptr;
  std::function<void()> m_destroyContextFunction = nullptr;

  // ── OpenGL ──────────────────────────────────────────────────────────────
  bool m_openGlInitialized = false;
  GLint m_currentFramebufferId = 0;

  proc_address_t getProcAddress(const char *sym);

  uintptr_t getCurrentFramebufferId();

  // ── Common display state ─────────────────────────────────────────────────
  QImage::Format m_pixelFormat = QImage::Format_RGB16;
  unsigned m_screenRotation = 0;
  bool m_paused = false;
  bool m_shouldSave = false;

  uint m_coreBaseWidth = 0;
  uint m_coreBaseHeight = 0;
  uint m_coreMaxWidth = 0;
  uint m_coreMaxHeight = 0;
  float m_coreAspectRatio = 0.0f;
  float m_calculatedAspectRatio = 0.0f;

  void initializeEmulatorInstance(QRhiCommandBuffer *cb);

  void displayPauseImage(QRhiCommandBuffer *cb);

  // Pushes the latest software frame into the instant-replay recorder.
  void feedClipRecorder(const QImage &frame);

  bool m_usingHardwareRenderer = false;


  // ── Vulkan ───────────────────────────────────────────────────────────────
  // Stored here because setHwRenderContextNegotiationInterface() may be called
  // before the first render() where initialize() is invoked.
  const retro_hw_render_context_negotiation_interface_vulkan *m_negotiation = nullptr;

  std::unique_ptr<EmulatorVulkanRenderer> m_vulkanRenderer;

  void destroyHwContext() override {
    if (m_vulkanRenderer)
      m_vulkanRenderer->destroy();
  }
};
