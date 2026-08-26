#include "emulator_item_renderer.hpp"

#include "../gui/game_image_provider.hpp"
#include "../gui/image_qt.hpp"
#include "diagnostics/performance_stats.hpp"

#include <firelight/media/clip_recorder.hpp>
#include <firelight/media/media_service.hpp>
#include <firelight/saves/isave_manager.hpp>

#include <QJsonObject>
#include <QOpenGLPaintDevice>
#include <QQuickWindow>
#include <QVulkanDeviceFunctions>
#include <QVulkanFunctions>
#include <QVulkanInstance>
#include <libretro/libretro_vulkan.h>
#include <rcheevos/ra_client.hpp>
#include <rhi/qrhi.h>
#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#endif
#include "emulator_item.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <spdlog/spdlog.h>

static EmulatorItemRenderer *globalRenderer = nullptr;
static QRhi *globalRhi = nullptr;

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// Construction / destruction
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

EmulatorItemRenderer::EmulatorItemRenderer(const QSGRendererInterface::GraphicsApi api, QWindow *window,
                                           firelight::emulation::EmulatorInstance *emulatorInstance,
                                           firelight::activity::IActivityLog *activityLog,
                                           firelight::achievements::RAClient *achievementManager,
                                           firelight::gui::GameImageProvider *gameImageProvider,
                                           firelight::saves::ISaveManager *saveManager,
                                           firelight::media::MediaService *mediaService)
    : m_window(window), m_graphicsApi(api), m_emulatorInstance(emulatorInstance), m_activityLog(activityLog),
      m_achievementManager(achievementManager), m_gameImageProvider(gameImageProvider), m_saveManager(saveManager),
      m_mediaService(mediaService) {
  globalRenderer = this;
  m_clipRecorder = std::make_unique<firelight::media::ClipRecorder>();
}

