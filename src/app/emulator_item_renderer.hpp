// TODO: NEEDS REVIEW
#pragma once

#include "audio/audio_manager.hpp"
#include "emulation/emulator_command.hpp"
#include "emulation/emulator_instance.hpp"
#include "frame_presenter.hpp"
#include "libretro/core.hpp"
#include "libretro/core_configuration.hpp"
#include "vulkan_core_context.hpp"

#include <firelight/activity/activity_log.hpp>
#include <firelight/libretro/video_data_receiver.hpp>
#include <firelight/monitoring/monitor.hpp>
#include <firelight/video_frame.hpp>

#include <QElapsedTimer>
#include <QImage>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQueue>
#include <QQuickRhiItemRenderer>
#include <QSGRenderNode>
#include <QVideoFrameInput>
#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <libretro/libretro_vulkan.h>
#include <memory>
#include <mutex>
#include <qchronotimer.h>
#include <qsgrendererinterface.h>
#include <rhi/qrhi.h>
#include <string>

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
} // namespace media
} // namespace firelight

// TODO
// Threading: created, used, and destroyed on the QML render thread — Qt drives
// initialize()/synchronize()/render() there, and a pass only shows the newest frame. The instance is
// held for the length of a pass. The core's callbacks (receive, setSystemAVInfo, the hardware
// interface) and prepareForFrames() run on the emulation thread. Commands that need the screen
// arrive on the renderer's own queue from that thread and run in synchronize()
class EmulatorItemRenderer : public QQuickRhiItemRenderer,
                             public QOpenGLFunctions,
                             public firelight::libretro::IVideoDataReceiver {
public:
  EmulatorItemRenderer(QSGRendererInterface::GraphicsApi api, QWindow *window, void *windowHandle,
                       std::weak_ptr<firelight::emulation::EmulatorInstance> emulatorInstance,
                       firelight::activity::IActivityLog *activityLog,
                       firelight::achievements::RAClient *achievementManager,
                       firelight::gui::GameImageProvider *gameImageProvider,
                       firelight::saves::ISaveManager *saveManager, firelight::media::MediaService *mediaService);

  void setHwRenderInterface(retro_hw_render_callback *iface) override;

  void onGeometryChanged(const std::function<void(int, int, float, double)> &callback) {
    m_geometryChangedCallback = callback;
  }

  void receive(const void *data, unsigned width, unsigned height, size_t pitch) override;

  retro_hw_context_type getPreferredHwRender() override;

  void setSystemAVInfo(retro_system_av_info *info) override;

  void setPixelFormat(retro_pixel_format *format) override;

  void setScreenRotation(unsigned rotation) override;

  void setHwRenderContextNegotiationInterface(retro_hw_render_context_negotiation_interface *iface) override;

  void getHwRenderInterface(retro_hw_render_interface **iface) override;

  QByteArray m_gameData;
  QByteArray m_saveData;
  QString m_corePath;
  QString m_contentHash;
  int m_saveSlotNumber;
  QString m_contentPath;

  using EmulatorCommand = firelight::emulation::EmulatorCommand;

  /**
   * Takes a command the emulator handed on because it needs a screen. Any thread
   */
  void enqueueCommand(const firelight::emulation::EmulatorCommand &command);

  /**
   * Handles a command the emulator handed on because it needs a screen
   */
  void handleCommand(const firelight::emulation::EmulatorCommand &command);

  /**
   * Makes frame the emulator's current one and hands it to the consumers that want every frame
   */
  void publishFrame(firelight::VideoFrame frame);

  /**
   * @return The emulator's current frame as a QImage, or a null image when there isn't one
   */
  [[nodiscard]] QImage currentFrameImage() const;

  /**
   * Puts the emulator's current frame on colorTexture() when it is not there already
   * @return Whether anything was uploaded
   */
  bool uploadCurrentFrame(QRhiResourceUpdateBatch *batch);

  /**
   * Brings a hardware-rendered core's device up once the display's handles are there
   * @return Whether frames may run. Emulation thread
   */
  bool prepareForFrames();

  /**
   * Whether the display's handles have been captured, which a hardware-rendered core needs before it
   * can be brought up. Any thread
   */
  [[nodiscard]] bool isHostReady() const { return m_hostReady.load(std::memory_order_acquire); }

protected:
  ~EmulatorItemRenderer() override;

  void initialize(QRhiCommandBuffer *cb) override;

  void synchronize(QQuickRhiItem *item) override;

  void render(QRhiCommandBuffer *cb) override;

private:
  QWindow *m_window = nullptr;
  void *m_windowHandle = nullptr;
  EmulatorItem *m_emulatorItem = nullptr;
  const QSGRendererInterface::GraphicsApi m_graphicsApi;

  // TODO
  // Locked into m_emulatorInstance for the length of a pass and released after, so the instance
  // can be destroyed between passes
  std::weak_ptr<firelight::emulation::EmulatorInstance> m_instanceHandle;
  std::shared_ptr<firelight::emulation::EmulatorInstance> m_emulatorInstance;

  std::mutex m_pendingCommandsMutex;
  std::deque<EmulatorCommand> m_pendingCommands;

  // TODO
  // What colorTexture() was last given, so a pass that shows the same frame again uploads nothing
  uint64_t m_uploadedFrameId = 0;
  QRhiTexture *m_uploadedTexture = nullptr;
  QSize m_uploadedSize;

  // TODO
  // The id of the software frame last counted as shown, which initialize() leaves alone
  uint64_t m_lastShownFrameId = 0;

  // Services, injected by EmulatorItem (which is the ServiceAccessor). The
  // renderer is one level removed from QML, so it takes its dependencies rather
  // than reaching into the locator itself
  firelight::activity::IActivityLog *m_activityLog;
  firelight::achievements::RAClient *m_achievementManager;
  firelight::gui::GameImageProvider *m_gameImageProvider;
  firelight::saves::ISaveManager *m_saveManager;
  firelight::media::MediaService *m_mediaService;

  // Instant-replay recorder: fed the software-rendered frames in receive(); its
  // rolling window is snapshotted + muxed to mp4 on CaptureVideoClip. Software
  // cores only — HW (Vulkan) cores don't deliver pixels to receive()
  std::unique_ptr<firelight::media::ClipRecorder> m_clipRecorder;
  std::atomic<double> m_clipFps{60.0};
  double m_announcedFps = 0.0;

  firelight::monitoring::Span m_syncSpan = firelight::monitoring::Monitor::instance().span(
      "sync", "Taking the item's state and the queued commands while the GUI thread is blocked");
  firelight::monitoring::Span m_drainCommandsSpan =
      firelight::monitoring::Monitor::instance().span("drain_commands", "Running the commands queued between frames");
  firelight::monitoring::Span m_renderPassSpan = firelight::monitoring::Monitor::instance().span(
      "render_pass", "One render pass, whether or not a frame ran in it");
  firelight::monitoring::Span m_uploadFrameSpan = firelight::monitoring::Monitor::instance().span(
      "upload_frame", "Uploading a software core's pixels to the target");
  firelight::monitoring::Span m_readbackSpan = firelight::monitoring::Monitor::instance().span(
      "readback", "Scheduling the copy of a hardware core's picture back to the CPU");
  firelight::monitoring::Marker m_frameRequestedMarker =
      firelight::monitoring::Monitor::instance().marker("frame_requested", "The pacer asked for a frame");
  firelight::monitoring::Marker m_frameDroppedMarker = firelight::monitoring::Monitor::instance().marker(
      "frame_dropped", "A frame was asked for with the pass already full, so it was never run");
  firelight::monitoring::Marker m_frameRepeatedMarker = firelight::monitoring::Monitor::instance().marker(
      "frame_repeated", "A pass found nothing newer than what the target already held");
  firelight::monitoring::Marker m_frameShownMarker =
      firelight::monitoring::Monitor::instance().marker("frame_shown", "A pass put a new frame on the target");
  firelight::monitoring::Marker m_frameGrabMarker = firelight::monitoring::Monitor::instance().marker(
      "frame_grab", "A pass took the newest frame there was, whether or not it was new");
  firelight::monitoring::Marker m_frameNoPictureMarker = firelight::monitoring::Monitor::instance().marker(
      "frame_no_picture", "The core finished a frame without handing over a new picture");
  int m_clipWidth = 0;
  int m_clipHeight = 0;
  int64_t m_clipFrameIndex = 0;
  int64_t m_streamFrameIndex = 0;

  QRhiResourceUpdateBatch *m_currentUpdateBatch = nullptr;
  // Forces a single framebuffer readback next frame (idle HW cores)
  bool m_captureNextFrame = false;

  QList<QString> m_rewindImageUrls{};

  QElapsedTimer m_playSessionTimer;
  firelight::activity::PlaySession m_playSession{};

  bool m_quitting = false;

  bool m_hooksInstalled = false;
  bool m_vulkanFailed = false;

  // TODO
  // How long a pass with nothing new waits for the next frame before showing the old one again
  static constexpr int64_t FRAME_WAIT_NS = 20'000'000;

  // TODO
  // Counts frames published, so a pass can wait for one newer than what it found
  std::atomic<uint64_t> m_framesPublished = 0;
  std::mutex m_frameWaitMutex;
  std::condition_variable m_frameArrived;

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

  // TODO
  // Refreshes the last present was held for, copied across in synchronize(). A pass that already
  // overran its slot must not take a second frame and overrun it further
  bool m_shouldSave = false;

  uint m_coreBaseWidth = 0;
  uint m_coreBaseHeight = 0;
  uint m_coreMaxWidth = 0;
  uint m_coreMaxHeight = 0;
  float m_coreAspectRatio = 0.0f;
  float m_calculatedAspectRatio = 0.0f;

  /**
   * Runs every command handed on since the last pass
   */
  void drainOwnCommands();

  /**
   * Wakes a pass waiting for a frame. Emulation thread
   */
  void noteFramePublished();

  /**
   * Puts the newest frame on the target
   * @return Whether it was newer than what the target held
   */
  bool showNewestFrame(QRhiResourceUpdateBatch *batch);

  // Reads the composited colorTexture() back and publishes it as the current frame
  // and feeds it to the clip recorder and netplay stream. Works for software
  // and hardware cores alike, since both fill colorTexture()
  void scheduleFrameReadback(QRhiResourceUpdateBatch *batch);

  // Whether anything needs a per-frame CPU copy right now (instant replay on,
  // host stream armed, or a one-shot capture pending). HW cores skip the
  // readback when nothing does
  [[nodiscard]] bool anyFrameConsumerActive() const;

  // Holds a capture command back one frame so a fresh readback can land first
  // (HW cores that were idle). Returns true if the command was deferred
  bool deferCaptureUntilFrameReady(const EmulatorCommand &command);

  // Pushes the latest frame into the instant-replay recorder
  void feedClipRecorder(const QImage &frame);
  // Pushes the latest frame into the netplay host stream
  void feedNetplayStream(const QImage &frame);

  bool m_usingHardwareRenderer = false;

  // ── Vulkan ───────────────────────────────────────────────────────────────
  // Stored here because setHwRenderContextNegotiationInterface() may be called
  // before the first render() where initialize() is invoked
  const retro_hw_render_context_negotiation_interface_vulkan *m_negotiation = nullptr;

  std::unique_ptr<VulkanCoreContext> m_vulkanCore;

  // TODO
  // Chosen from the backend the window is on; null when that backend cannot show a shared picture
  std::unique_ptr<IFramePresenter> m_presenter;
  VulkanHostHandles m_hostHandles;
  std::atomic<bool> m_hostReady = false;

  void destroyHwContext() override {
    if (m_vulkanCore) {
      m_vulkanCore->destroy();
    }
  }
};
