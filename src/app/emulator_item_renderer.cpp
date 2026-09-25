// TODO: NEEDS REVIEW
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
#include <QScopeGuard>
#include <QVulkanDeviceFunctions>
#include <QVulkanFunctions>
#include <libretro/libretro_vulkan.h>
#include <rcheevos/ra_client.hpp>
#include <rhi/qrhi.h>
#include <rhi/qrhi_platform.h>
#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#endif
#include "emulation/emulation_service.hpp"
#include "emulation/pace_probe.hpp"
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
                                           void *windowHandle,
                                           std::weak_ptr<firelight::emulation::EmulatorInstance> emulatorInstance,
                                           firelight::activity::IActivityLog *activityLog,
                                           firelight::achievements::RAClient *achievementManager,
                                           firelight::gui::GameImageProvider *gameImageProvider,
                                           firelight::saves::ISaveManager *saveManager,
                                           firelight::media::MediaService *mediaService)
    : m_window(window), m_windowHandle(windowHandle), m_graphicsApi(api),
      m_instanceHandle(std::move(emulatorInstance)), m_activityLog(activityLog),
      m_achievementManager(achievementManager), m_gameImageProvider(gameImageProvider), m_saveManager(saveManager),
      m_mediaService(mediaService) {
  globalRenderer = this;
  m_clipRecorder = std::make_unique<firelight::media::ClipRecorder>();
  m_presenter = makeFramePresenter(api);
}

