#include "emulator_item_renderer.hpp"
#include <firelight/saves/isave_manager.hpp>
#include <rcheevos/ra_client.hpp>
#include <QJsonObject>

#include "../gui/game_image_provider.hpp"
#include <firelight/media/media_service.hpp>

#include <QOpenGLPaintDevice>
#include <QQuickWindow>
#include <QVulkanDeviceFunctions>
#include <QVulkanFunctions>
#include <libretro/libretro_vulkan.h>
#include <rhi/qrhi.h>
#ifdef _WIN32
#  include <vulkan/vulkan_win32.h>
#endif
#include <spdlog/spdlog.h>

#include "emulator_item.hpp"

static EmulatorItemRenderer *globalRenderer = nullptr;
static QRhi *globalRhi = nullptr;

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// Construction / destruction
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

EmulatorItemRenderer::EmulatorItemRenderer(
  const QSGRendererInterface::GraphicsApi api, QWindow *window,
  firelight::emulation::EmulatorInstance *emulatorInstance,
  firelight::activity::IActivityLog *activityLog,
  firelight::achievements::RAClient *achievementManager,
  firelight::gui::GameImageProvider *gameImageProvider,
  firelight::saves::ISaveManager *saveManager,
  firelight::media::MediaService *mediaService)
  : m_window(window), m_graphicsApi(api),
    m_emulatorInstance(emulatorInstance), m_activityLog(activityLog),
    m_achievementManager(achievementManager),
    m_gameImageProvider(gameImageProvider), m_saveManager(saveManager),
    m_mediaService(mediaService) {
  globalRenderer = this;
}

