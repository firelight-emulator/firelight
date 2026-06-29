#include "emulator_item_renderer.hpp"

#include "../gui/game_image_provider.hpp"

#include <QOpenGLPaintDevice>
#include <QQuickWindow>
#include <QVulkanDeviceFunctions>
#include <QVulkanFunctions>
#include <libretro/libretro_vulkan.h>
#include <rhi/qrhi.h>
#ifdef _WIN32
#  include <windows.h>
#  include <vulkan/vulkan_win32.h>
#endif
#include <spdlog/spdlog.h>

#include "emulator_item.hpp"

static EmulatorItemRenderer *globalRenderer = nullptr;
static QRhi *globalRhi = nullptr;

// ─────────────────────────────────────────────────────────────────────────────
// Construction / destruction
// ─────────────────────────────────────────────────────────────────────────────

EmulatorItemRenderer::EmulatorItemRenderer(
  const QSGRendererInterface::GraphicsApi api, QWindow *window,
  firelight::emulation::EmulatorInstance *emulatorInstance)
  : m_window(window), m_graphicsApi(api),
    m_emulatorInstance(emulatorInstance) {
  globalRenderer = this;
}

EmulatorItemRenderer::~EmulatorItemRenderer() {
  m_quitting = true;

  if (!m_paused && m_playSessionTimer.isValid())
    m_playSession.unpausedDurationMillis += m_playSessionTimer.elapsed();

  m_playSession.endTime = QDateTime::currentMSecsSinceEpoch();
  getActivityLog()->createPlaySession(m_playSession);

  getAchievementManager()->unloadGame();

  for (auto &url: m_rewindImageUrls)
    getGameImageProvider()->removeImageWithUrl(url);
  m_rewindImageUrls.clear();

  if (m_graphicsApi == QSGRendererInterface::Vulkan)
    destroyVulkan();
}

// ─────────────────────────────────────────────────────────────────────────────
// IVideoDataReceiver — general
// ─────────────────────────────────────────────────────────────────────────────

retro_hw_context_type EmulatorItemRenderer::getPreferredHwRender() {
  m_usingHardwareRenderer = true;
  spdlog::info("SETTING HW RENDER FLAG TO TRUE IN EmulatorItemRenderer::getPreferredHwRender");
  if (m_graphicsApi == QSGRendererInterface::OpenGL)
    return RETRO_HW_CONTEXT_OPENGL;
  if (m_graphicsApi == QSGRendererInterface::Vulkan) {
    spdlog::info("EmulatorItemRenderer: Preferred HW render is Vulkan");
    return RETRO_HW_CONTEXT_VULKAN;
  }

  return RETRO_HW_CONTEXT_NONE;
}

void EmulatorItemRenderer::getHwRenderContext(retro_hw_context_type &contextType,
                                              unsigned int &major,
                                              unsigned int &minor) {
  m_usingHardwareRenderer = true;
  spdlog::info("SETTING HW RENDER FLAG TO TRUE IN EmulatorItemRenderer::getHwRenderContext");
  if (m_graphicsApi == QSGRendererInterface::Vulkan) {
    contextType = RETRO_HW_CONTEXT_VULKAN;
    major = 1;
    minor = 1;
  } else if (m_graphicsApi == QSGRendererInterface::OpenGL) {
    contextType = RETRO_HW_CONTEXT_OPENGL_CORE;
    major = 3;
    minor = 1;
  } else {
    contextType = RETRO_HW_CONTEXT_NONE;
    major = 0;
    minor = 0;
  }
}

proc_address_t EmulatorItemRenderer::getProcAddress(const char *sym) {
  if (m_graphicsApi == QSGRendererInterface::OpenGL)
    return QOpenGLContext::currentContext()->getProcAddress(sym);
  return nullptr;
}

void EmulatorItemRenderer::setResetContextFunc(
  context_reset_func contextResetFunc) {
  m_resetContextFunction = contextResetFunc;
}