EmulatorItemRenderer::~EmulatorItemRenderer() {
  m_quitting = true;

  if (m_clipRecorder) {
    m_clipRecorder->stop();
  }

  if (!m_paused && m_playSessionTimer.isValid()) {
    m_playSession.unpausedDurationMillis += m_playSessionTimer.elapsed();
  }

  m_playSession.endedAt = QDateTime::currentMSecsSinceEpoch();
  m_activityLog->createPlaySession(m_playSession);

  m_achievementManager->unloadGame();

  for (auto &url : m_rewindImageUrls) {
    m_gameImageProvider->removeImageWithUrl(url);
  }
  m_rewindImageUrls.clear();

  if (m_vulkanRenderer) {
    m_vulkanRenderer->destroy();
  }
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// IVideoDataReceiver â€” general
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

retro_hw_context_type EmulatorItemRenderer::getPreferredHwRender() {
  if (m_graphicsApi == QSGRendererInterface::Vulkan) {
    return RETRO_HW_CONTEXT_VULKAN;
  }
  return RETRO_HW_CONTEXT_NONE;
}

proc_address_t EmulatorItemRenderer::getProcAddress(const char *sym) {
  if (m_graphicsApi == QSGRendererInterface::OpenGL) {
    return QOpenGLContext::currentContext()->getProcAddress(sym);
  }
  return nullptr;
}

uintptr_t EmulatorItemRenderer::getCurrentFramebufferId() { return m_currentFramebufferId; }

void EmulatorItemRenderer::setSystemAVInfo(retro_system_av_info *info) {
  if (!info) {
    return;
  }
  m_coreBaseWidth = info->geometry.base_width;
  m_coreBaseHeight = info->geometry.base_height;
  m_coreMaxWidth = info->geometry.max_width;
  m_coreMaxHeight = info->geometry.max_height;
  m_coreAspectRatio = info->geometry.aspect_ratio;
  m_calculatedAspectRatio = static_cast<float>(m_coreBaseWidth) / static_cast<float>(m_coreBaseHeight);
  if (info->timing.fps > 0) {
    m_clipFps = info->timing.fps;
  }

  // TODO
  // Frame and sample totals belong to the game being measured, and this is where a new one announces
  // the geometry it wants
  firelight::diagnostics::PerformanceStats::instance().reset();

  // TODO
  // Reported from here rather than from the item, because the item swaps width and height for a
  // rotated aspect ratio and has no copy of the maximum geometry at all
  firelight::diagnostics::PerformanceStats::instance().setCoreInfo(
      static_cast<int>(m_coreBaseWidth), static_cast<int>(m_coreBaseHeight), static_cast<int>(m_coreMaxWidth),
      static_cast<int>(m_coreMaxHeight), m_coreAspectRatio, info->timing.fps);

  if (m_geometryChangedCallback) {
    m_geometryChangedCallback(m_coreBaseWidth, m_coreBaseHeight, m_coreAspectRatio, info->timing.fps);
  }
}

void EmulatorItemRenderer::setPixelFormat(retro_pixel_format *format) {
  switch (*format) {
  case RETRO_PIXEL_FORMAT_XRGB8888:
    m_pixelFormat = QImage::Format_RGB32;
    break;
  case RETRO_PIXEL_FORMAT_RGB565:
    m_pixelFormat = QImage::Format_RGB16;
    break;
  default:
    break;
  }
}

void EmulatorItemRenderer::setScreenRotation(unsigned rotation) { m_screenRotation = rotation; }

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// IVideoDataReceiver â€” HW render setup
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

void EmulatorItemRenderer::setHwRenderContextNegotiationInterface(
    retro_hw_render_context_negotiation_interface *iface) {
  // Libretro API only defines Vulkan and Unknown, and Unknown is an error, so just check for Vulkan
  if (iface->interface_type != RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_VULKAN) {
    spdlog::warn("Received non-Vulkan context negotiation interface (type {})",
                 static_cast<int>(iface->interface_type));
    return;
  }

  // If we're not running Vulkan then we can't use this interface, so just ignore it
  if (m_graphicsApi != QSGRendererInterface::Vulkan) {
    return;
  }

  m_usingHardwareRenderer = true;
  m_vulkanRenderer = std::make_unique<EmulatorVulkanRenderer>();

  // Store the interface for the Vulkan renderer to use later when initializing the context
  // This pointer is owned by the core and should not be freed by us
  m_negotiation = reinterpret_cast<const retro_hw_render_context_negotiation_interface_vulkan *>(iface);

  spdlog::info("Stored Vulkan context negotiation interface (version {})", m_negotiation->interface_version);
}

void EmulatorItemRenderer::setHwRenderInterface(retro_hw_render_callback *iface) {
  // I believe this is only used for OpenGL... need to confirm. Vulkan uses the negotiation interface instead

  spdlog::info("IS THIS ACTUALLY BEING CALLED? EmulatorItemRenderer::getHwRenderInterface");
  m_usingHardwareRenderer = true;

  // Store reset/destroy for all APIs
  m_resetContextFunction = iface->context_reset;
  m_destroyContextFunction = iface->context_destroy;

  if (m_graphicsApi == QSGRendererInterface::OpenGL) {
    iface->get_proc_address = [](const char *sym) -> retro_proc_address_t {
      return globalRenderer->getProcAddress(sym);
    };
    iface->get_current_framebuffer = [] { return globalRenderer->getCurrentFramebufferId(); };
  } else if (m_graphicsApi == QSGRendererInterface::Vulkan) {
    // Vulkan cores must not call these; provide safe stubs
    iface->get_current_framebuffer = []() -> uintptr_t { return 0; };
    iface->get_proc_address = nullptr;
  }
}

void EmulatorItemRenderer::getHwRenderInterface(retro_hw_render_interface **iface) {
  // We expect this to be called after the core sets the context negotiation interface
  if (m_graphicsApi != QSGRendererInterface::Vulkan || !m_vulkanRenderer) {
    spdlog::error("Vulkan renderer not initialized; cannot set HW render interface");
    return;
  }

  *reinterpret_cast<retro_hw_render_interface_vulkan **>(iface) = m_vulkanRenderer->hwRenderInterface();
}

// IVideoDataReceiver - per-frame video

void EmulatorItemRenderer::receive(const void *data, const unsigned width, const unsigned height, const size_t pitch) {
  if (data == RETRO_HW_FRAME_BUFFER_VALID) {
    // Vulkan: m_coreImage already set by set_image() earlier this frame
    // Record the actual render dimensions so synchronize() can resize colorTexture to match
    if (m_vulkanRenderer) {
      m_vulkanRenderer->setRenderDimensions(width, height);
    }

    return;
  }

  if (data && width > 0 && height > 0 && pitch > 0) {
    QImage image(static_cast<const uchar *>(data), width, height, pitch, m_pixelFormat);

    auto newImage = image.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
    if (m_screenRotation != 0) {
      newImage = newImage.transformed(QTransform().rotate(m_screenRotation * 90.0));
    }

    // The frame goes to the slot the right way up, and everything that wants it — this renderer
    // included — reads it from there
    publishFrame(firelight::gui::toVideoFrame(newImage));
  }
}

void EmulatorItemRenderer::publishFrame(firelight::VideoFrame frame) {
  if (!m_emulatorInstance) {
    return;
  }

  m_emulatorInstance->getFrameSlot().publish(std::move(frame));

  // These want every frame rather than the latest one, so they are fed as frames arrive. Nothing
  // else needs a copy, and making one per frame for nobody is a full-frame allocation a frame
  if (!anyFrameConsumerActive()) {
    return;
  }

  const auto published = m_emulatorInstance->getFrameSlot().get();
  if (!published || published->isNull()) {
    return;
  }

  const auto asImage = firelight::gui::toQImage(*published);

  feedClipRecorder(asImage);
  feedNetplayStream(asImage);
}

QImage EmulatorItemRenderer::currentFrameImage() const {
  if (!m_emulatorInstance) {
    return {};
  }

  const auto frame = m_emulatorInstance->getFrameSlot().get();

  return frame ? firelight::gui::toQImage(*frame) : QImage();
}

void EmulatorItemRenderer::uploadCurrentFrame(QRhiResourceUpdateBatch *batch) {
  if (!m_emulatorInstance || batch == nullptr) {
    return;
  }

  const auto frame = m_emulatorInstance->getFrameSlot().get();
  if (!frame || frame->isNull()) {
    return;
  }

  auto image = firelight::gui::toQImage(*frame);
  // OpenGL's default framebuffer is bottom-up, so what the slot holds the right way up has to go
  // to the texture upside down
  if (m_graphicsApi == QSGRendererInterface::OpenGL) {
    image.flip(Qt::Vertical);
  }

  batch->uploadTexture(colorTexture(), image);
}

// Reads the composited frame back off the GPU and fans it out to every CPU-side
// capture consumer. colorTexture() is filled by both the software path
// (uploadTexture) and the hardware path (copyTexture), so this is the one place
// frames are captured regardless of how the core rendered
void EmulatorItemRenderer::scheduleFrameReadback(QRhiResourceUpdateBatch *batch) {
  auto *rbResult = new QRhiReadbackResult;
  rbResult->completed = [this, rbResult] {
    if (!rbResult->data.isEmpty()) {
      const auto *pixels = reinterpret_cast<const uchar *>(rbResult->data.constData());
      // Own the pixels: the readback buffer is freed when this callback returns
      QImage frame = QImage(pixels, rbResult->pixelSize.width(), rbResult->pixelSize.height(),
                            QImage::Format_RGBA8888_Premultiplied)
                         .copy();
      // OpenGL's default framebuffer is bottom-up
      if (m_graphicsApi == QSGRendererInterface::OpenGL) {
        frame.flip(Qt::Vertical);
      }
      publishFrame(firelight::gui::toVideoFrame(frame));
    }
    delete rbResult;
  };
  batch->readBackTexture(QRhiReadbackDescription(colorTexture()), rbResult);
  m_captureNextFrame = false;
}

bool EmulatorItemRenderer::anyFrameConsumerActive() const {
  if (m_captureNextFrame) {
    return true;
  }
  if (!m_emulatorInstance) {
    return false;
  }
  if (m_emulatorInstance->getInstantReplayEnabled()) {
    return true;
  }
  if (auto *sink = m_emulatorInstance->getNetplayStreamSink()) {
    return sink->wantsFrames();
  }
  return false;
}

bool EmulatorItemRenderer::deferCaptureUntilFrameReady(const EmulatorCommand &command) {
  // Only HW cores idle enough to skip readback need this; software cores and
  // active HW cores already have a fresh frame in the slot. Paused cores never run
  // a frame, so deferring would never resolve — capture the pause image instead
  if (!m_usingHardwareRenderer || m_paused || command.deferred || anyFrameConsumerActive()) {
    return false;
  }
  m_captureNextFrame = true;
  auto deferred = command;
  deferred.deferred = true;
  // Back onto the emulator's queue, which the next drain picks up — a frame later, by which time a
  // readback has happened
  m_emulatorInstance->submitCommand(deferred);
  return true;
}

// Same core-frame pts scheme as the clip recorder; the sink no-ops unless a
// host stream is armed
void EmulatorItemRenderer::feedNetplayStream(const QImage &frame) {
  if (!m_emulatorInstance) {
    return;
  }
  auto *sink = m_emulatorInstance->getNetplayStreamSink();
  if (!sink || !sink->wantsFrames()) {
    return;
  }
  const int fps = m_clipFps >= 1.0 ? static_cast<int>(m_clipFps + 0.5) : 60;
  sink->pushVideoFrame(frame, m_streamFrameIndex * 1000 / fps);
  m_streamFrameIndex++;
}

// Keeps the rolling instant-replay window fed with the latest frame. newImage is
// a deep copy (from convertToFormat), so ClipRecorder can hand it to its encoder
// worker safely. The pts is core-frame-based (not wall clock), so the window is
// N seconds of gameplay regardless of fast-forward
void EmulatorItemRenderer::feedClipRecorder(const QImage &frame) {
  if (!m_clipRecorder) {
    return;
  }

  // Gated by the "instant-replay-enabled" setting (resolved on the instance)
  // When off, tear down the recorder so it isn't burning CPU encoding
  if (!m_emulatorInstance || !m_emulatorInstance->getInstantReplayEnabled()) {
    if (m_clipRecorder->isRecording()) {
      m_clipRecorder->stop();
      spdlog::info("Clip recorder stopped (instant-replay setting off)");
    }
    return;
  }

  const int width = frame.width();
  const int height = frame.height();
  if (width <= 0 || height <= 0) {
    return;
  }

  const int fps = m_clipFps >= 1.0 ? static_cast<int>(m_clipFps + 0.5) : 60;

  // (Re)start when the source geometry changes (some cores switch resolution)
  if (!m_clipRecorder->isRecording() || width != m_clipWidth || height != m_clipHeight) {
    if (!m_clipRecorder->start(width, height, fps, 48000, 2)) {
      spdlog::warn("Clip recorder failed to start ({}x{}@{}fps)", width, height, fps);
      return;
    }
    spdlog::info("Clip recorder started ({}x{}@{}fps)", width, height, fps);
    m_clipWidth = width;
    m_clipHeight = height;
    m_clipFrameIndex = 0;
  }

  m_clipRecorder->pushVideoFrame(frame, m_clipFrameIndex * 1000 / fps);
  m_clipFrameIndex++;
}

// QQuickRhiItemRenderer overrides

void EmulatorItemRenderer::initialize(QRhiCommandBuffer *cb) {
  if (globalRhi == nullptr) {
    globalRhi = rhi();
  }

  if (m_graphicsApi == QSGRendererInterface::OpenGL) {
    if (!m_openGlInitialized) {
      initializeOpenGLFunctions();
      m_openGlInitialized = true;
    }

    // context_reset for OpenGL, must be called inside the GL context,
    // which the QRhi render thread provides here
    if (m_resetContextFunction) {
      QRhiResourceUpdateBatch *batch = rhi()->nextResourceUpdateBatch();
      cb->beginPass(renderTarget(), {0, 0, 0, 0}, {1.0f, 0}, batch, QRhiCommandBuffer::ExternalContent);
      cb->beginExternal();
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_currentFramebufferId);
      m_resetContextFunction();
      m_resetContextFunction = nullptr;
      cb->endExternal();
      cb->endPass(batch);
    }
  }
}