EmulatorItemRenderer::~EmulatorItemRenderer() {
  m_quitting = true;

  if (!m_paused && m_playSessionTimer.isValid())
    m_playSession.unpausedDurationMillis += m_playSessionTimer.elapsed();

  m_playSession.endTime = QDateTime::currentMSecsSinceEpoch();
  m_activityLog->createPlaySession(m_playSession);

  m_achievementManager->unloadGame();

  for (auto &url: m_rewindImageUrls)
    m_gameImageProvider->removeImageWithUrl(url);
  m_rewindImageUrls.clear();

  if (m_vulkanRenderer) {
    m_vulkanRenderer->destroy();
  }
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// IVideoDataReceiver â€” general
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

retro_hw_context_type EmulatorItemRenderer::getPreferredHwRender() {
  // if (m_graphicsApi == QSGRendererInterface::OpenGL) {
  //   return RETRO_HW_CONTEXT_OPENGL;
  // }
  //
  // if (m_graphicsApi == QSGRendererInterface::Vulkan) {
  //   return RETRO_HW_CONTEXT_VULKAN;
  // }

  return RETRO_HW_CONTEXT_NONE;
}

proc_address_t EmulatorItemRenderer::getProcAddress(const char *sym) {
  if (m_graphicsApi == QSGRendererInterface::OpenGL)
    return QOpenGLContext::currentContext()->getProcAddress(sym);
  return nullptr;
}

uintptr_t EmulatorItemRenderer::getCurrentFramebufferId() {
  return m_currentFramebufferId;
}

void EmulatorItemRenderer::setSystemAVInfo(retro_system_av_info *info) {
  if (!info)
    return;
  m_coreBaseWidth = info->geometry.base_width;
  m_coreBaseHeight = info->geometry.base_height;
  m_coreMaxWidth = info->geometry.max_width;
  m_coreMaxHeight = info->geometry.max_height;
  m_coreAspectRatio = info->geometry.aspect_ratio;
  m_calculatedAspectRatio =
      static_cast<float>(m_coreBaseWidth) / static_cast<float>(m_coreBaseHeight);

  if (m_geometryChangedCallback)
    m_geometryChangedCallback(m_coreBaseWidth, m_coreBaseHeight,
                              m_coreAspectRatio, info->timing.fps);
}

void EmulatorItemRenderer::setPixelFormat(retro_pixel_format *format) {
  switch (*format) {
    case RETRO_PIXEL_FORMAT_XRGB8888: m_pixelFormat = QImage::Format_RGB32;
      break;
    case RETRO_PIXEL_FORMAT_RGB565: m_pixelFormat = QImage::Format_RGB16;
      break;
    default: break;
  }
}

void EmulatorItemRenderer::setScreenRotation(unsigned rotation) {
  m_screenRotation = rotation;
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// IVideoDataReceiver â€” HW render setup
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

void EmulatorItemRenderer::setHwRenderContextNegotiationInterface(
  retro_hw_render_context_negotiation_interface *iface) {
  // Libretro API only defines Vulkan and Unknown, and Unknown is an error, so just check for Vulkan
  if (iface->interface_type != RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_VULKAN) {
    spdlog::warn(
      "Received non-Vulkan context negotiation interface (type {})",
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
  m_negotiation =
      reinterpret_cast<const retro_hw_render_context_negotiation_interface_vulkan *>(
        iface);

  spdlog::info("Stored Vulkan context negotiation interface (version {})",
               m_negotiation->interface_version);
}

void EmulatorItemRenderer::setHwRenderInterface(
  retro_hw_render_callback *iface) {
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
    iface->get_current_framebuffer = [] {
      return globalRenderer->getCurrentFramebufferId();
    };
  } else if (m_graphicsApi == QSGRendererInterface::Vulkan) {
    // Vulkan cores must not call these; provide safe stubs
    iface->get_current_framebuffer = []() -> uintptr_t { return 0; };
    iface->get_proc_address = nullptr;
  }
}

void EmulatorItemRenderer::getHwRenderInterface(
  retro_hw_render_interface **iface) {
  // We expect this to be called after the core sets the context negotiation interface
  if (m_graphicsApi != QSGRendererInterface::Vulkan || !m_vulkanRenderer) {
    spdlog::error("Vulkan renderer not initialized; cannot set HW render interface");
    return;
  }

  *reinterpret_cast<retro_hw_render_interface_vulkan **>(iface) = m_vulkanRenderer->hwRenderInterface();
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// IVideoDataReceiver â€” per-frame video
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

void EmulatorItemRenderer::receive(const void *data, const unsigned width,
                                   const unsigned height, const size_t pitch) {
  if (data == RETRO_HW_FRAME_BUFFER_VALID) {
    // Vulkan: m_coreImage already set by set_image() earlier this frame.
    // Record the actual render dimensions so synchronize() can resize colorTexture to match.
    if (m_vulkanRenderer) {
      m_vulkanRenderer->setRenderDimensions(width, height);
    }

    return;
  }

  if (data && width > 0 && height > 0 && pitch > 0) {
    QImage image(static_cast<const uchar *>(data), width, height, pitch,
                 m_pixelFormat);
    if (m_graphicsApi == QSGRendererInterface::OpenGL)
      image.flip(Qt::Vertical);

    auto newImage = image.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
    if (m_screenRotation != 0)
      newImage = newImage.transformed(QTransform().rotate(m_screenRotation * 90.0));

    m_currentUpdateBatch->uploadTexture(colorTexture(), newImage);
  }
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// QQuickRhiItemRenderer overrides
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

void EmulatorItemRenderer::initialize(QRhiCommandBuffer *cb) {
  if (globalRhi == nullptr) {
    globalRhi = rhi();
  }

  if (m_graphicsApi == QSGRendererInterface::OpenGL) {
    if (!m_openGlInitialized) {
      initializeOpenGLFunctions();
      m_openGlInitialized = true;
    }

    // context_reset for OpenGL â€” must be called inside the GL context,
    // which the QRhi render thread provides here
    if (m_resetContextFunction) {
      QRhiResourceUpdateBatch *batch = rhi()->nextResourceUpdateBatch();
      cb->beginPass(renderTarget(), {0, 0, 0, 0}, {1.0f, 0}, batch,
                    QRhiCommandBuffer::ExternalContent);
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
  if (!emulatorItem)
    return;

  m_emulatorItem = emulatorItem;

  if (m_paused && !emulatorItem->paused()) {
    // Resumed: bring audio back.
    if (m_emulatorInstance)
      m_emulatorInstance->setPaused(false);
    if (m_playSessionTimer.isValid())
      m_playSessionTimer.restart();
    else
      m_playSessionTimer.start();
  } else if (!m_paused && emulatorItem->paused()) {
    // Paused: suspend audio so the queued buffer doesn't keep playing.
    if (m_emulatorInstance)
      m_emulatorInstance->setPaused(true);
    if (m_playSessionTimer.isValid())
      m_playSession.unpausedDurationMillis += m_playSessionTimer.elapsed();
  }

  m_paused = emulatorItem->paused();
  m_contentHash = emulatorItem->m_contentHash;
  m_saveSlotNumber = emulatorItem->m_saveSlotNumber;

  // Apply video-callback render dimensions to colorTexture.
  // synchronize() runs with the main thread blocked, so setFixed* is safe here.
  if (m_vulkanRenderer) {
    const uint32_t pendingW = m_vulkanRenderer->pendingWidth();
    const uint32_t pendingH = m_vulkanRenderer->pendingHeight();
    if (pendingW >= 2 && pendingH >= 2 &&
        (static_cast<int>(pendingW) != emulatorItem->fixedColorBufferWidth() ||
         static_cast<int>(pendingH) != emulatorItem->fixedColorBufferHeight())) {
      spdlog::info("synchronize: resizing colorBuffer {}x{} -> {}x{}",
                   emulatorItem->fixedColorBufferWidth(), emulatorItem->fixedColorBufferHeight(),
                   pendingW, pendingH);
      emulatorItem->setFixedColorBufferWidth(pendingW);
      emulatorItem->setFixedColorBufferHeight(pendingH);
    }
  }

  while (!m_commandQueue.isEmpty()) {
    const auto command = m_commandQueue.dequeue();
    switch (command.type) {
      case RunFrame:
        m_shouldRunFrame = true;
        break;

      case WriteRewindPoint: {
        if (m_paused)
          break;
        SuspendPoint sp;
        sp.state = m_emulatorInstance->serializeState();
        sp.image = m_currentImage;
        sp.timestamp = QDateTime::currentMSecsSinceEpoch();
        sp.retroachievementsState = m_achievementManager->serializeState();
        m_rewindPoints.push_front(sp);
        if (m_rewindPoints.length() > 10)
          m_rewindPoints.pop_back();
      }
      break;

      case EmitRewindPoints: {
        for (auto &url: m_rewindImageUrls)
          m_gameImageProvider->removeImageWithUrl(url);
        m_rewindImageUrls.clear();

        QList<QJsonObject> points;
        auto now = QDateTime::currentMSecsSinceEpoch();
        for (const auto &point: m_rewindPoints) {
          auto t = QDateTime::fromMSecsSinceEpoch(point.timestamp).time();
          auto diff = t.secsTo(QDateTime::fromMSecsSinceEpoch(now).time());
          QJsonObject obj;
          auto url = m_gameImageProvider->setImage(point.image);
          m_rewindImageUrls.append(url);
          obj["image_url"] = url;
          obj["time"] = t.toString();
          obj["ago"] = QString::number(diff) + " seconds ago";
          points.append(obj);
        }

        QJsonObject obj;
        auto url = m_gameImageProvider->setImage(m_currentImage);
        m_rewindImageUrls.append(url);
        obj["image_url"] = url;
        obj["time"] = QDateTime::fromMSecsSinceEpoch(now).time().toString();
        obj["ago"] = "Just now";
        points.prepend(obj);

        emulatorItem->rewindPointsReady(points);
      }
      break;

      case LoadRewindPoint: {
        const auto &point = m_rewindPoints.at(command.rewindPointIndex - 1);
        m_emulatorInstance->deserializeState(point.state);
        if (!point.retroachievementsState.empty())
          m_achievementManager->deserializeState(point.retroachievementsState);
        if (m_paused) {
          m_overlayImage = point.image;
          m_overlayImage.flip(Qt::Vertical);
          m_overlayImage =
              m_overlayImage.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
          m_currentImage = m_overlayImage;
          if (m_graphicsApi == QSGRendererInterface::OpenGL)
            m_currentImage.flip(Qt::Vertical);
        }
      }
      break;

      case WriteSuspendPoint: {
        SuspendPoint sp;
        sp.state = m_emulatorInstance->serializeState();
        sp.retroachievementsState = m_achievementManager->serializeState();
        sp.image = m_currentImage;
        sp.timestamp = QDateTime::currentMSecsSinceEpoch();
        sp.saveSlotNumber = m_saveSlotNumber;
        m_saveManager->writeSuspendPoint(m_contentHash, m_saveSlotNumber,
                                            command.suspendPointIndex, sp);
      }
      break;

      case CaptureScreenshot: {
        // Reuse the current frame image the suspend-point path captures. Copy so
        // the PNG write doesn't race the next frame's readback.
        if (const auto mediaService = m_mediaService;
            mediaService && !m_currentImage.isNull()) {
          mediaService->saveScreenshot(m_contentHash, m_currentImage.copy());
        }
      }
      break;

      case LoadSuspendPoint: {
        const auto point = m_saveManager->readSuspendPoint(
          m_contentHash, m_saveSlotNumber, command.suspendPointIndex);
        if (point.has_value()) {
          SuspendPoint before;
          before.state = m_emulatorInstance->serializeState();
          before.retroachievementsState = m_achievementManager->serializeState();
          before.image = m_currentImage;
          before.timestamp = QDateTime::currentMSecsSinceEpoch();
          before.saveSlotNumber = m_saveSlotNumber;
          m_beforeLastLoadSuspendPoint = before;
          emulatorItem->m_canUndoLoadSuspendPoint = true;
          emulatorItem->canUndoLoadSuspendPointChanged();

          m_emulatorInstance->deserializeState(point->state);
          if (!point->retroachievementsState.empty())
            m_achievementManager->deserializeState(point->retroachievementsState);

          if (m_paused) {
            m_overlayImage = point->image;
            m_overlayImage.flip(Qt::Vertical);
            m_overlayImage =
                m_overlayImage.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
            m_currentImage = m_overlayImage;
            if (m_graphicsApi == QSGRendererInterface::OpenGL)
              m_currentImage.flip(Qt::Vertical);
          }
        }
      }
      break;

      case UndoLoadSuspendPoint: {
        emulatorItem->m_canUndoLoadSuspendPoint = false;
        emulatorItem->canUndoLoadSuspendPointChanged();
        if (m_beforeLastLoadSuspendPoint.state.empty())
          break;

        m_emulatorInstance->deserializeState(m_beforeLastLoadSuspendPoint.state);
        if (!m_beforeLastLoadSuspendPoint.retroachievementsState.empty())
          m_achievementManager->deserializeState(
            m_beforeLastLoadSuspendPoint.retroachievementsState);

        if (m_paused) {
          m_overlayImage = m_beforeLastLoadSuspendPoint.image;
          m_overlayImage.flip(Qt::Vertical);
          m_overlayImage =
              m_overlayImage.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
          m_currentImage = m_overlayImage;
          if (m_graphicsApi == QSGRendererInterface::OpenGL)
            m_currentImage.flip(Qt::Vertical);
        }
        m_beforeLastLoadSuspendPoint = SuspendPoint();
      }
      break;

      case SetPlaybackMultiplier:
        m_playbackMultiplier = command.playbackMultiplier;
        if (m_playbackMultiplier < 1) {
          m_waitFrames = static_cast<int>(1.0 / m_playbackMultiplier);
          m_currentWaitFrames = m_waitFrames;
        } else if (m_playbackMultiplier == 1) {
          m_waitFrames = 0;
          m_currentWaitFrames = 0;
        }
        break;
    }
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

  // If we're using Vulkan and the first frame isn't ready yet, clear to opaque black so colorTexture always has valid content
  if (m_vulkanRenderer && !m_vulkanRenderer->isFirstFrameReady()) {
    cb->beginPass(renderTarget(), {0, 0, 0, 1}, {1.0f, 0}, nullptr);
    cb->endPass();
  }

  // Initialize Vulkan renderer if required. This is deferred until after the emulator is loaded so that
  // m_negotiation is available. Runs on the first frame after load
  if (m_vulkanRenderer && !m_vulkanRenderer->isInitialized()) {
    if (m_negotiation) {
      if (!m_vulkanRenderer->initialize(rhi(), m_negotiation, m_resetContextFunction)) {
        spdlog::error("EmulatorItemRenderer: Vulkan initialization failed");
        return;
      }
      m_resetContextFunction = nullptr;
    }

    update();
    return;
  }

  // If we're paused, display the pause image and skip running a frame
  if (m_paused) {
    displayPauseImage(cb);
    return;
  }

  // The renderer hasn't been told to run a frame yet, so skip running a frame
  if (!m_shouldRunFrame) {
    return;
  }

  if (m_currentWaitFrames > 0) {
    m_currentWaitFrames--;
    return;
  }
  m_currentWaitFrames = m_waitFrames;
  m_shouldRunFrame = false;

  // ------------------------------------------------------------
  // If we made it here, we're going to run at least one frame
  // ------------------------------------------------------------
  emit m_emulatorItem->aboutToRunFrame();

  if (!m_usingHardwareRenderer) {
    QRhiResourceUpdateBatch *batch = rhi()->nextResourceUpdateBatch();
    cb->beginPass(renderTarget(), {0, 0, 0, 0}, {1.0f, 0}, batch,
                  QRhiCommandBuffer::ExternalContent);
    m_currentUpdateBatch = batch;
    cb->beginExternal();
    if (m_playbackMultiplier > 1) {
      for (int i = 0; i < static_cast<int>(m_playbackMultiplier); i++)
        m_emulatorInstance->runFrame();
    } else {
      m_emulatorInstance->runFrame();
    }
    cb->endExternal();

    auto *rbResult = new QRhiReadbackResult;
    rbResult->completed = [this, rbResult] {
      const auto *p = reinterpret_cast<const uchar *>(rbResult->data.constData());
      m_currentImage = QImage(p, rbResult->pixelSize.width(),
                              rbResult->pixelSize.height(),
                              QImage::Format_RGBA8888_Premultiplied);
      if (m_graphicsApi == QSGRendererInterface::OpenGL)
        m_currentImage.flip(Qt::Vertical);
      delete rbResult;
    };
    batch->readBackTexture(QRhiReadbackDescription(colorTexture()), rbResult);

    m_currentUpdateBatch = nullptr;
    cb->endPass(batch);
  } else if (m_vulkanRenderer) {
    m_vulkanRenderer->renderFrame(m_emulatorInstance, m_playbackMultiplier,
                                  colorTexture()->pixelSize(), rhi());

    if (m_vulkanRenderer->isFirstFrameReady() &&
        m_vulkanRenderer->sharedTexture() &&
        m_vulkanRenderer->sharedSemValue() > 0) {
      // Composite the shared image into colorTexture() via a GPU copy.
      // renderFrame() already CPU-waited on the blit fence, so m_sharedImage
      // is guaranteed complete â€” no GPU-side semaphore needed here.
      m_vulkanRenderer->sharedTexture()->createFrom({
        reinterpret_cast<quint64>(m_vulkanRenderer->qtSharedImage()),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      });

      QRhiResourceUpdateBatch *batch = rhi()->nextResourceUpdateBatch();
      batch->copyTexture(colorTexture(), m_vulkanRenderer->sharedTexture());
      cb->beginPass(renderTarget(), {0, 0, 0, 1}, {1.0f, 0}, nullptr);
      cb->endPass(batch);
    } else {
      // No real frame yet so clear to opaque black so colorTexture always has valid content
      cb->beginPass(renderTarget(), {0, 0, 0, 1}, {1.0f, 0}, nullptr);
      cb->endPass();
    }
  }

  update();
}

void EmulatorItemRenderer::initializeEmulatorInstance(QRhiCommandBuffer *cb) {
  QRhiResourceUpdateBatch *batch = rhi()->nextResourceUpdateBatch();
  cb->beginPass(renderTarget(), {0, 0, 0, 0}, {1.0f, 0}, batch,
                QRhiCommandBuffer::ExternalContent);
  cb->beginExternal();
  m_emulatorInstance->initialize(this);
  cb->endExternal();
  cb->endPass(batch);

  m_playSession.contentHash = m_contentHash.toStdString();
  m_playSession.startTime = QDateTime::currentMSecsSinceEpoch();
  m_playSession.slotNumber = m_saveSlotNumber;
  if (!m_paused) {
    m_playSessionTimer.start();
  }
}

void EmulatorItemRenderer::displayPauseImage(QRhiCommandBuffer *cb) {
  if (!m_overlayImage.isNull()) {
    QRhiResourceUpdateBatch *batch = rhi()->nextResourceUpdateBatch();
    cb->beginPass(renderTarget(), {0, 0, 0, 0}, {1.0f, 0}, batch,
                  QRhiCommandBuffer::ExternalContent);
    batch->uploadTexture(colorTexture(), m_overlayImage.copy());
    cb->endPass(batch);
    m_overlayImage = QImage();
  }
}

void EmulatorItemRenderer::submitCommand(const EmulatorCommand command) {
  if (!m_emulatorInstance || m_quitting)
    return;
  m_commandQueue.enqueue(command);
}

// (Vulkan implementation lives in EmulatorVulkanRenderer.)