void EmulatorItemRenderer::setDestroyContextFunc(
  context_destroy_func contextDestroyFunc) {
  m_destroyContextFunction = contextDestroyFunc;
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

// ─────────────────────────────────────────────────────────────────────────────
// IVideoDataReceiver — HW render setup
// ─────────────────────────────────────────────────────────────────────────────

void EmulatorItemRenderer::setHwRenderInterface(
  retro_hw_render_callback *iface) {
  m_usingHardwareRenderer = true;
  // Store reset/destroy for all APIs
  setResetContextFunc(iface->context_reset);
  setDestroyContextFunc(iface->context_destroy);

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

void EmulatorItemRenderer::setHwRenderContextNegotiationInterface(
  retro_hw_render_context_negotiation_interface *iface) {
  m_usingHardwareRenderer = true;
  spdlog::info("SETTING HW RENDER FLAG TO TRUE IN EmulatorItemRenderer::setHwRenderContextNegotiationInterface");
  if (m_graphicsApi != QSGRendererInterface::Vulkan)
    return;

  spdlog::info("Received Vulkan context negotiation interface (version {})",
               iface->interface_version);
  // Store the pointer as-is — do NOT modify the struct; the core owns it
  m_negotiation =
      reinterpret_cast<const retro_hw_render_context_negotiation_interface_vulkan *>(
        iface);
  spdlog::info("Stored Vulkan context negotiation interface (version {})",
               m_negotiation->interface_version);
}

void EmulatorItemRenderer::setHwRenderInterface(
  retro_hw_render_interface **iface) {
  m_usingHardwareRenderer = true;
  spdlog::info("SETTING HW RENDER FLAG TO TRUE IN EmulatorItemRenderer::setHwRenderInterface");
  if (m_graphicsApi != QSGRendererInterface::Vulkan)
    return;
  // Called from inside context_reset — return the interface we built in initVulkan()
  *reinterpret_cast<retro_hw_render_interface_vulkan **>(iface) = &m_vkInterface;
}

// ─────────────────────────────────────────────────────────────────────────────
// IVideoDataReceiver — per-frame video
// ─────────────────────────────────────────────────────────────────────────────

void EmulatorItemRenderer::receive(const void *data, unsigned width,
                                   unsigned height, size_t pitch) {
  if (data == RETRO_HW_FRAME_BUFFER_VALID) {
    // Vulkan: m_coreImage already set by set_image() earlier this frame.
    // Record the actual render dimensions so synchronize() can resize colorTexture to match.
    if (width >= 2 && height >= 2) {
      m_pendingColorBufferW = width;
      m_pendingColorBufferH = height;
    }
    m_vkRenderWidth = width;
    m_vkRenderHeight = height;
    m_vkFrameReady = true;
    return;
  }

  if (data && width > 0 && height > 0 && pitch > 0) {
    // Software framebuffer — upload via QRhi (OpenGL path)
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

// ─────────────────────────────────────────────────────────────────────────────
// QQuickRhiItemRenderer overrides
// ─────────────────────────────────────────────────────────────────────────────

void EmulatorItemRenderer::initialize(QRhiCommandBuffer *cb) {
  if (globalRhi == nullptr)
    globalRhi = rhi();

  if (m_graphicsApi == QSGRendererInterface::OpenGL) {
    if (!m_openGlInitialized) {
      initializeOpenGLFunctions();
      m_openGlInitialized = true;
    }

    // context_reset for OpenGL — must be called inside the GL context,
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
    return;
  }

  // Vulkan: initVulkan() is called from render(), not here.
  // initialize() runs once before the first render, before the emulator is
  // loaded, so m_negotiation is always null at that point.
}

void EmulatorItemRenderer::synchronize(QQuickRhiItem *item) {
  const auto emulatorItem = dynamic_cast<EmulatorItem *>(item);
  if (!emulatorItem)
    return;

  m_emulatorItem = emulatorItem;

  if (m_paused && !emulatorItem->paused()) {
    if (m_playSessionTimer.isValid())
      m_playSessionTimer.restart();
    else
      m_playSessionTimer.start();
  } else if (!m_paused && emulatorItem->paused()) {
    if (m_playSessionTimer.isValid())
      m_playSession.unpausedDurationMillis += m_playSessionTimer.elapsed();
  }

  m_paused = emulatorItem->paused();
  m_contentHash = emulatorItem->m_contentHash;
  m_saveSlotNumber = emulatorItem->m_saveSlotNumber;
  m_platformId = emulatorItem->m_platformId;

  // Apply video-callback render dimensions to colorTexture.
  // synchronize() runs with the main thread blocked, so setFixed* is safe here.
  if (m_pendingColorBufferW >= 2 && m_pendingColorBufferH >= 2 &&
      (static_cast<int>(m_pendingColorBufferW) != emulatorItem->fixedColorBufferWidth() ||
       static_cast<int>(m_pendingColorBufferH) != emulatorItem->fixedColorBufferHeight())) {
    spdlog::info("synchronize: resizing colorBuffer {}x{} -> {}x{}",
                 emulatorItem->fixedColorBufferWidth(), emulatorItem->fixedColorBufferHeight(),
                 m_pendingColorBufferW, m_pendingColorBufferH);
    emulatorItem->setFixedColorBufferWidth(m_pendingColorBufferW);
    emulatorItem->setFixedColorBufferHeight(m_pendingColorBufferH);
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
        sp.retroachievementsState = getAchievementManager()->serializeState();
        m_rewindPoints.push_front(sp);
        if (m_rewindPoints.length() > 10)
          m_rewindPoints.pop_back();
      }
      break;

      case EmitRewindPoints: {
        for (auto &url: m_rewindImageUrls)
          getGameImageProvider()->removeImageWithUrl(url);
        m_rewindImageUrls.clear();

        QList<QJsonObject> points;
        auto now = QDateTime::currentMSecsSinceEpoch();
        for (const auto &point: m_rewindPoints) {
          auto t = QDateTime::fromMSecsSinceEpoch(point.timestamp).time();
          auto diff = t.secsTo(QDateTime::fromMSecsSinceEpoch(now).time());
          QJsonObject obj;
          auto url = getGameImageProvider()->setImage(point.image);
          m_rewindImageUrls.append(url);
          obj["image_url"] = url;
          obj["time"] = t.toString();
          obj["ago"] = QString::number(diff) + " seconds ago";
          points.append(obj);
        }

        QJsonObject obj;
        auto url = getGameImageProvider()->setImage(m_currentImage);
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
          getAchievementManager()->deserializeState(point.retroachievementsState);
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
        sp.retroachievementsState = getAchievementManager()->serializeState();
        sp.image = m_currentImage;
        sp.timestamp = QDateTime::currentMSecsSinceEpoch();
        sp.saveSlotNumber = m_saveSlotNumber;
        getSaveManager()->writeSuspendPoint(m_contentHash, m_saveSlotNumber,
                                            command.suspendPointIndex, sp);
      }
      break;

      case LoadSuspendPoint: {
        const auto point = getSaveManager()->readSuspendPoint(
          m_contentHash, m_saveSlotNumber, command.suspendPointIndex);
        if (point.has_value()) {
          SuspendPoint before;
          before.state = m_emulatorInstance->serializeState();
          before.retroachievementsState = getAchievementManager()->serializeState();
          before.image = m_currentImage;
          before.timestamp = QDateTime::currentMSecsSinceEpoch();
          before.saveSlotNumber = m_saveSlotNumber;
          m_beforeLastLoadSuspendPoint = before;
          emulatorItem->m_canUndoLoadSuspendPoint = true;
          emulatorItem->canUndoLoadSuspendPointChanged();

          m_emulatorInstance->deserializeState(point->state);
          if (!point->retroachievementsState.empty())
            getAchievementManager()->deserializeState(point->retroachievementsState);

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
          getAchievementManager()->deserializeState(
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
    update();
    return;
  }

  if (m_graphicsApi == QSGRendererInterface::Vulkan) {
    // For HW cores: clear to black until the first real game frame is blitted.
    // Software cores upload directly to colorTexture so they don't need this.
    if (m_usingHardwareRenderer && !m_firstFrameReady) {
      cb->beginPass(renderTarget(), {0, 0, 0, 1}, {1.0f, 0}, nullptr);
      cb->endPass();
    }

    // initVulkan() is deferred until after the emulator is loaded so that
    // m_negotiation is available. Run it on the first frame after load.
    if (!m_vulkanInitialized && m_usingHardwareRenderer) {
      if (m_negotiation) {
        if (!initVulkan()) {
          spdlog::error("EmulatorItemRenderer: Vulkan initialization failed");
          return;
        }
      }

      update();
      return;
    }
  }

  if (m_paused) {
    displayPauseImage(cb);
    return;
  }

  if (m_emulatorItem) {
    emit m_emulatorItem->aboutToRunFrame();
  }

  // ── Vulkan path ───────────────────────────────────────────────────────────
  if (m_graphicsApi == QSGRendererInterface::Vulkan) {
    if (!m_emulatorInstance || !m_emulatorInstance->isInitialized() ||
        !m_shouldRunFrame)
      return;

    if (m_currentWaitFrames > 0) {
      m_currentWaitFrames--;
      return;
    }
    m_currentWaitFrames = m_waitFrames;
    m_shouldRunFrame = false;

    if (!m_usingHardwareRenderer) {
      // Software core in a Vulkan Qt context: upload raw pixels via QRhi batch.
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
      m_currentUpdateBatch = nullptr;
      cb->endPass(batch);
    } else {
      renderVulkanFrame();

      if (m_firstFrameReady && m_sharedTex && m_sharedSemValue > 0) {
        // Composite the shared image into colorTexture() via a GPU copy.
        // renderVulkanFrame() already CPU-waited on the blit fence, so m_sharedImage
        // is guaranteed complete — no GPU-side semaphore needed here.
        m_sharedTex->createFrom({
          reinterpret_cast<quint64>(m_qtSharedImage),
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        });

        QRhiResourceUpdateBatch *batch = rhi()->nextResourceUpdateBatch();
        batch->copyTexture(colorTexture(), m_sharedTex);
        cb->beginPass(renderTarget(), {0, 0, 0, 1}, {1.0f, 0}, nullptr);
        cb->endPass(batch);
      } else {
        // No real frame yet — clear to opaque black so colorTexture always has valid content.
        cb->beginPass(renderTarget(), {0, 0, 0, 1}, {1.0f, 0}, nullptr);
        cb->endPass();
      }
    }

    update(); // keep the render loop going
    return;
  }

  // ── OpenGL / software path ────────────────────────────────────────────────

  if (!m_paused && m_emulatorInstance && m_emulatorInstance->isInitialized() &&
      m_shouldRunFrame) {
    if (m_currentWaitFrames > 0) {
      m_currentWaitFrames--;
      return;
    }
    m_currentWaitFrames = m_waitFrames;
    m_shouldRunFrame = false;

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

    // Readback for rewind / suspend point preview images
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
  }
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

// ─────────────────────────────────────────────────────────────────────────────
// Vulkan — initialization
// ─────────────────────────────────────────────────────────────────────────────

bool EmulatorItemRenderer::initVulkan() {
  if (!m_usingHardwareRenderer) {
    return false;
  }

  const auto *vk =
      static_cast<const QRhiVulkanNativeHandles *>(rhi()->nativeHandles());

  // Get the raw vkGetInstanceProcAddr directly from the Vulkan loader DLL.
  //
  // Qt's QVulkanInstance::getInstanceProcAddr("vkGetInstanceProcAddr") returns
  // the trampoline that routes through Qt's layer chain (Steam overlay, OBS,
  // etc.).  When Granite creates its own separate VkInstance and then calls
  // volkLoadInstance(granite_instance), volk uses this proc addr to resolve
  // every function pointer.  Qt's layer-chained trampoline may not recognise
  // granite_instance (because the overlay layer was never attached to it) and
  // returns null / crashes for physical-device functions such as
  // vkGetPhysicalDeviceMemoryProperties.
  //
  // The raw loader export (from vulkan-1.dll) correctly dispatches for ANY
  // valid instance, bypassing the Qt-specific layer chain entirely.
  PFN_vkGetInstanceProcAddr globalProcAddr = nullptr;
#ifdef _WIN32
  if (HMODULE vulkanLib = GetModuleHandleA("vulkan-1.dll")) {
    globalProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      GetProcAddress(vulkanLib, "vkGetInstanceProcAddr"));
  }
#endif
  if (!globalProcAddr) {
    // Fallback: ask Qt for its proc addr (works when there are no conflicting
    // implicit layers or when the Vulkan loader is single-instance only).
    globalProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      vk->inst->getInstanceProcAddr("vkGetInstanceProcAddr"));
  }
  if (!globalProcAddr) {
    spdlog::error("EmulatorItemRenderer: could not obtain vkGetInstanceProcAddr");
    return false;
  }

  // ── Create the VkDevice ───────────────────────────────────────────────────
  //
  // Pass Qt's VkInstance so Granite uses it directly rather than creating its
  // own. This keeps the instance consistent between Granite's internal state
  // and the m_vkInterface.instance we hand back to the core — they must match.
  // GPU is left as VK_NULL_HANDLE so Granite still picks the best device.
  //
  // VkPhysicalDeviceFeatures must not be nullptr: parallel_create_device
  // SIMD-copies the features struct without a null guard, causing a crash.
  retro_vulkan_context ctx{};
  VkPhysicalDeviceFeatures features{};

  // Disable implicit Vulkan layers before Granite calls vkCreateInstance.
  // Steam/OBS/Overwolf overlay layers crash or return null function pointers
  // when invoked for a second VkInstance in the same process (they already
  // initialised for Qt's instance and don't handle the second one).
  // VK_LOADER_LAYERS_DISABLE=~implicit~ is honoured by the Vulkan Loader.
  std::string savedLayerDisable; {
    const char *v = getenv("VK_LOADER_LAYERS_DISABLE");
    if (v) savedLayerDisable = v;
  }
  _putenv_s("VK_LOADER_LAYERS_DISABLE", "~implicit~");

  const bool deviceOk = m_negotiation->create_device(
    &ctx, vk->inst->vkInstance(), VK_NULL_HANDLE, VK_NULL_HANDLE,
    globalProcAddr, nullptr, 0, nullptr, 0, &features);

  _putenv_s("VK_LOADER_LAYERS_DISABLE",
            savedLayerDisable.empty() ? "" : savedLayerDisable.c_str());

  if (!deviceOk) {
    spdlog::error("EmulatorItemRenderer: create_device failed");
    return false;
  }

  // Store destroy_device as a bare function pointer now, before the core DLL could
  // be unloaded. m_negotiation itself becomes a dangling pointer after
  // coreLib->unload(), so destroyVulkan() must call m_fnDestroyDevice directly.
  m_fnDestroyDevice = m_negotiation->destroy_device;

  m_vkDevice = ctx.device;
  m_vkQueue = ctx.queue;
  m_vkQueueFamilyIndex = ctx.queue_family_index;

  m_vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
    globalProcAddr(vk->inst->vkInstance(), "vkGetDeviceProcAddr"));

  auto d = [&](const char *name) {
    return m_vkGetDeviceProcAddr(m_vkDevice, name);
  };
  m_vkfQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(d("vkQueueSubmit"));
  m_vkfCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(d("vkCreateCommandPool"));
  m_vkfAllocCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(d("vkAllocateCommandBuffers"));
  m_vkfBeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(d("vkBeginCommandBuffer"));
  m_vkfEndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(d("vkEndCommandBuffer"));
  m_vkfCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(d("vkCmdPipelineBarrier"));
  m_vkfCmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(d("vkCmdCopyImageToBuffer"));
  m_vkfCreateFence = reinterpret_cast<PFN_vkCreateFence>(d("vkCreateFence"));
  m_vkfDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(d("vkDestroyFence"));
  m_vkfWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(d("vkWaitForFences"));
  m_vkfResetFences = reinterpret_cast<PFN_vkResetFences>(d("vkResetFences"));
  m_vkfDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(d("vkDestroyCommandPool"));
  m_vkfDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(d("vkDestroyDevice"));
  m_vkfDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(d("vkDeviceWaitIdle"));
  m_vkfCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(d("vkCreateBuffer"));
  m_vkfDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(d("vkDestroyBuffer"));
  m_vkfAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(d("vkAllocateMemory"));
  m_vkfFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(d("vkFreeMemory"));
  m_vkfBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(d("vkBindBufferMemory"));
  m_vkfGetBufferMemoryReqs = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(d("vkGetBufferMemoryRequirements"));
  m_vkfMapMemory = reinterpret_cast<PFN_vkMapMemory>(d("vkMapMemory"));
  m_vkfUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(d("vkUnmapMemory"));
  m_vkfCmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(d("vkCmdCopyImage"));
  m_vkfCmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(d("vkCmdBlitImage"));
  m_vkfCreateImage = reinterpret_cast<PFN_vkCreateImage>(d("vkCreateImage"));
  m_vkfDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(d("vkDestroyImage"));
  m_vkfBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(d("vkBindImageMemory"));
  m_vkfGetImageMemReqs = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(d("vkGetImageMemoryRequirements"));
  m_vkfCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(d("vkCreateSemaphore"));
  m_vkfDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(d("vkDestroySemaphore"));
#ifdef _WIN32
  m_vkfGetMemWin32Handle = reinterpret_cast<PFN_vkGetMemoryWin32HandleKHR>(d("vkGetMemoryWin32HandleKHR"));
  m_vkfGetSemWin32Handle = reinterpret_cast<PFN_vkGetSemaphoreWin32HandleKHR>(d("vkGetSemaphoreWin32HandleKHR"));
#endif

  // Physical device memory properties — needed by ensureStagingBuffer()
  auto vkGetPhysDevMemProps =
      reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(
        globalProcAddr(vk->inst->vkInstance(), "vkGetPhysicalDeviceMemoryProperties"));
  vkGetPhysDevMemProps(ctx.gpu, &m_vkPhysDevMemProps);

#ifdef _WIN32
  // ── Create Granite-side timeline semaphore (exported for Qt device import) ──
  spdlog::info("EmulatorItemRenderer: setting up shared-image path");
  if (!m_vkfCreateSemaphore || !m_vkfGetSemWin32Handle) {
    spdlog::warn("  Granite missing vkCreateSemaphore/vkGetSemaphoreWin32HandleKHR — CPU readback");
  } else {
    VkSemaphoreTypeCreateInfo semType{VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO};
    semType.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semType.initialValue = 0;

    VkExportSemaphoreCreateInfo exportSem{VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO};
    exportSem.pNext = &semType;
    exportSem.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    VkSemaphoreCreateInfo sci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    sci.pNext = &exportSem;

    spdlog::info("  creating Granite-side timeline semaphore");
    VkResult res = m_vkfCreateSemaphore(m_vkDevice, &sci, nullptr, &m_sharedSemaphore);
    if (res != VK_SUCCESS) {
      spdlog::warn("  vkCreateSemaphore failed ({}) — CPU readback", int(res));
    } else {
      VkSemaphoreGetWin32HandleInfoKHR getHandle{VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR};
      getHandle.semaphore = m_sharedSemaphore;
      getHandle.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
      spdlog::info("  exporting Win32 handle");
      if (m_vkfGetSemWin32Handle(m_vkDevice, &getHandle, &m_sharedSemHandle) != VK_SUCCESS) {
        spdlog::warn("  vkGetSemaphoreWin32HandleKHR failed — CPU readback");
        m_vkfDestroySemaphore(m_vkDevice, m_sharedSemaphore, nullptr);
        m_sharedSemaphore = VK_NULL_HANDLE;
      } else {
        spdlog::info("  Granite semaphore exported OK");
      }
    }
  }

  // ── Get Qt's VkDevice, load its function pointers, import the semaphore ───
  if (m_sharedSemHandle) {
    spdlog::info("  loading Qt device functions");
    const auto *qtNative = static_cast<const QRhiVulkanNativeHandles *>(rhi()->nativeHandles());
    m_qtDevice = qtNative->dev;
    spdlog::info("  Qt VkDevice = {:p}", static_cast<void *>(m_qtDevice));

    // Use do{}while(false) so we can break out on any error without goto
    bool sharedOk = false;
    do {
      HMODULE vulkanLib = GetModuleHandleA("vulkan-1.dll");
      if (!vulkanLib) {
        spdlog::warn("  vulkan-1.dll not found — CPU readback");
        break;
      }

      m_qtGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
        GetProcAddress(vulkanLib, "vkGetDeviceProcAddr"));
      if (!m_qtGetDeviceProcAddr) {
        spdlog::warn("  vkGetDeviceProcAddr missing — CPU readback");
        break;
      }

      auto qd = [&](const char *name) -> PFN_vkVoidFunction {
        return m_qtGetDeviceProcAddr(m_qtDevice, name);
      };
      m_qtfCreateImage = reinterpret_cast<PFN_vkCreateImage>(qd("vkCreateImage"));
      m_qtfDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(qd("vkDestroyImage"));
      m_qtfAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(qd("vkAllocateMemory"));
      m_qtfFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(qd("vkFreeMemory"));
      m_qtfBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(qd("vkBindImageMemory"));
      m_qtfGetImageMemReqs = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(qd("vkGetImageMemoryRequirements"));
      m_qtfCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(qd("vkCreateSemaphore"));
      m_qtfDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(qd("vkDestroySemaphore"));
      m_qtfQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(qd("vkQueueSubmit"));
      m_qtfImportSemWin32Handle = reinterpret_cast<PFN_vkImportSemaphoreWin32HandleKHR>(qd(
        "vkImportSemaphoreWin32HandleKHR"));

      spdlog::info("  Qt fn ptrs: createSem={} importSem={} queueSubmit={}",
                   m_qtfCreateSemaphore ? "ok" : "null",
                   m_qtfImportSemWin32Handle ? "ok" : "null",
                   m_qtfQueueSubmit ? "ok" : "null");

      if (!m_qtfCreateSemaphore || !m_qtfImportSemWin32Handle || !m_qtfQueueSubmit ||
          !m_qtfCreateImage || !m_qtfAllocateMemory || !m_qtfBindImageMemory ||
          !m_qtfGetImageMemReqs) {
        spdlog::warn("  Qt device missing required functions — CPU readback");
        break;
      }

      auto vkGetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(qd("vkGetDeviceQueue"));
      if (!vkGetDeviceQueue) {
        spdlog::warn("  vkGetDeviceQueue null");
        break;
      }
      vkGetDeviceQueue(m_qtDevice, qtNative->gfxQueueFamilyIdx, 0, &m_qtQueue);
      spdlog::info("  Qt queue obtained, family={}", qtNative->gfxQueueFamilyIdx);

      // Create timeline semaphore on Qt's device and import Granite's handle into it
      spdlog::info("  creating Qt-side timeline semaphore");
      VkSemaphoreTypeCreateInfo qtSemType{VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO};
      qtSemType.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
      qtSemType.initialValue = 0;
      VkSemaphoreCreateInfo qtSci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
      qtSci.pNext = &qtSemType;
      VkResult res = m_qtfCreateSemaphore(m_qtDevice, &qtSci, nullptr, &m_qtImportedSem);
      if (res != VK_SUCCESS) {
        spdlog::warn("  Qt vkCreateSemaphore failed ({}) — CPU readback", int(res));
        break;
      }

      spdlog::info("  importing Granite semaphore handle into Qt device");
      VkImportSemaphoreWin32HandleInfoKHR importSem{VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR};
      importSem.semaphore = m_qtImportedSem;
      importSem.flags = 0;
      importSem.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
      importSem.handle = m_sharedSemHandle;
      importSem.name = nullptr;
      res = m_qtfImportSemWin32Handle(m_qtDevice, &importSem);
      if (res != VK_SUCCESS) {
        spdlog::warn("  vkImportSemaphoreWin32HandleKHR failed ({}) — CPU readback", int(res));
        m_qtfDestroySemaphore(m_qtDevice, m_qtImportedSem, nullptr);
        m_qtImportedSem = VK_NULL_HANDLE;
        break;
      }
      spdlog::info("  cross-device semaphore ready — GPU copy path enabled");
      sharedOk = true;
    } while (false);

    if (!sharedOk && m_qtImportedSem != VK_NULL_HANDLE && m_qtfDestroySemaphore) {
      m_qtfDestroySemaphore(m_qtDevice, m_qtImportedSem, nullptr);
      m_qtImportedSem = VK_NULL_HANDLE;
    }
  }
#endif

  createVulkanPerFrameResources();
  buildVulkanHwInterface(vk->inst->vkInstance(), ctx.gpu, globalProcAddr);

  // context_reset: the core calls GET_HW_RENDER_INTERFACE from inside here,
  // which returns &m_vkInterface. Granite then finishes its device setup.
  if (m_resetContextFunction) {
    m_resetContextFunction();
    m_resetContextFunction = nullptr;
  }

  m_vulkanInitialized = true;
  spdlog::info("EmulatorItemRenderer: Vulkan initialized");
  return true;
}

bool EmulatorItemRenderer::ensureStagingBuffer() {
  VkDeviceSize required =
      static_cast<VkDeviceSize>(m_vkRenderWidth) * m_vkRenderHeight * 4;
  if (m_vkStagingBuffer != VK_NULL_HANDLE && m_vkStagingSize >= required)
    return true;

  if (m_vkStagingBuffer != VK_NULL_HANDLE) {
    m_vkfDestroyBuffer(m_vkDevice, m_vkStagingBuffer, nullptr);
    m_vkfFreeMemory(m_vkDevice, m_vkStagingMemory, nullptr);
    m_vkStagingBuffer = VK_NULL_HANDLE;
    m_vkStagingMemory = VK_NULL_HANDLE;
  }
  m_vkStagingSize = required;

  VkBufferCreateInfo bci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bci.size = m_vkStagingSize;
  bci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (m_vkfCreateBuffer(m_vkDevice, &bci, nullptr, &m_vkStagingBuffer) != VK_SUCCESS) {
    spdlog::error("EmulatorItemRenderer: staging buffer creation failed");
    return false;
  }

  VkMemoryRequirements memReqs;
  m_vkfGetBufferMemoryReqs(m_vkDevice, m_vkStagingBuffer, &memReqs);

  uint32_t memType = UINT32_MAX;
  for (uint32_t i = 0; i < m_vkPhysDevMemProps.memoryTypeCount; i++) {
    if (!(memReqs.memoryTypeBits & (1u << i))) continue;
    auto flags = m_vkPhysDevMemProps.memoryTypes[i].propertyFlags;
    if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
        (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
      memType = i;
      break;
    }
  }
  if (memType == UINT32_MAX) {
    spdlog::error("EmulatorItemRenderer: no host-visible+coherent memory type found");
    return false;
  }

  VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  mai.allocationSize = memReqs.size;
  mai.memoryTypeIndex = memType;
  if (m_vkfAllocateMemory(m_vkDevice, &mai, nullptr, &m_vkStagingMemory) != VK_SUCCESS) {
    spdlog::error("EmulatorItemRenderer: staging memory allocation failed");
    return false;
  }

  m_vkfBindBufferMemory(m_vkDevice, m_vkStagingBuffer, m_vkStagingMemory, 0);
  spdlog::info("EmulatorItemRenderer: staging buffer {}x{}", m_vkRenderWidth, m_vkRenderHeight);
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Vulkan — shared image management
// ─────────────────────────────────────────────────────────────────────────────

bool EmulatorItemRenderer::ensureSharedImage() {
#ifdef _WIN32
  // Size the shared image to colorTexture() so the GPU blit covers the output exactly.
  // This way Qt's copyTexture() is always same-size and never wraps/clips.
  const QSize ctSize = colorTexture()->pixelSize();
  const uint32_t wantW = static_cast<uint32_t>(ctSize.width());
  const uint32_t wantH = static_cast<uint32_t>(ctSize.height());

  if (m_sharedImage != VK_NULL_HANDLE &&
      m_sharedImageW == wantW &&
      m_sharedImageH == wantH &&
      m_sharedImageFmt == m_coreImageFormat)
    return true;

  destroySharedImage();

  m_sharedImageW = wantW;
  m_sharedImageH = wantH;
  m_sharedImageFmt = m_coreImageFormat;

  // ── Create image on Granite's device with exportable memory ──────────────
  VkExternalMemoryImageCreateInfo extImg{VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO};
  extImg.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

  VkImageCreateInfo ici{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  ici.pNext = &extImg;
  ici.imageType = VK_IMAGE_TYPE_2D;
  ici.format = m_coreImageFormat;
  ici.extent = {wantW, wantH, 1};
  ici.mipLevels = 1;
  ici.arrayLayers = 1;
  ici.samples = VK_SAMPLE_COUNT_1_BIT;
  ici.tiling = VK_IMAGE_TILING_OPTIMAL;
  ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  if (m_vkfCreateImage(m_vkDevice, &ici, nullptr, &m_sharedImage) != VK_SUCCESS) {
    spdlog::error("ensureSharedImage: Granite-side image creation failed");
    return false;
  }

  VkMemoryRequirements memReqs{};
  m_vkfGetImageMemReqs(m_vkDevice, m_sharedImage, &memReqs);

  uint32_t memType = UINT32_MAX;
  for (uint32_t i = 0; i < m_vkPhysDevMemProps.memoryTypeCount; i++) {
    if (!(memReqs.memoryTypeBits & (1u << i))) continue;
    if (m_vkPhysDevMemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
      memType = i;
      break;
    }
  }
  if (memType == UINT32_MAX) {
    spdlog::error("ensureSharedImage: no device-local memory type found");
    return false;
  }

  VkExportMemoryAllocateInfo exportMem{VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO};
  exportMem.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

  VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  mai.pNext = &exportMem;
  mai.allocationSize = memReqs.size;
  mai.memoryTypeIndex = memType;

  if (m_vkfAllocateMemory(m_vkDevice, &mai, nullptr, &m_sharedImageMem) != VK_SUCCESS) {
    spdlog::error("ensureSharedImage: Granite-side memory allocation failed");
    return false;
  }
  m_vkfBindImageMemory(m_vkDevice, m_sharedImage, m_sharedImageMem, 0);

  VkMemoryGetWin32HandleInfoKHR getHandle{VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR};
  getHandle.memory = m_sharedImageMem;
  getHandle.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
  if (m_vkfGetMemWin32Handle(m_vkDevice, &getHandle, &m_sharedImageHandle) != VK_SUCCESS) {
    spdlog::error("ensureSharedImage: vkGetMemoryWin32HandleKHR failed");
    return false;
  }

  // ── Create corresponding image on Qt's device backed by the imported memory ──
  VkExternalMemoryImageCreateInfo qtExtImg{VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO};
  qtExtImg.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

  VkImageCreateInfo qtIci = ici;
  qtIci.pNext = &qtExtImg;
  qtIci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

  if (m_qtfCreateImage(m_qtDevice, &qtIci, nullptr, &m_qtSharedImage) != VK_SUCCESS) {
    spdlog::error("ensureSharedImage: Qt-side image creation failed");
    return false;
  }

  VkImportMemoryWin32HandleInfoKHR importMem{VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR};
  importMem.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
  importMem.handle = m_sharedImageHandle;

  VkMemoryAllocateInfo qtMai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  qtMai.pNext = &importMem;
  qtMai.allocationSize = memReqs.size;
  qtMai.memoryTypeIndex = memType;

  if (m_qtfAllocateMemory(m_qtDevice, &qtMai, nullptr, &m_qtSharedImageMem) != VK_SUCCESS) {
    spdlog::error("ensureSharedImage: Qt-side memory import failed");
    return false;
  }
  m_qtfBindImageMemory(m_qtDevice, m_qtSharedImage, m_qtSharedImageMem, 0);

  // ── Wrap as QRhiTexture so Qt can copy from it ────────────────────────────
  if (m_sharedTex) {
    m_sharedTex->destroy();
    delete m_sharedTex;
    m_sharedTex = nullptr;
  }
  const QRhiTexture::Format rhiFormat =
  (m_coreImageFormat == VK_FORMAT_R8G8B8A8_UNORM ||
   m_coreImageFormat == VK_FORMAT_R8G8B8A8_SRGB)
    ? QRhiTexture::RGBA8
    : QRhiTexture::BGRA8;

  m_sharedTex = rhi()->newTexture(rhiFormat, QSize(int(wantW), int(wantH)), 1);
  if (!m_sharedTex->createFrom({
    reinterpret_cast<quint64>(m_qtSharedImage),
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
  })) {
    spdlog::error("ensureSharedImage: QRhiTexture::createFrom failed");
    delete m_sharedTex;
    m_sharedTex = nullptr;
    return false;
  }

  spdlog::info("ensureSharedImage: shared={}x{} core={}x{} fmt={} — GPU copy path active",
               wantW, wantH, m_vkRenderWidth, m_vkRenderHeight, int(m_coreImageFormat));
  return true;
#else
  return false;
#endif
}

void EmulatorItemRenderer::destroySharedImage() {
  if (m_sharedTex) {
    m_sharedTex->destroy();
    delete m_sharedTex;
    m_sharedTex = nullptr;
  }
#ifdef _WIN32
  if (m_qtSharedImage != VK_NULL_HANDLE && m_qtfDestroyImage) {
    m_qtfDestroyImage(m_qtDevice, m_qtSharedImage, nullptr);
    m_qtSharedImage = VK_NULL_HANDLE;
  }
  if (m_qtSharedImageMem != VK_NULL_HANDLE && m_qtfFreeMemory) {
    m_qtfFreeMemory(m_qtDevice, m_qtSharedImageMem, nullptr);
    m_qtSharedImageMem = VK_NULL_HANDLE;
  }
  if (m_sharedImageHandle) {
    CloseHandle(m_sharedImageHandle);
    m_sharedImageHandle = nullptr;
  }
  if (m_sharedImage != VK_NULL_HANDLE && m_vkfDestroyImage) {
    m_vkfDestroyImage(m_vkDevice, m_sharedImage, nullptr);
    m_sharedImage = VK_NULL_HANDLE;
  }
  if (m_sharedImageMem != VK_NULL_HANDLE && m_vkfFreeMemory) {
    m_vkfFreeMemory(m_vkDevice, m_sharedImageMem, nullptr);
    m_sharedImageMem = VK_NULL_HANDLE;
  }
#endif
  m_sharedImageW = m_sharedImageH = 0;
  m_sharedImageFmt = VK_FORMAT_UNDEFINED;
}

void EmulatorItemRenderer::createVulkanPerFrameResources() {
  m_vkFrameFences.resize(1);
  m_vkCmdBuffers.resize(1);

  VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  fi.flags = VK_FENCE_CREATE_SIGNALED_BIT; // pre-signaled so first wait_sync_index passes
  m_vkfCreateFence(m_vkDevice, &fi, nullptr, &m_vkFrameFences[0]);

  VkCommandPoolCreateInfo pi{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  pi.queueFamilyIndex = m_vkQueueFamilyIndex;
  pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  m_vkfCreateCommandPool(m_vkDevice, &pi, nullptr, &m_vkCmdPool);

  VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  ai.commandPool = m_vkCmdPool;
  ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  ai.commandBufferCount = 1;
  m_vkfAllocCommandBuffers(m_vkDevice, &ai, m_vkCmdBuffers.data());
}

void EmulatorItemRenderer::buildVulkanHwInterface(
  VkInstance instance, VkPhysicalDevice physDev,
  PFN_vkGetInstanceProcAddr procAddr) {
  m_vkInterface = {};
  m_vkInterface.interface_type = RETRO_HW_RENDER_INTERFACE_VULKAN;
  m_vkInterface.interface_version = RETRO_HW_RENDER_INTERFACE_VULKAN_VERSION;
  m_vkInterface.instance = instance;
  m_vkInterface.gpu = physDev;
  m_vkInterface.device = m_vkDevice;
  m_vkInterface.queue = m_vkQueue;
  m_vkInterface.queue_index = m_vkQueueFamilyIndex;
  m_vkInterface.handle = this;
  m_vkInterface.get_instance_proc_addr = procAddr;
  m_vkInterface.get_device_proc_addr = m_vkGetDeviceProcAddr;

  // Readback path: single slot, no swapchain
  m_vkInterface.get_sync_index = [](void *) -> unsigned { return 0; };
  m_vkInterface.get_sync_index_mask = [](void *) -> unsigned { return 1; };

  // No-op: renderVulkanFrame() CPU-waits for the previous readback to finish
  // before calling runFrame() again, so the core's VkImage is always safe to
  // reuse by the time wait_sync_index fires on the next frame.
  m_vkInterface.wait_sync_index = [](void *) {
  };

  m_vkInterface.set_image = [](void *handle,
                               const retro_vulkan_image *image,
                               uint32_t /* num_semaphores */,
                               const VkSemaphore * /* semaphores */,
                               uint32_t /* src_queue_family */) {
    auto *r = static_cast<EmulatorItemRenderer *>(handle);
    r->m_coreImage = image->create_info.image;
    r->m_coreImageFormat = image->create_info.format;
    r->m_coreImageView = image->image_view;
    r->m_coreImageLayout = image->image_layout; // SHADER_READ_ONLY_OPTIMAL
    r->m_vkFrameReady = true;
  };

  m_vkInterface.set_signal_semaphore = [](void *handle, VkSemaphore sem) {
    static_cast<EmulatorItemRenderer *>(handle)->m_coreSignalSem = sem;
  };

  m_vkInterface.lock_queue = [](void *handle) {
    static_cast<EmulatorItemRenderer *>(handle)->m_vkQueueMutex.lock();
  };

  m_vkInterface.unlock_queue = [](void *handle) {
    static_cast<EmulatorItemRenderer *>(handle)->m_vkQueueMutex.unlock();
  };

  m_vkInterface.set_command_buffers = nullptr; // not used by mupen

  m_vkInterfaceReady = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Vulkan — per-frame rendering (GPU shared-image copy path)
// ─────────────────────────────────────────────────────────────────────────────

void EmulatorItemRenderer::renderVulkanFrame() {
  m_coreSignalSem = VK_NULL_HANDLE;
  m_vkFrameReady = false;

  if (m_playbackMultiplier > 1) {
    for (int i = 0; i < static_cast<int>(m_playbackMultiplier); i++)
      m_emulatorInstance->runFrame();
  } else {
    m_emulatorInstance->runFrame();
  }

  // vkFrameReady is set by core in the video callback
  if (!m_vkFrameReady || m_coreImage == VK_NULL_HANDLE || !m_usingHardwareRenderer) {
    spdlog::debug("renderVulkanFrame: early exit – frameReady={} coreImage={} hwRenderer={}",
                  m_vkFrameReady, m_coreImage != VK_NULL_HANDLE, m_usingHardwareRenderer);
    return;
  }

  // Skip until receive() has reported real dimensions (first frame may be 1×1 init)
  if (m_vkRenderWidth < 2 || m_vkRenderHeight < 2) {
    spdlog::debug("renderVulkanFrame: degenerate dims {}x{}, skipping blit",
                  m_vkRenderWidth, m_vkRenderHeight);
    return;
  }

  if (!ensureSharedImage()) {
    spdlog::debug("renderVulkanFrame: ensureSharedImage() failed");
    return;
  }

  m_firstFrameReady = true;


  spdlog::debug("renderVulkanFrame: blitting {}x{} -> {}x{} fmt={}",
                m_vkRenderWidth, m_vkRenderHeight, m_sharedImageW, m_sharedImageH,
                static_cast<int>(m_coreImageFormat));

  // Wait for the previous frame's copy command buffer to finish before we
  // record into it again, then reset for re-use. (Fence is pre-signaled on first call.)
  // Done here, after all early-return paths, so we never reset without a matching submit.
  m_vkfWaitForFences(m_vkDevice, 1, &m_vkFrameFences[0], VK_TRUE, UINT64_MAX);
  m_vkfResetFences(m_vkDevice, 1, &m_vkFrameFences[0]);

  VkCommandBuffer cmd = m_vkCmdBuffers[0];
  VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  m_vkfBeginCommandBuffer(cmd, &bi);

  // Transition core image SHADER_READ_ONLY → TRANSFER_SRC
  // Transition shared image UNDEFINED → TRANSFER_DST (discard previous contents)
  VkImageMemoryBarrier preCopy[2]{};
  preCopy[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  preCopy[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  preCopy[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  preCopy[0].oldLayout = m_coreImageLayout;
  preCopy[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  preCopy[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  preCopy[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  preCopy[0].image = m_coreImage;
  preCopy[0].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

  preCopy[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  preCopy[1].srcAccessMask = 0;
  preCopy[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  preCopy[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; // discard OK
  preCopy[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  preCopy[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  preCopy[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  preCopy[1].image = m_sharedImage;
  preCopy[1].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

  m_vkfCmdPipelineBarrier(cmd,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                          0, 0, nullptr, 0, nullptr, 2, preCopy);

  // GPU blit: core image (render res) → shared image (display res), with linear scaling
  VkImageBlit blitRgn{};
  blitRgn.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  blitRgn.srcOffsets[0] = {0, 0, 0};
  blitRgn.srcOffsets[1] = {static_cast<int32_t>(m_vkRenderWidth), static_cast<int32_t>(m_vkRenderHeight), 1};
  blitRgn.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  blitRgn.dstOffsets[0] = {0, 0, 0};
  blitRgn.dstOffsets[1] = {static_cast<int32_t>(m_sharedImageW), static_cast<int32_t>(m_sharedImageH), 1};
  m_vkfCmdBlitImage(cmd,
                    m_coreImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    m_sharedImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &blitRgn, VK_FILTER_LINEAR);

  // Transition core image back to SHADER_READ_ONLY
  // Transition shared image TRANSFER_DST → SHADER_READ_ONLY so Qt can sample it
  VkImageMemoryBarrier postCopy[2]{};
  postCopy[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  postCopy[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  postCopy[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  postCopy[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  postCopy[0].newLayout = m_coreImageLayout;
  postCopy[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  postCopy[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  postCopy[0].image = m_coreImage;
  postCopy[0].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

  postCopy[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  postCopy[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  postCopy[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  postCopy[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  postCopy[1].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  postCopy[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  postCopy[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  postCopy[1].image = m_sharedImage;
  postCopy[1].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

  m_vkfCmdPipelineBarrier(cmd,
                          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          0, 0, nullptr, 0, nullptr, 2, postCopy);

  m_vkfEndCommandBuffer(cmd);

  // Submit: wait on set_image's binary semaphore (if any), signal timeline semaphore
  m_sharedSemValue++;

  uint64_t waitVal = 0; // ignored for binary semaphore per spec
  uint64_t signalVal = m_sharedSemValue;

  VkTimelineSemaphoreSubmitInfo tsInfo{VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO};
  tsInfo.signalSemaphoreValueCount = 1;
  tsInfo.pSignalSemaphoreValues = &signalVal;

  VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  si.pNext = &tsInfo;
  si.commandBufferCount = 1;
  si.pCommandBuffers = &cmd;
  si.signalSemaphoreCount = 1;
  si.pSignalSemaphores = &m_sharedSemaphore;

  if (m_coreSignalSem != VK_NULL_HANDLE) {
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &m_coreSignalSem;
    si.pWaitDstStageMask = &waitStage;
    tsInfo.waitSemaphoreValueCount = 1;
    tsInfo.pWaitSemaphoreValues = &waitVal;
  } {
    std::lock_guard lock(m_vkQueueMutex);
    m_vkfQueueSubmit(m_vkQueue, 1, &si, m_vkFrameFences[0]);
  }
  // CPU-sync: wait for the blit to finish so Qt can safely read m_sharedImage
  // without needing a cross-device semaphore import. The wait is only for the
  // blit CB (fast), not a full CPU readback — we still avoid the memcpy.
  // The fence stays signaled; the wait at the top of the next frame is a no-op.
  m_vkfWaitForFences(m_vkDevice, 1, &m_vkFrameFences[0], VK_TRUE, UINT64_MAX);
}

// ─────────────────────────────────────────────────────────────────────────────
// Vulkan — teardown
// ─────────────────────────────────────────────────────────────────────────────

void EmulatorItemRenderer::destroyVulkan() {
  if (m_vkDevice == VK_NULL_HANDLE)
    return;

  m_vkfDeviceWaitIdle(m_vkDevice);

  // context_destroy is called by Core::~Core() while the DLL is loaded — not here.

  // Shared image / cross-device resources
  destroySharedImage();
#ifdef _WIN32
  if (m_qtImportedSem != VK_NULL_HANDLE && m_qtfDestroySemaphore) {
    m_qtfDestroySemaphore(m_qtDevice, m_qtImportedSem, nullptr);
    m_qtImportedSem = VK_NULL_HANDLE;
  }
  if (m_sharedSemHandle) {
    CloseHandle(m_sharedSemHandle);
    m_sharedSemHandle = nullptr;
  }
  if (m_sharedSemaphore != VK_NULL_HANDLE && m_vkfDestroySemaphore) {
    m_vkfDestroySemaphore(m_vkDevice, m_sharedSemaphore, nullptr);
    m_sharedSemaphore = VK_NULL_HANDLE;
  }
#endif

  if (m_fnDestroyDevice) {
    m_fnDestroyDevice();
    m_fnDestroyDevice = nullptr;
  }

  if (!m_vkFrameFences.empty() && m_vkFrameFences[0])
    m_vkfDestroyFence(m_vkDevice, m_vkFrameFences[0], nullptr);
  m_vkFrameFences.clear();

  if (m_vkCmdPool)
    m_vkfDestroyCommandPool(m_vkDevice, m_vkCmdPool, nullptr);
  m_vkCmdPool = VK_NULL_HANDLE;

  if (m_vkStagingBuffer) {
    m_vkfDestroyBuffer(m_vkDevice, m_vkStagingBuffer, nullptr);
    m_vkStagingBuffer = VK_NULL_HANDLE;
  }
  if (m_vkStagingMemory) {
    m_vkfFreeMemory(m_vkDevice, m_vkStagingMemory, nullptr);
    m_vkStagingMemory = VK_NULL_HANDLE;
  }
  m_vkStagingSize = 0;

  m_vkfDestroyDevice(m_vkDevice, nullptr);
  m_vkDevice = VK_NULL_HANDLE;

  // Destroy the Granite VkInstance if we created it (v2 create_instance path).
  // Qt's VkInstance is not ours to destroy.
  if (m_graniteInstance != VK_NULL_HANDLE && m_vkfDestroyInstance) {
    m_vkfDestroyInstance(m_graniteInstance, nullptr);
    m_graniteInstance = VK_NULL_HANDLE;
  }
}