void EmulatorItemRenderer::synchronize(QQuickRhiItem *item) {
  const auto emulatorItem = dynamic_cast<EmulatorItem *>(item);
  if (!emulatorItem) {
    return;
  }

  m_emulatorItem = emulatorItem;

  if (m_emulatorInstance && !m_hooksInstalled) {
    m_hooksInstalled = true;
    m_emulatorInstance->setCommandSink(
        [this](const firelight::emulation::EmulatorCommand &command) { handleCommand(command); });
    // A rewind point's picture is whatever is on screen, scaled down — which only this side can make
    m_emulatorInstance->setThumbnailProvider([this] {
      auto thumb = currentFrameImage();
      if (thumb.width() > 640) {
        thumb = thumb.scaledToWidth(640, Qt::FastTransformation);
      }
      return firelight::gui::toImage(thumb);
    });
    // Straight into the slot rather than through publishFrame: this is a picture being put back,
    // not a frame the game produced, and the recorders want only the ones it did
    m_emulatorInstance->setFrameRestorer([this](const firelight::Image &image) {
      auto restored = firelight::gui::toQImage(image);
      if (!restored.isNull() && m_emulatorInstance) {
        m_emulatorInstance->getFrameSlot().publish(firelight::gui::toVideoFrame(restored));
      }
    });
  }

  if (m_paused && !emulatorItem->paused()) {
    // Resumed: bring audio back
    if (m_emulatorInstance) {
      m_emulatorInstance->setPaused(false);
    }
    if (m_playSessionTimer.isValid()) {
      m_playSessionTimer.restart();
    } else {
      m_playSessionTimer.start();
    }
  } else if (!m_paused && emulatorItem->paused()) {
    // Paused: suspend audio so the queued buffer doesn't keep playing
    if (m_emulatorInstance) {
      m_emulatorInstance->setPaused(true);
    }
    if (m_playSessionTimer.isValid()) {
      m_playSession.unpausedDurationMillis += m_playSessionTimer.elapsed();
    }
  }

  m_paused = emulatorItem->paused();
  m_contentHash = emulatorItem->m_contentHash;
  m_saveSlotNumber = emulatorItem->m_saveSlotNumber;

  // Apply video-callback render dimensions to colorTexture
  // synchronize() runs with the main thread blocked, so setFixed* is safe here
  if (m_vulkanRenderer) {
    const uint32_t pendingW = m_vulkanRenderer->pendingWidth();
    const uint32_t pendingH = m_vulkanRenderer->pendingHeight();
    if (pendingW >= 2 && pendingH >= 2 &&
        (static_cast<int>(pendingW) != emulatorItem->fixedColorBufferWidth() ||
         static_cast<int>(pendingH) != emulatorItem->fixedColorBufferHeight())) {
      spdlog::info("synchronize: resizing colorBuffer {}x{} -> {}x{}", emulatorItem->fixedColorBufferWidth(),
                   emulatorItem->fixedColorBufferHeight(), pendingW, pendingH);
      emulatorItem->setFixedColorBufferWidth(pendingW);
      emulatorItem->setFixedColorBufferHeight(pendingH);
    }
  }

  // TODO
  // Everything queued runs here, between frames rather than inside one, so a state can't be
  // serialized out of a half-run frame. A RunFrame only raises the flag render() takes
  if (m_emulatorInstance && m_emulatorInstance->isInitialized()) {
    m_emulatorInstance->drainCommands();

    // TODO
    // The GUI's undo affordance follows what the emulator actually has to undo. This is the one
    // moment the item can be written from here, because synchronize() runs with the GUI blocked
    if (const auto canUndo = m_emulatorInstance->canUndoLoadSuspendPoint();
        canUndo != emulatorItem->m_canUndoLoadSuspendPoint) {
      emulatorItem->m_canUndoLoadSuspendPoint = canUndo;
      emit emulatorItem->canUndoLoadSuspendPointChanged();
    }
  }
}