EmulatorItemRenderer::~EmulatorItemRenderer() {
  m_quitting = true;

  if (const auto instance = m_instanceHandle.lock()) {
    instance->setCommandSink(nullptr);
    instance->setThumbnailProvider(nullptr);
    instance->setFrameRestorer(nullptr);
  }

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

  if (m_vulkanCore) {
    m_vulkanCore->destroy();
  }

  if (m_presenter) {
    m_presenter->destroy();
  }
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// IVideoDataReceiver â€” general
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

retro_hw_context_type EmulatorItemRenderer::getPreferredHwRender() {
  if (m_presenter) {
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
  // Resets the totals only when the timing changed, not on a geometry-only announcement
  const auto timingChanged = info->timing.fps != m_announcedFps;
  m_announcedFps = info->timing.fps;

  if (timingChanged) {
    firelight::diagnostics::PerformanceStats::instance().reset();
  }

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

  if (!m_presenter) {
    return;
  }

  m_usingHardwareRenderer = true;
  m_vulkanCore = std::make_unique<VulkanCoreContext>();

  // Store the interface for the Vulkan renderer to use later when initializing the context
  // This pointer is owned by the core and should not be freed by us
  m_negotiation = reinterpret_cast<const retro_hw_render_context_negotiation_interface_vulkan *>(iface);

  spdlog::info("Stored Vulkan context negotiation interface (version {})", m_negotiation->interface_version);
}

void EmulatorItemRenderer::setHwRenderInterface(retro_hw_render_callback *iface) {
  // I believe this is only used for OpenGL... need to confirm. Vulkan uses the negotiation interface instead

  m_usingHardwareRenderer = true;

  // Store reset/destroy for all APIs
  m_resetContextFunction = iface->context_reset;
  m_destroyContextFunction = iface->context_destroy;

  if (m_graphicsApi == QSGRendererInterface::OpenGL) {
    iface->get_proc_address = [](const char *sym) -> retro_proc_address_t {
      return globalRenderer->getProcAddress(sym);
    };
    iface->get_current_framebuffer = [] { return globalRenderer->getCurrentFramebufferId(); };
  } else {
    // Vulkan cores must not call these; provide safe stubs
    iface->get_current_framebuffer = []() -> uintptr_t { return 0; };
    iface->get_proc_address = nullptr;
  }
}

void EmulatorItemRenderer::getHwRenderInterface(retro_hw_render_interface **iface) {
  // We expect this to be called after the core sets the context negotiation interface
  if (!m_vulkanCore) {
    spdlog::error("Vulkan renderer not initialized; cannot set HW render interface");
    return;
  }

  *reinterpret_cast<retro_hw_render_interface_vulkan **>(iface) = m_vulkanCore->hwRenderInterface();
}

// IVideoDataReceiver - per-frame video

void EmulatorItemRenderer::receive(const void *data, const unsigned width, const unsigned height, const size_t pitch) {
  if (data == RETRO_HW_FRAME_BUFFER_VALID) {
    // Vulkan: m_coreImage already set by set_image() earlier this frame
    // Record the actual render dimensions so synchronize() can resize colorTexture to match
    if (m_vulkanCore) {
      m_vulkanCore->setRenderDimensions(width, height);

      if (m_vulkanCore->blitLatest()) {
        noteFramePublished();
      } else {
        m_frameNoPictureMarker.mark();
      }
    }

    return;
  }

  if (!data) {
    m_frameNoPictureMarker.mark();
    return;
  }

  if (width > 0 && height > 0 && pitch > 0) {
    QImage image(static_cast<const uchar *>(data), width, height, pitch, m_pixelFormat);

    auto newImage = image.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
    if (m_screenRotation != 0) {
      newImage = newImage.transformed(QTransform().rotate(m_screenRotation * 90.0));
    }

    // The frame goes to the slot the right way up, and everything that wants it — this renderer
    // included — reads it from there
    publishFrame(firelight::gui::toVideoFrame(newImage));
    noteFramePublished();
  }
}

void EmulatorItemRenderer::publishFrame(firelight::VideoFrame frame) {
  const auto instance = m_instanceHandle.lock();
  if (!instance) {
    return;
  }

  instance->getFrameSlot().publish(std::move(frame));

  // These want every frame rather than the latest one, so they are fed as frames arrive. Nothing
  // else needs a copy, and making one per frame for nobody is a full-frame allocation a frame
  if (!anyFrameConsumerActive()) {
    return;
  }

  const auto published = instance->getFrameSlot().get();
  if (!published || published->isNull()) {
    return;
  }

  const auto asImage = firelight::gui::toQImage(*published);

  feedClipRecorder(asImage);
  feedNetplayStream(asImage);
}

QImage EmulatorItemRenderer::currentFrameImage() const {
  const auto instance = m_instanceHandle.lock();
  if (!instance) {
    return {};
  }

  const auto frame = instance->getFrameSlot().get();

  return frame ? firelight::gui::toQImage(*frame) : QImage();
}

bool EmulatorItemRenderer::uploadCurrentFrame(QRhiResourceUpdateBatch *batch) {
  if (!m_emulatorInstance || batch == nullptr) {
    return false;
  }

  const auto frame = m_emulatorInstance->getFrameSlot().get();
  if (!frame || frame->isNull()) {
    return false;
  }

  // TODO
  // The target already holds this frame
  if (frame->id == m_uploadedFrameId && colorTexture() == m_uploadedTexture &&
      colorTexture()->pixelSize() == m_uploadedSize) {
    return false;
  }

  m_uploadedFrameId = frame->id;
  m_uploadedTexture = colorTexture();
  m_uploadedSize = colorTexture()->pixelSize();

  auto image = firelight::gui::toQImage(*frame);
  // OpenGL's default framebuffer is bottom-up, so what the slot holds the right way up has to go
  // to the texture upside down
  if (m_graphicsApi == QSGRendererInterface::OpenGL) {
    image.flip(Qt::Vertical);
  }

  batch->uploadTexture(colorTexture(), image);
  return true;
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
  const auto instance = m_instanceHandle.lock();
  if (m_captureNextFrame) {
    return true;
  }
  if (!instance) {
    return false;
  }
  if (instance->getInstantReplayEnabled()) {
    return true;
  }
  if (auto *sink = instance->getNetplayStreamSink()) {
    return sink->wantsFrames();
  }
  return false;
}

bool EmulatorItemRenderer::deferCaptureUntilFrameReady(const EmulatorCommand &command) {
  const auto instance = m_instanceHandle.lock();
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
  if (instance) {
    instance->submitCommand(deferred);
  }
  return true;
}

// Same core-frame pts scheme as the clip recorder; the sink no-ops unless a
// host stream is armed
void EmulatorItemRenderer::feedNetplayStream(const QImage &frame) {
  const auto instance = m_instanceHandle.lock();
  if (!instance) {
    return;
  }
  auto *sink = instance->getNetplayStreamSink();
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
  const auto instance = m_instanceHandle.lock();
  if (!m_clipRecorder) {
    return;
  }

  // Gated by the "instant-replay-enabled" setting (resolved on the instance)
  // When off, tear down the recorder so it isn't burning CPU encoding
  if (!instance || !instance->getInstantReplayEnabled()) {
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

  m_uploadedFrameId = 0;
  m_uploadedTexture = nullptr;

  // TODO
  // What a hardware-rendered core needs to know about the display: captured once here, used from the
  // thread that brings the core up
  if (m_presenter && !m_presenter->initialize(rhi(), m_windowHandle, m_hostHandles)) {
    spdlog::warn("EmulatorItemRenderer: the display cannot show a hardware-rendered core");
  }

  m_hostReady.store(true, std::memory_order_release);

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
  const firelight::monitoring::ScopedSpan syncSpan(m_syncSpan);
  const auto emulatorItem = dynamic_cast<EmulatorItem *>(item);
  if (!emulatorItem) {
    return;
  }

  m_emulatorItem = emulatorItem;

  // TODO
  // Held for this pass only, so the instance can go away between passes
  m_emulatorInstance = m_instanceHandle.lock();
  const auto releaseInstance = qScopeGuard([this] { m_emulatorInstance.reset(); });

  if (firelight::emulation::PaceProbe::isEnabled()) {
    firelight::emulation::PaceProbe::instance().syncs.fetch_add(1);
  }

  if (m_emulatorInstance && !m_hooksInstalled) {
    m_hooksInstalled = true;
    m_emulatorInstance->setCommandSink(
        [this](const firelight::emulation::EmulatorCommand &command) { enqueueCommand(command); });
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
      if (const auto instance = m_instanceHandle.lock(); !restored.isNull() && instance) {
        instance->getFrameSlot().publish(firelight::gui::toVideoFrame(restored));
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
  if (m_vulkanCore) {
    const uint32_t pendingW = m_vulkanCore->pendingWidth();
    const uint32_t pendingH = m_vulkanCore->pendingHeight();
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
  // The commands the emulation thread handed on run here, with the GUI blocked
  if (m_emulatorInstance && m_emulatorInstance->isInitialized()) {
    if (m_playSession.startedAt == 0) {
      m_playSession.contentHash = m_contentHash.toStdString();
      m_playSession.startedAt = QDateTime::currentMSecsSinceEpoch();
      m_playSession.saveSlot = m_saveSlotNumber;

      if (!m_paused) {
        m_playSessionTimer.start();
      }
    }

    {
      const firelight::monitoring::ScopedSpan drainSpan(m_drainCommandsSpan);
      drainOwnCommands();
    }

    // TODO
    // The GUI's undo affordance follows what the emulator actually has to undo. This is the one
    // moment the item can be written from here, because synchronize() runs with the GUI blocked
    if (const auto canUndo = m_emulatorInstance->canUndoLoadSuspendPoint();
        canUndo != emulatorItem->m_canUndoLoadSuspendPoint) {
      emulatorItem->m_canUndoLoadSuspendPoint = canUndo;
      emit emulatorItem->canUndoLoadSuspendPointChanged();
      EventDispatcher::instance().publish(firelight::emulation::UndoLoadSuspendPointChangedEvent{.available = canUndo});
    }
  }
}

void EmulatorItemRenderer::enqueueCommand(const EmulatorCommand &command) {
  std::lock_guard lock(m_pendingCommandsMutex);
  m_pendingCommands.push_back(command);
}

void EmulatorItemRenderer::drainOwnCommands() {
  std::deque<EmulatorCommand> pending;

  {
    std::lock_guard lock(m_pendingCommandsMutex);
    pending.swap(m_pendingCommands);
  }

  for (const auto &command : pending) {
    handleCommand(command);
  }
}

void EmulatorItemRenderer::handleCommand(const firelight::emulation::EmulatorCommand &command) {
  using firelight::emulation::EmulatorCommandType;

  if (!m_emulatorInstance || m_quitting) {
    return;
  }

  switch (command.type) {
  case EmulatorCommandType::EmitRewindPoints: {
    for (auto &url : m_rewindImageUrls) {
      m_gameImageProvider->removeImageWithUrl(url);
    }
    m_rewindImageUrls.clear();

    QList<QJsonObject> points;
    const auto now = QDateTime::currentMSecsSinceEpoch();

    for (const auto &point : m_emulatorInstance->getRewindPointPictures()) {
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

  default:
    // Everything else is the emulator's own business and never reaches here
    break;
  }
}

void EmulatorItemRenderer::render(QRhiCommandBuffer *cb) {
  const firelight::monitoring::ScopedSpan passSpan(m_renderPassSpan);
  if (m_quitting) {
    return;
  }

  const auto passStartNs = std::chrono::steady_clock::now().time_since_epoch().count();

  m_emulatorInstance = m_instanceHandle.lock();
  const auto releaseInstance = qScopeGuard([this] { m_emulatorInstance.reset(); });

  if (!m_emulatorInstance || !m_emulatorInstance->isInitialized()) {
    cb->beginPass(renderTarget(), {0, 0, 0, 1}, {1.0f, 0}, nullptr);
    cb->endPass();
    return;
  }

  // TODO
  // Named as the overlay shows it, so it lines up with what another emulator reports for the same
  // machine. The sizes come from the target the frame is drawn into
  {
    const char *apiName = m_graphicsApi == QSGRendererInterface::Vulkan       ? "vulkan"
                          : m_graphicsApi == QSGRendererInterface::OpenGL     ? "opengl"
                          : m_graphicsApi == QSGRendererInterface::Direct3D11 ? "d3d11"
                          : m_graphicsApi == QSGRendererInterface::Metal      ? "metal"
                                                                              : "software";
    const auto target = renderTarget()->pixelSize();
    const auto shared = m_vulkanCore ? m_vulkanCore->getLatestFrame() : SharedFrame{};
    const auto renderWidth = static_cast<int>(shared.width);
    const auto renderHeight = static_cast<int>(shared.height);
    firelight::diagnostics::PerformanceStats::instance().setVideo(
        apiName, renderWidth > 0 ? renderWidth : target.width(), renderHeight > 0 ? renderHeight : target.height());
  }

  QRhiResourceUpdateBatch *batch = rhi()->nextResourceUpdateBatch();
  const auto publishedAtStart = m_framesPublished.load(std::memory_order_acquire);
  m_frameGrabMarker.mark();
  auto shown = showNewestFrame(batch);

  // TODO
  // On Vulkan, a pass that finds nothing new waits for the next frame rather than sampling the game
  // where that swapchain happened to put it: the display then follows the game by one frame
  if (!shown && !m_paused && m_graphicsApi == QSGRendererInterface::Vulkan) {
    std::unique_lock lock(m_frameWaitMutex);
    m_frameArrived.wait_for(lock, std::chrono::nanoseconds(FRAME_WAIT_NS), [this, publishedAtStart] {
      return m_framesPublished.load(std::memory_order_acquire) > publishedAtStart;
    });
    lock.unlock();
    shown = showNewestFrame(batch);
  }

  // TODO
  // A software frame counts as shown only when its id differs from the last one counted
  const auto isNewFrame = shown && (m_vulkanCore || m_uploadedFrameId != m_lastShownFrameId);

  if (isNewFrame) {
    m_frameShownMarker.mark();
    m_lastShownFrameId = m_uploadedFrameId;
  } else if (!m_paused) {
    m_frameRepeatedMarker.mark();
  }

  // TODO
  // Still nothing newer: the frame the target already shows goes up again, so this pass still
  // presents, which is what a pass asked for after a stall is for
  if (!shown) {
    if (m_vulkanCore && m_presenter) {
      m_presenter->showAgain(batch, colorTexture());
    } else {
      m_uploadedFrameId = 0;
      uploadCurrentFrame(batch);
    }
  }

  cb->beginPass(renderTarget(), {0, 0, 0, 1}, {1.0f, 0}, nullptr);
  cb->endPass(batch);

  if (m_emulatorItem) {
    m_emulatorItem->notePassDuration(std::chrono::steady_clock::now().time_since_epoch().count() - passStartNs);
  }
}

bool EmulatorItemRenderer::showNewestFrame(QRhiResourceUpdateBatch *batch) {
  if (m_vulkanCore) {
    const auto shown =
        m_presenter && m_presenter->show(m_vulkanCore->getLatestFrame(), rhi(), batch, colorTexture());

    // Read the composited frame back only when something needs it, so an
    // idle HW core doesn't pay for a per-frame GPU->CPU copy
    if (shown && anyFrameConsumerActive()) {
      const firelight::monitoring::ScopedSpan readbackSpan(m_readbackSpan);
      scheduleFrameReadback(batch);
    }

    return shown;
  }

  const firelight::monitoring::ScopedSpan uploadSpan(m_uploadFrameSpan);
  return uploadCurrentFrame(batch);
}

void EmulatorItemRenderer::noteFramePublished() {
  m_framesPublished.fetch_add(1, std::memory_order_release);
  {
    std::lock_guard lock(m_frameWaitMutex);
  }
  m_frameArrived.notify_all();
}

bool EmulatorItemRenderer::prepareForFrames() {
  if (!m_hostReady.load(std::memory_order_acquire)) {
    return false;
  }

  if (!m_vulkanCore || m_vulkanCore->isInitialized()) {
    return true;
  }

  if (!m_negotiation || m_vulkanFailed) {
    return false;
  }

  if (!m_vulkanCore->initialize(m_hostHandles, m_negotiation, m_resetContextFunction)) {
    spdlog::error("EmulatorItemRenderer: Vulkan initialization failed");
    m_vulkanFailed = true;
    return false;
  }

  m_resetContextFunction = nullptr;
  return true;
}