namespace {
// FLPACE (temporary instrumentation — remove with its two call sites)
// Frames asked for against frames actually run. Any gap is time the game never got, and it is never
// made up — measured at about one percent before m_framesToRun became a count
std::atomic<int> paceRequested{0};
std::atomic<int> paceRan{0};
std::atomic<int64_t> paceReportAtNs{0};

int64_t paceIntervalNs() {
  const auto secs = qEnvironmentVariableIntValue("FL_DIAG_SECS");
  return (secs > 0 ? secs : 10) * 1000000000LL;
}

void paceReport() {
  const auto nowNs = std::chrono::steady_clock::now().time_since_epoch().count();
  auto due = paceReportAtNs.load();

  if (due == 0) {
    paceReportAtNs.store(nowNs + paceIntervalNs());
    return;
  }

  if (nowNs < due || !paceReportAtNs.compare_exchange_strong(due, nowNs + paceIntervalNs())) {
    return;
  }

  const auto requested = paceRequested.exchange(0);
  const auto ran = paceRan.exchange(0);

  const auto secs = static_cast<double>(paceIntervalNs()) / 1e9;

  spdlog::info("FLPACE requested={} ran={} lost={} ({:.2f}%) -> {:.3f} fps actual", requested, ran, requested - ran,
               requested > 0 ? (requested - ran) * 100.0 / requested : 0.0, ran / secs);
}
} // namespace

void EmulatorItemRenderer::handleCommand(const firelight::emulation::EmulatorCommand &command) {
  using firelight::emulation::EmulatorCommandType;

  if (!m_emulatorInstance || m_quitting) {
    return;
  }

  switch (command.type) {
  case EmulatorCommandType::RunFrame:
    paceRequested.fetch_add(1); // FLPACE (temporary)
    firelight::diagnostics::PerformanceStats::instance().recordFrame(0, 0, 1);
    m_framesToRun = std::min(m_framesToRun + 1, MAX_FRAMES_PER_PASS);
    break;

  case EmulatorCommandType::EmitRewindPoints: {
    for (auto &url : m_rewindImageUrls) {
      m_gameImageProvider->removeImageWithUrl(url);
    }
    m_rewindImageUrls.clear();

    QList<QJsonObject> points;
    const auto now = QDateTime::currentMSecsSinceEpoch();

    for (const auto &point : m_emulatorInstance->getRewindPoints()) {
      const auto t = QDateTime::fromMSecsSinceEpoch(point.timestamp).time();
      const auto diff = t.secsTo(QDateTime::fromMSecsSinceEpoch(now).time());
      QJsonObject obj;
      const auto url = m_gameImageProvider->setImage(firelight::gui::toQImage(point.image));
      m_rewindImageUrls.append(url);
      obj["image_url"] = url;
      obj["time"] = t.toString();
      obj["ago"] = QString::number(diff) + " seconds ago";
      points.append(obj);
    }

    QJsonObject obj;
    obj["image_url"] = m_gameImageProvider->setImage(currentFrameImage());
    m_rewindImageUrls.append(obj["image_url"].toString());
    obj["time"] = QDateTime::fromMSecsSinceEpoch(now).time().toString();
    obj["ago"] = "Just now";
    points.prepend(obj);

    if (m_emulatorItem) {
      m_emulatorItem->rewindPointsReady(points);
    }
  } break;

  case EmulatorCommandType::CaptureScreenshot: {
    if (deferCaptureUntilFrameReady(command)) {
      break;
    }

    const auto screenshot = currentFrameImage();
    if (const auto mediaService = m_mediaService; mediaService && !screenshot.isNull()) {
      mediaService->saveScreenshot(m_contentHash, screenshot);
    }
  } break;

  case EmulatorCommandType::CaptureVideoClip: {
    // Flush the encoder so the snapshot includes the most recent gameplay, then mux the rolling
    // window to an mp4
    if (const auto mediaService = m_mediaService; mediaService && m_clipRecorder) {
      m_clipRecorder->flush();
      const auto snapshot = m_clipRecorder->snapshot();
      spdlog::info("Clip capture requested: recording={}, {} packets, {}x{}", m_clipRecorder->isRecording(),
                   snapshot.video.size(), snapshot.width, snapshot.height);
      if (!snapshot.empty()) {
        mediaService->saveClip(m_contentHash, snapshot);
      } else {
        spdlog::warn("Clip capture: empty window — is instant replay turned "
                     "on, and is this a software-rendered core?");
      }
    } else {
      spdlog::warn("Clip capture: media service or recorder missing");
    }
  } break;

  case EmulatorCommandType::SetPlaybackMultiplier:
    m_playbackMultiplier = command.playbackMultiplier;
    if (m_playbackMultiplier < 1) {
      m_waitFrames = static_cast<int>(1.0 / m_playbackMultiplier);
      m_currentWaitFrames = m_waitFrames;
    } else if (m_playbackMultiplier == 1) {
      m_waitFrames = 0;
      m_currentWaitFrames = 0;
    }
    break;

  default:
    // Everything else is the emulator's own business and never reaches here
    break;
  }
}

void EmulatorItemRenderer::render(QRhiCommandBuffer *cb) {
  if (m_quitting) {
    return;
  }

  if (m_emulatorInstance && !m_emulatorInstance->isInitialized()) {
    initializeEmulatorInstance(cb);

    if (!m_emulatorInstance->isInitialized()) {
      spdlog::error("EmulatorItemRenderer: Emulator instance failed to initialize");
    }
    update();
    return;
  }

  // If we're using Vulkan and the first frame isn't ready yet, clear to opaque black so colorTexture always has valid
  // content
  if (m_vulkanRenderer && !m_vulkanRenderer->isFirstFrameReady()) {
    cb->beginPass(renderTarget(), {0, 0, 0, 1}, {1.0f, 0}, nullptr);
    cb->endPass();
  }

  // Initialize Vulkan renderer if required. This is deferred until after the emulator is loaded so that
  // m_negotiation is available. Runs on the first frame after load
  if (m_vulkanRenderer && !m_vulkanRenderer->isInitialized()) {
    if (m_negotiation) {
      // Some HW cores (PPSSPP) require a real VkSurfaceKHR passed to
      // create_device — they query swapchain capabilities from it and crash on
      // VK_NULL_HANDLE. Others (parallel-RDP) render offscreen and ignore it
      // Hand over the window's surface for both
      VkSurfaceKHR surface = VK_NULL_HANDLE;
      if (auto *inst = m_window ? m_window->vulkanInstance() : nullptr) {
        surface = inst->surfaceForWindow(m_window);
      }
      if (!m_vulkanRenderer->initialize(rhi(), m_negotiation, surface, m_resetContextFunction)) {
        spdlog::error("EmulatorItemRenderer: Vulkan initialization failed");
        return;
      }
      m_resetContextFunction = nullptr;
    }

    update();
    return;
  }

  // TODO
  // If we're paused, display the pause image and skip running a frame — unless a frame was asked
  // for outright, which is what stepping a paused game is
  if (m_paused && m_framesToRun == 0) {
    displayPauseImage(cb);
    return;
  }

  // TODO
  // No frame is due, so there is nothing new to show — leave what is on screen alone
  if (m_framesToRun == 0) {
    return;
  }

  if (m_currentWaitFrames > 0) {
    m_currentWaitFrames--;
    return;
  }
  m_currentWaitFrames = m_waitFrames;

  const auto framesThisPass = m_framesToRun;
  m_framesToRun = 0;

  paceRan.fetch_add(framesThisPass); // FLPACE (temporary)
  paceReport();

  // TODO
  // Named as the overlay shows it, so it lines up with what another emulator reports for the same
  // machine. The sizes come from the target the frame was just drawn into
  {
    const char *apiName = m_graphicsApi == QSGRendererInterface::Vulkan       ? "vulkan"
                          : m_graphicsApi == QSGRendererInterface::OpenGL     ? "opengl"
                          : m_graphicsApi == QSGRendererInterface::Direct3D11 ? "d3d11"
                          : m_graphicsApi == QSGRendererInterface::Metal      ? "metal"
                                                                              : "software";
    const auto target = renderTarget()->pixelSize();
    const auto renderWidth = m_vulkanRenderer ? static_cast<int>(m_vulkanRenderer->sharedImageWidth()) : 0;
    const auto renderHeight = m_vulkanRenderer ? static_cast<int>(m_vulkanRenderer->sharedImageHeight()) : 0;
    firelight::diagnostics::PerformanceStats::instance().setVideo(
        apiName, renderWidth > 0 ? renderWidth : target.width(), renderHeight > 0 ? renderHeight : target.height());
  }

  // TODO
  // Timed here rather than around the core alone, because the gap between passes is what the
  // player experiences and what another emulator's overlay is reporting
  {
    static int64_t lastPassNs = 0;
    const auto nowNs = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto sinceLastNs = lastPassNs > 0 ? nowNs - lastPassNs : 0;
    lastPassNs = nowNs;
    firelight::diagnostics::PerformanceStats::instance().recordFrame(sinceLastNs, framesThisPass, 0);
  }

  // ------------------------------------------------------------
  // If we made it here, we're going to run at least one frame
  // ------------------------------------------------------------
  emit m_emulatorItem->aboutToRunFrame();

  if (!m_usingHardwareRenderer) {
    QRhiResourceUpdateBatch *batch = rhi()->nextResourceUpdateBatch();
    cb->beginPass(renderTarget(), {0, 0, 0, 0}, {1.0f, 0}, batch, QRhiCommandBuffer::ExternalContent);
    m_currentUpdateBatch = batch;
    cb->beginExternal();

    const auto repeats = m_playbackMultiplier > 1 ? static_cast<int>(m_playbackMultiplier) : 1;

    for (auto frame = 0; frame < framesThisPass; ++frame) {
      for (auto repeat = 0; repeat < repeats; ++repeat) {
        m_emulatorInstance->runFrame();
      }
    }

    cb->endExternal();

    // A software core hands over CPU pixels, so the frame is already in the slot — reading it back
    // off the GPU to get a copy we were given would be a round trip for nothing
    uploadCurrentFrame(batch);

    m_currentUpdateBatch = nullptr;
    cb->endPass(batch);
  } else if (m_vulkanRenderer) {
    // renderFrame repeats runFrame() by its multiplier, so the frames owed multiply through it
    m_vulkanRenderer->renderFrame(m_emulatorInstance, m_playbackMultiplier * static_cast<float>(framesThisPass),
                                  colorTexture()->pixelSize(), rhi());

    if (m_vulkanRenderer->isFirstFrameReady() && m_vulkanRenderer->sharedTexture() &&
        m_vulkanRenderer->sharedSemValue() > 0) {
      // Composite the shared image into colorTexture() via a GPU copy
      // renderFrame() already CPU-waited on the blit fence, so m_sharedImage
      // is guaranteed complete â€” no GPU-side semaphore needed here
      m_vulkanRenderer->sharedTexture()->createFrom(
          {reinterpret_cast<quint64>(m_vulkanRenderer->qtSharedImage()), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});

      QRhiResourceUpdateBatch *batch = rhi()->nextResourceUpdateBatch();
      batch->copyTexture(colorTexture(), m_vulkanRenderer->sharedTexture());
      // Read the composited frame back only when something needs it, so an
      // idle HW core doesn't pay for a per-frame GPU->CPU copy
      if (anyFrameConsumerActive()) {
        scheduleFrameReadback(batch);
      }
      cb->beginPass(renderTarget(), {0, 0, 0, 1}, {1.0f, 0}, nullptr);
      cb->endPass(batch);
    } else {
      // No real frame yet so clear to opaque black so colorTexture always has valid content
      cb->beginPass(renderTarget(), {0, 0, 0, 1}, {1.0f, 0}, nullptr);
      cb->endPass();
    }
  }
}

void EmulatorItemRenderer::initializeEmulatorInstance(QRhiCommandBuffer *cb) {
  QRhiResourceUpdateBatch *batch = rhi()->nextResourceUpdateBatch();
  cb->beginPass(renderTarget(), {0, 0, 0, 0}, {1.0f, 0}, batch, QRhiCommandBuffer::ExternalContent);
  cb->beginExternal();
  m_emulatorInstance->initialize(this);
  cb->endExternal();
  cb->endPass(batch);

  m_playSession.contentHash = m_contentHash.toStdString();
  m_playSession.startedAt = QDateTime::currentMSecsSinceEpoch();
  m_playSession.saveSlot = m_saveSlotNumber;
  if (!m_paused) {
    m_playSessionTimer.start();
  }
}

void EmulatorItemRenderer::displayPauseImage(QRhiCommandBuffer *cb) {
  // Whatever is in the slot is what should be on screen — the last live frame, or the picture a
  // rewind point pinned there. Nothing else can have changed it while the emulator is stopped
  QRhiResourceUpdateBatch *batch = rhi()->nextResourceUpdateBatch();
  cb->beginPass(renderTarget(), {0, 0, 0, 0}, {1.0f, 0}, batch, QRhiCommandBuffer::ExternalContent);
  uploadCurrentFrame(batch);
  cb->endPass(batch);
}

// (Vulkan implementation lives in EmulatorVulkanRenderer.)
