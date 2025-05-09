#include "emulator_item_renderer.hpp"

#include <QAudioInput>
#include <QMediaFormat>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QQuickWindow>
#include <QVideoFrame>
#include <libretro/libretro_vulkan.h>
#include <rhi/qrhi.h>
#include <spdlog/spdlog.h>

#include "emulator_item.hpp"
#include <QVulkanDeviceFunctions>
#include <QVulkanFunctions>

#include <unistd.h>

static EmulatorItemRenderer *globalRenderer = nullptr;
static QRhi *globalRhi = nullptr;
static QRhiCommandBuffer *globalCb = nullptr;

EmulatorItemRenderer::EmulatorItemRenderer(
    const QSGRendererInterface::GraphicsApi api,
    std::unique_ptr<libretro::Core> core)
    : m_graphicsApi(api), m_core(std::move(core)) {
  m_core->setVideoReceiver(this);
  globalRenderer = this;
  m_emulationTimer.start();
  m_renderCallTimer.start();
  // m_actualTimer.setInterval(std::chrono::nanoseconds(100000));
  // m_actualTimer.setSingleShot(false);
  // m_actualTimer.setTimerType(Qt::PreciseTimer);
  //
  // QObject::connect(&m_actualTimer, &QChronoTimer::timeout, [&] { update();
  // }); m_actualTimer.callOnTimeout([&] {
  //   auto elapsed = m_emulationTimer.nsecsElapsed();
  //   if (elapsed > m_emulationTimingTargetNs) {
  //     m_emulationTimer.restart();
  //     QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
  //     update();
  //   }
  //   // spdlog::info("Actual timer elapsed: {}",
  //   // m_emulationTimer.nsecsElapsed());
  // });

  // m_emulationTimerThread.start();
  // m_actualTimer.moveToThread(&m_emulationTimerThread);
  // QMetaObject::invokeMethod(&m_actualTimer, "start", Qt::QueuedConnection);
}

void EmulatorItemRenderer::submitCommand(const EmulatorCommand command) {
  m_commandQueue.enqueue(command);
}

EmulatorItemRenderer::~EmulatorItemRenderer() {
  spdlog::info("Destroying EmulatorItemRenderer");
  m_quitting = true;

  m_emulationTimer.invalidate();

  if (!m_paused) {
    if (m_playSessionTimer.isValid()) {
      m_playSession.unpausedDurationMillis += m_playSessionTimer.elapsed();
    }
  }

  m_playSession.endTime = QDateTime::currentMSecsSinceEpoch();
  getActivityLog()->createPlaySession(m_playSession);

  save(true);
  getAchievementManager()->unloadGame();

  for (auto &url : m_rewindImageUrls) {
    getGameImageProvider()->removeImageWithUrl(url);
  }

  m_rewindImageUrls.clear();
  // Don't need to destroy the context here as it is handled by the Core object.
}

void EmulatorItemRenderer::receive(const void *data, unsigned width,
                                   unsigned height, size_t pitch) {
  if (data && data != RETRO_HW_FRAME_BUFFER_VALID && width > 0 && height > 0 &&
      pitch > 0) {
    QImage image((uchar *)data, width, height, pitch, m_pixelFormat);
    if (m_graphicsApi == QSGRendererInterface::OpenGL) {
      image.mirror();
    }
    auto newImage =
        image.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
    m_currentUpdateBatch->uploadTexture(colorTexture(), newImage);
  }
}

retro_hw_context_type EmulatorItemRenderer::getPreferredHwRender() {
  if (m_graphicsApi == QSGRendererInterface::OpenGL) {
    return RETRO_HW_CONTEXT_OPENGL;
  }

  if (m_graphicsApi == QSGRendererInterface::Vulkan) {
    return RETRO_HW_CONTEXT_VULKAN;
  }

  return RETRO_HW_CONTEXT_NONE;
}

void EmulatorItemRenderer::getHwRenderContext(
    retro_hw_context_type &contextType, unsigned int &major,
    unsigned int &minor) {
  // contextType = RETRO_HW_CONTEXT_OPENGLES_VERSION;
  // major = 3;
  // minor = 1;
}

proc_address_t EmulatorItemRenderer::getProcAddress(const char *sym) {
  if (m_graphicsApi == QSGRendererInterface::OpenGL) {
    return QOpenGLContext::currentContext()->getProcAddress(sym);
  }
  if (m_graphicsApi == QSGRendererInterface::Vulkan) {
    spdlog::info("Returning address for symbol: {}", sym);

    auto vulkanHandles = reinterpret_cast<const QRhiVulkanNativeHandles *>(
        rhi()->nativeHandles());
    auto addr = vulkanHandles->inst->functions()->vkGetDeviceProcAddr(
        vulkanHandles->dev, sym);

    // spdlog::info("Address: {}", addr);
    return addr;
  }

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
  // return m_fbo ? m_fbo->handle() : 0;
  return m_currentFramebufferId;
}

void EmulatorItemRenderer::setSystemAVInfo(retro_system_av_info *info) {
  if (info) {
    m_coreBaseWidth = info->geometry.base_width;
    m_coreBaseHeight = info->geometry.base_height;
    m_coreMaxWidth = info->geometry.max_width;
    m_coreMaxHeight = info->geometry.max_height;
    m_coreAspectRatio = info->geometry.aspect_ratio;
    m_calculatedAspectRatio = static_cast<float>(m_coreBaseWidth) /
                              static_cast<float>(m_coreBaseHeight);

    auto millis = 1000.0 / info->timing.fps;
    m_emulationTimingTargetNs = static_cast<std::int64_t>(millis * 1000000.0);

    if (m_geometryChangedCallback) {
      m_geometryChangedCallback(m_coreBaseWidth, m_coreBaseHeight,
                                m_coreAspectRatio, info->timing.fps);
    }
  }
}

void EmulatorItemRenderer::setPixelFormat(retro_pixel_format *format) {
  switch (*format) {
  case RETRO_PIXEL_FORMAT_0RGB1555:
    spdlog::debug("Pixel format: 0RGB1555\n");
    break;
  case RETRO_PIXEL_FORMAT_XRGB8888:
    m_pixelFormat = QImage::Format_RGB32;
    break;
  case RETRO_PIXEL_FORMAT_RGB565:
    m_pixelFormat = QImage::Format_RGB16;
    break;
  case RETRO_PIXEL_FORMAT_UNKNOWN:
    spdlog::debug("Pixel format: UNKNOWN\n");
    break;
  }
}

void EmulatorItemRenderer::setHwRenderContextNegotiationInterface(
    retro_hw_render_context_negotiation_interface *inter) {
  if (m_graphicsApi == QSGRendererInterface::Vulkan) {
    spdlog::info("Doing the vulkan stuff");
    auto iface = reinterpret_cast<
        retro_hw_render_context_negotiation_interface_vulkan *>(inter);

    auto thingggg = *iface;
    // iface->create_device = [](retro_vulkan_context *context, VkInstance
    // instance, VkPhysicalDevice gpu,
    //                           VkSurfaceKHR surface, PFN_vkGetInstanceProcAddr
    //                           get_instance_proc_addr, const char
    //                           **required_device_extensions, unsigned
    //                           num_required_device_extensions, const char
    //                           **required_device_layers, unsigned
    //                           num_required_device_layers, const
    //                           VkPhysicalDeviceFeatures *required_features) {
    //     spdlog::info("Calling create_device");
    //     return false;
    // };
    // iface->create_device2 = [](retro_vulkan_context *context, VkInstance
    // instance, VkPhysicalDevice gpu,
    //                            VkSurfaceKHR surface,
    //                            PFN_vkGetInstanceProcAddr
    //                            get_instance_proc_addr,
    //                            retro_vulkan_create_device_wrapper_t
    //                            create_device_wrapper, void *opaque) {
    //     printf("Calling create device 2***************\n");
    //     // auto vulkanHandles = reinterpret_cast<const
    //     QRhiVulkanNativeHandles *>(rhi()->nativeHandles());
    //     // return vulkanHandles-;
    //     return false;
    // };
    //
    // iface->create_instance = [](PFN_vkGetInstanceProcAddr
    // get_instance_proc_addr, const VkApplicationInfo *app,
    //                             retro_vulkan_create_instance_wrapper_t
    //                             create_instance_wrapper, void *opaque) {
    //     spdlog::info("Calling create_instance");
    //     printf("Calling create instance***************\n");
    //     auto vulkanHandles = reinterpret_cast<const QRhiVulkanNativeHandles
    //     *>(globalRenderer->rhi()->
    //         nativeHandles());
    //     return vulkanHandles->inst->vkInstance();
    // };
    //
    // iface->destroy_device = []() {
    //     printf("destroy device***************\n");
    //     spdlog::info("Doing nothing in destroy device");
    // };
  }
}

void EmulatorItemRenderer::setHwRenderInterface(
    retro_hw_render_interface **iface) {
  if (m_graphicsApi == QSGRendererInterface::Vulkan) {
    spdlog::info("Setting Vulkan hardware render interface");
    const auto ptr =
        reinterpret_cast<retro_hw_render_interface_vulkan **>(iface);
    *ptr = new retro_hw_render_interface_vulkan;

    auto vulkanHandles = reinterpret_cast<const QRhiVulkanNativeHandles *>(
        globalRhi->nativeHandles());

    (*ptr)->device = vulkanHandles->dev;
    (*ptr)->instance = vulkanHandles->inst->vkInstance();
    (*ptr)->gpu = vulkanHandles->physDev;
    (*ptr)->queue = vulkanHandles->gfxQueue;
    (*ptr)->queue_index = vulkanHandles->gfxQueueIdx;
    (*ptr)->get_device_proc_addr = [](VkDevice device, const char *pName) {
      spdlog::info("Getting device proc addr: {}", pName);
      auto handles = reinterpret_cast<const QRhiVulkanNativeHandles *>(
          globalRhi->nativeHandles());
      return handles->inst->functions()->vkGetDeviceProcAddr(device, pName);
    };

    (*ptr)->get_instance_proc_addr = [](VkInstance instance,
                                        const char *pName) {
      spdlog::info("Getting instance proc addr: {}", pName);
      auto handles = reinterpret_cast<const QRhiVulkanNativeHandles *>(
          globalRhi->nativeHandles());
      return handles->inst->getInstanceProcAddr(pName);
    };

    (*ptr)->handle = this;

    (*ptr)->interface_type = RETRO_HW_RENDER_INTERFACE_VULKAN;
    (*ptr)->interface_version = RETRO_HW_RENDER_INTERFACE_VULKAN_VERSION;

    (*ptr)->wait_sync_index = [](void *handle) {
      spdlog::info("Calling wait sync index");
      return;
    };
    (*ptr)->get_sync_index = [](void *handle) {
      spdlog::info("Calling get sync index");
      return (uint32_t)0;
    };
    (*ptr)->get_sync_index_mask = [](void *handle) {
      spdlog::info("Calling get sync index mask");
      return (uint32_t)4;
    };
    (*ptr)->lock_queue = [](void *handle) {
      spdlog::info("Calling lock queue");
    };
    (*ptr)->set_command_buffers = [](void *handle, uint32_t num_cmd,
                                     const VkCommandBuffer *cmd) {
      spdlog::info("Calling set command buffers");
    };
    (*ptr)->set_image =
        [](void *handle, const retro_vulkan_image *image,
           uint32_t num_semaphores, const VkSemaphore *semaphores,
           uint32_t src_queue_family) { spdlog::info("Calling set image"); };
    (*ptr)->set_signal_semaphore = [](void *handle, VkSemaphore semaphore) {
      spdlog::info("Calling set signal semaphore");
    };
    (*ptr)->unlock_queue = [](void *handle) {
      spdlog::info("Calling unlock queue");
    };
  }
}

void EmulatorItemRenderer::initialize(QRhiCommandBuffer *cb) {
  globalCb = cb;
  if (globalRhi == nullptr) {
    globalRhi = rhi();
  }

  if (m_graphicsApi == QSGRendererInterface::OpenGL) {
    if (!m_openGlInitialized) {
      initializeOpenGLFunctions();
      m_openGlInitialized = true;
    }
  }

  // if (m_createDeviceFunction1) {
  //     auto handles = reinterpret_cast<const QRhiVulkanNativeHandles
  //     *>(globalRhi->nativeHandles()); retro_vulkan_context ctx{};
  //
  //     for (const auto &ext: handles->inst->supportedExtensions()) {
  //         spdlog::info("Supported extension: {}", ext.name.toStdString());
  //     }
  //
  //     ctx.device = handles->dev;
  //     ctx.gpu = handles->physDev;
  //     ctx.queue = handles->gfxQueue;
  //     ctx.queue_family_index = handles->gfxQueueIdx;
  //     ctx.presentation_queue = handles->gfxQueue;
  //     ctx.presentation_queue_family_index = handles->gfxQueueIdx;
  //
  //     m_createDeviceFunction1(&ctx, handles->inst->vkInstance(),
  //     handles->physDev,
  //                             QVulkanInstance::surfaceForWindow(m_window),
  //                             [](VkInstance instance, const char *pName) {
  //                                 spdlog::info("Getting instance proc addr:
  //                                 {}", pName); auto handles =
  //                                 reinterpret_cast<const
  //                                 QRhiVulkanNativeHandles *>(globalRhi->
  //                                     nativeHandles());
  //                                 return
  //                                 handles->inst->getInstanceProcAddr(pName);
  //                             }, {}, 0, {}, 0, {});
  //     m_createDeviceFunction1 = nullptr;
  // }

  // if (m_createDeviceFunction) {
  //     QRhiResourceUpdateBatch *resourceUpdates =
  //     rhi()->nextResourceUpdateBatch(); cb->beginPass(renderTarget(), {0, 0,
  //     0, 0}, {1.0f, 0}, resourceUpdates, QRhiCommandBuffer::ExternalContent);
  //     cb->beginExternal();
  //     auto handles = reinterpret_cast<const QRhiVulkanNativeHandles
  //     *>(globalRhi->nativeHandles()); retro_vulkan_context ctx{};
  //
  //     for (const auto &ext: handles->inst->supportedExtensions()) {
  //         spdlog::info("Supported extension: {}", ext.name.toStdString());
  //     }
  //
  //     ctx.device = handles->dev;
  //     ctx.gpu = handles->physDev;
  //     ctx.queue = handles->gfxQueue;
  //     ctx.queue_family_index = handles->gfxQueueIdx;
  //     ctx.presentation_queue = handles->gfxQueue;
  //     ctx.presentation_queue_family_index = handles->gfxQueueIdx;
  //
  //     // auto getInstanceProcAddr =
  //     reinterpret_cast<PFN_vkGetInstanceProcAddr>(handles->inst->getInstanceProcAddr);
  //
  //     m_createDeviceFunction(&ctx, handles->inst->vkInstance(),
  //     handles->physDev,
  //                            QVulkanInstance::surfaceForWindow(m_window),
  //                            [](VkInstance instance, const char *pName) {
  //                                spdlog::info("Getting instance proc addr:
  //                                {}", pName); auto handles =
  //                                reinterpret_cast<const
  //                                QRhiVulkanNativeHandles *>(globalRhi->
  //                                    nativeHandles());
  //                                return
  //                                handles->inst->getInstanceProcAddr(pName);
  //                            },
  //                            [](VkPhysicalDevice gpu, void *opaque, const
  //                            VkDeviceCreateInfo *create_info) {
  //                                auto myHandles = reinterpret_cast<const
  //                                QRhiVulkanNativeHandles *>(globalRhi->
  //                                    nativeHandles());
  //                                return myHandles->dev;
  //                            }, this);
  //
  //     cb->endExternal();
  //     cb->endPass(resourceUpdates);
  //
  //     m_createDeviceFunction = nullptr;
  // }

  if (m_resetContextFunction) {
    QRhiResourceUpdateBatch *resourceUpdates = rhi()->nextResourceUpdateBatch();
    cb->beginPass(renderTarget(), {0, 0, 0, 0}, {1.0f, 0}, resourceUpdates,
                  QRhiCommandBuffer::ExternalContent);
    cb->beginExternal();

    if (m_graphicsApi == QSGRendererInterface::OpenGL) {
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_currentFramebufferId);
    }

    spdlog::info("Calling reset context function");
    m_resetContextFunction();
    m_resetContextFunction = nullptr;

    cb->endExternal();
    cb->endPass(resourceUpdates);
  }
}

void EmulatorItemRenderer::synchronize(QQuickRhiItem *item) {
  const auto emulatorItem = dynamic_cast<EmulatorItem *>(item);

  if (emulatorItem == nullptr) {
    return;
  }

  m_emulatorItem = emulatorItem;
  // spdlog::error(
  //     "Synchronizing: {}",
  //     std::chrono::high_resolution_clock::now().time_since_epoch().count());

  // if (emulatorItem->window() != nullptr) {
  //   if (emulatorItem->window()->screen() != nullptr) {
  //     auto refreshRate = emulatorItem->window()->screen()->refreshRate();
  //     auto floor = std::floor(refreshRate);
  //     auto ceil = std::ceil(refreshRate);
  //
  //     if (floor == 120 || ceil == 120) {
  //       m_waitFrames = 1;
  //     } else if ((floor < 182 && floor > 172) || (ceil < 182 && ceil > 172))
  //     {
  //       // TODO: Do something here to indicate that the timing is gonna be a
  //       bit
  //       // off
  //       m_waitFrames = 2;
  //     }
  //   }
  // }

  if (m_paused && !emulatorItem->paused()) {
    if (m_playSessionTimer.isValid()) {
      m_playSessionTimer.restart();
    } else {
      m_playSessionTimer.start();
    }
    // Going from paused to unpaused
  } else if (!m_paused && emulatorItem->paused()) {
    if (m_playSessionTimer.isValid()) {
      m_playSession.unpausedDurationMillis += m_playSessionTimer.elapsed();
    }
    // Going from unpaused to paused
  }

  m_paused = emulatorItem->paused();

  m_gameData = emulatorItem->m_gameData;
  m_saveData = emulatorItem->m_saveData;
  m_corePath = emulatorItem->m_corePath;
  m_contentHash = emulatorItem->m_contentHash;
  m_saveSlotNumber = emulatorItem->m_saveSlotNumber;
  m_platformId = emulatorItem->m_platformId;
  m_contentPath = emulatorItem->m_contentPath;
  m_gameReady = emulatorItem->m_gameReady;

  while (!m_commandQueue.isEmpty()) {
    switch (const auto command = m_commandQueue.dequeue(); command.type) {
    case RunFrame:
      m_shouldRunFrame = true;
      break;
    case ResetGame:
      m_core->reset();
      getAchievementManager()->reset();
      break;
    case WriteSaveFile:
      m_shouldSave = true;
      break;
    case WriteRewindPoint: {
      if (m_paused) {
        return;
      }
      SuspendPoint suspendPoint;
      suspendPoint.state = m_core->serializeState();
      suspendPoint.image = m_currentImage;
      suspendPoint.timestamp = QDateTime::currentMSecsSinceEpoch();
      suspendPoint.retroachievementsState =
          getAchievementManager()->serializeState();
      m_rewindPoints.push_front(suspendPoint);

      if (m_rewindPoints.length() > 10) {
        m_rewindPoints.pop_back();
      }

    } break;
    case EmitRewindPoints: {
      for (auto &url : m_rewindImageUrls) {
        getGameImageProvider()->removeImageWithUrl(url);
      }

      m_rewindImageUrls.clear();

      QList<QJsonObject> points;
      int i = 1;

      auto now = QDateTime::currentMSecsSinceEpoch();
      for (const auto &point : m_rewindPoints) {
        auto time = QDateTime::fromMSecsSinceEpoch(point.timestamp).time();
        auto diff = time.secsTo(QDateTime::fromMSecsSinceEpoch(now).time());

        QJsonObject obj;
        auto url = getGameImageProvider()->setImage(point.image);
        m_rewindImageUrls.append(url);
        obj["image_url"] = url;
        // obj["time"] = now - point.timestamp;
        obj["time"] = time.toString();
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
    } break;
    case LoadRewindPoint: {
      auto point = m_rewindPoints.at(command.rewindPointIndex - 1);
      m_core->deserializeState(point.state);
      if (!point.retroachievementsState.empty()) {
        getAchievementManager()->deserializeState(point.retroachievementsState);
      }
      if (m_paused) {
        m_overlayImage = point.image;
        m_overlayImage.mirror();
        m_overlayImage = m_overlayImage.convertToFormat(
            QImage::Format_RGBA8888_Premultiplied);

        m_currentImage = m_overlayImage;
        if (m_graphicsApi == QSGRendererInterface::OpenGL) {
          m_currentImage.mirror();
        }
      }
    } break;
    case WriteSuspendPoint: {
      SuspendPoint suspendPoint;
      suspendPoint.state = m_core->serializeState();
      suspendPoint.retroachievementsState =
          getAchievementManager()->serializeState();
      suspendPoint.image = m_currentImage;
      suspendPoint.timestamp = QDateTime::currentMSecsSinceEpoch();
      suspendPoint.saveSlotNumber = m_saveSlotNumber;

      getSaveManager()->writeSuspendPoint(m_contentHash, m_saveSlotNumber,
                                          command.suspendPointIndex,
                                          suspendPoint);
    } break;
    case LoadSuspendPoint: {
      const auto point = getSaveManager()->readSuspendPoint(
          m_contentHash, m_saveSlotNumber, command.suspendPointIndex);
      if (point.has_value()) {
        SuspendPoint suspendPoint;
        suspendPoint.state = m_core->serializeState();
        suspendPoint.retroachievementsState =
            getAchievementManager()->serializeState();
        suspendPoint.image = m_currentImage;
        suspendPoint.timestamp = QDateTime::currentMSecsSinceEpoch();
        suspendPoint.saveSlotNumber = m_saveSlotNumber;
        m_beforeLastLoadSuspendPoint = suspendPoint;
        emulatorItem->m_canUndoLoadSuspendPoint = true;
        emulatorItem->canUndoLoadSuspendPointChanged();

        m_core->deserializeState(point->state);
        if (!point->retroachievementsState.empty()) {
          getAchievementManager()->deserializeState(
              point->retroachievementsState);
        }

        if (m_paused) {
          m_overlayImage = point->image;
          m_overlayImage.mirror();
          m_overlayImage = m_overlayImage.convertToFormat(
              QImage::Format_RGBA8888_Premultiplied);

          m_currentImage = m_overlayImage;
          if (m_graphicsApi == QSGRendererInterface::OpenGL) {
            m_currentImage.mirror();
          }
        }
      }
    } break;
    case UndoLoadSuspendPoint:
      emulatorItem->m_canUndoLoadSuspendPoint = false;
      emulatorItem->canUndoLoadSuspendPointChanged();
      if (m_beforeLastLoadSuspendPoint.state.empty()) {
        break;
      }

      m_core->deserializeState(m_beforeLastLoadSuspendPoint.state);
      if (!m_beforeLastLoadSuspendPoint.retroachievementsState.empty()) {
        getAchievementManager()->deserializeState(
            m_beforeLastLoadSuspendPoint.retroachievementsState);
      }

      if (m_paused) {
        m_overlayImage = m_beforeLastLoadSuspendPoint.image;
        m_overlayImage.mirror();
        m_overlayImage = m_overlayImage.convertToFormat(
            QImage::Format_RGBA8888_Premultiplied);

        m_currentImage = m_overlayImage;
        if (m_graphicsApi == QSGRendererInterface::OpenGL) {
          m_currentImage.mirror();
        }
      }

      m_beforeLastLoadSuspendPoint = SuspendPoint();

      // Handle undo load suspend point
      break;
    case SetPlaybackMultiplier:
      spdlog::info("Setting playback multiplier to {}",
                   command.playbackMultiplier);
      m_playbackMultiplier = command.playbackMultiplier;
      break;
    }
  }
}

void EmulatorItemRenderer::render(QRhiCommandBuffer *cb) {
  if (m_quitting) {
    return;
  }

  auto elapsedSinceLastFrame = m_emulationTimer.nsecsElapsed();
  auto renderTime = m_renderCallTimer.nsecsElapsed();
  if (renderTime < m_emulationTimingTargetNs) {
    m_renderCallTimes.push_back(renderTime);

    if (m_renderCallTimes.size() > 40) {
      m_renderCallTimes.pop_front();
    }

    auto sum = std::accumulate(m_renderCallTimes.begin(),
                               m_renderCallTimes.end(), 0LL);
    m_averageTimeBetweenRenderCalls = sum / m_renderCallTimes.size();
  }

  m_renderCallTimer.restart();

  // spdlog::info("Average time between render calls: {}",
  //              m_averageTimeBetweenRenderCalls);

  auto deviationIfRunNow = elapsedSinceLastFrame - m_emulationTimingTargetNs;
  auto deviationIfRunLater = elapsedSinceLastFrame +
                             m_averageTimeBetweenRenderCalls -
                             m_emulationTimingTargetNs;

  if (deviationIfRunLater < 0 && deviationIfRunNow < 0) {
    update();
    return;
  }

  // spdlog::info("If we run now: {}, If we run later: {}", deviationIfRunNow,
  //              deviationIfRunLater);

  // if (m_measureTime) {
  //   m_measureTime = false;
  //   m_emulationWorkTimeBuffer.push_back(elapsedSinceLastFrame);
  //   if (m_emulationWorkTimeBuffer.size() > 40) {
  //     m_emulationWorkTimeBuffer.pop_front();
  //   }
  //
  //   // get average
  //   auto sum = std::accumulate(m_emulationWorkTimeBuffer.begin(),
  //                              m_emulationWorkTimeBuffer.end(), 0LL);
  //   m_averageEmulationTime = sum / m_emulationWorkTimeBuffer.size();
  // }
  //
  // // spdlog::info("Average time: {}", m_averageEmulationTime);
  //
  // if (!m_runNextFrame) {
  //   auto ifRunBefore = elapsedSinceLastFrame;
  //   auto ifRunAfter = elapsedSinceLastFrame + m_averageEmulationTime;
  //
  //   if (ifRunBefore < m_emulationTimingTargetNs &&
  //       ifRunAfter < m_emulationTimingTargetNs) {
  //     update();
  //     return;
  //   }
  //
  //   // spdlog::info("diff if run before: {}",
  //   //              ifRunBefore - m_emulationTimingTargetNs);
  //   // spdlog::info("diff if run after: {}", ifRunAfter -
  //   // m_emulationTimingTargetNs);
  //
  //   auto diffIfRunBefore = std::abs(ifRunBefore - m_emulationTimingTargetNs);
  //   auto diffIfRunAfter = std::abs(ifRunAfter - m_emulationTimingTargetNs);
  //
  //   // spdlog::info("Before: {}, After: {}", diffIfRunBefore,
  //   diffIfRunAfter);
  //
  //   if (diffIfRunBefore > diffIfRunAfter) {
  //     update();
  //     return;
  //   }
  //
  //   // spdlog::info("Total target deviation: {}", m_totalTargetDeviation);
  //
  //   if (m_totalTargetDeviation < 0) {
  //     m_runNextFrame = true;
  //     update();
  //     return;
  //   }
  //   //
  //   // if (m_totalTargetDeviation) {
  //   //   update();
  //   //   return;
  //   // }
  // }
  //
  // m_totalTargetDeviation += elapsedSinceLastFrame -
  // m_emulationTimingTargetNs; m_runNextFrame = false;

  // spdlog::info("total target deviation: {}", m_totalTargetDeviation);

  // if (abs(m_emulationTimingTargetNs - ifRunBefore))

  // if (elapsed < m_emulationTimingTargetNs - 300000) {
  //   // spdlog::info("Diff: {}", m_emulationTimingTargetNs - elapsed);
  //   // if (m_emulationTimingTargetNs - elapsed >
  //   //     std::chrono::milliseconds(3).count()) {
  //   //   // sleep for 2ms
  //   //   std::this_thread::sleep_for(std::chrono::microseconds(100));
  //   // }
  //   update();
  //   return;
  // }

  // spdlog::info("Running frame after {}ms", elapsed / 1000000.0f);

  if (m_emulatorItem) {
    emit m_emulatorItem->aboutToRunFrame();
  }

  m_emulationTimer.restart();

  // spdlog::error(
  //     "Starting frame: {}",
  //     std::chrono::high_resolution_clock::now().time_since_epoch().count());

  // auto elapsed = m_oneFrameTimer.nsecsElapsed();
  // m_oneFrameTimer.restart();
  //
  // spdlog::info("Time between frames: {}", elapsed);

  globalCb = cb;
  // auto nativeHandles = reinterpret_cast<const QRhiGles2NativeHandles
  // *>(rhi()->nativeHandles());
  if (m_paused) {
    if (!m_overlayImage.isNull()) {
      const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
      QRhiResourceUpdateBatch *resourceUpdates =
          rhi()->nextResourceUpdateBatch();
      cb->beginPass(renderTarget(), clearColor, {1.0f, 0}, resourceUpdates,
                    QRhiCommandBuffer::ExternalContent);

      resourceUpdates->uploadTexture(colorTexture(), m_overlayImage.copy());
      cb->endPass(resourceUpdates);
      m_overlayImage = QImage();
    }
    return;
  }

  if (m_core && m_coreInitialized && m_currentWaitFrames > 0) {
    m_currentWaitFrames--;
    // update();
    return;
  }

  m_currentWaitFrames = m_waitFrames;

  if (m_core && !m_coreInitialized) {
    const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
    QRhiResourceUpdateBatch *resourceUpdates = rhi()->nextResourceUpdateBatch();
    cb->beginPass(renderTarget(), clearColor, {1.0f, 0}, resourceUpdates,
                  QRhiCommandBuffer::ExternalContent);
    cb->beginExternal();
    m_core->init();

    libretro::Game game(
        m_contentPath.toStdString(),
        vector<unsigned char>(m_gameData.begin(), m_gameData.end()));
    m_core->loadGame(&game);

    if (m_saveData.size() > 0) {
      m_core->writeMemoryData(libretro::SAVE_RAM,
                              vector(m_saveData.begin(), m_saveData.end()));
    }

    getAchievementManager()->loadGame(m_platformId, m_contentHash);

    // m_encoder = new firelight::av::VideoEncoder(640, 480, 60);
    // m_decoder = new firelight::av::VideoDecoder();
    // m_recorder.record();
    cb->endExternal();
    cb->endPass(resourceUpdates);

    m_coreInitialized = true;
    m_playSession.contentHash = m_contentHash.toStdString();
    m_playSession.startTime = QDateTime::currentMSecsSinceEpoch();
    m_playSession.slotNumber = m_saveSlotNumber;

    if (!m_paused) {
      m_playSessionTimer.start();
    }

    update();
  } else if (!m_paused && m_core && m_coreInitialized && m_shouldRunFrame) {
    // m_shouldRunFrame = false;
    // m_frameNumber++;

    if (m_shouldSave) {
      save(false);
      m_shouldSave = false;
    }

    const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
    QRhiResourceUpdateBatch *resourceUpdates = rhi()->nextResourceUpdateBatch();
    cb->beginPass(renderTarget(), clearColor, {1.0f, 0}, resourceUpdates,
                  QRhiCommandBuffer::ExternalContent);
    m_currentUpdateBatch = resourceUpdates;

    cb->beginExternal();
    for (int i = 0; i < m_playbackMultiplier; i++) {
      m_core->run(0);
      getAchievementManager()->doFrame(m_core.get());
    }

    update();
    cb->endExternal();

    // if (m_frameNumber == 1000) {
    auto *rbResult = new QRhiReadbackResult;
    rbResult->completed = [this, rbResult] {
      // {
      const QImage::Format fmt =
          QImage::Format_RGBA8888_Premultiplied; // fits QRhiTexture::RGBA8
      const auto *p =
          reinterpret_cast<const uchar *>(rbResult->data.constData());
      m_currentImage = QImage(p, rbResult->pixelSize.width(),
                              rbResult->pixelSize.height(), fmt);

      if (m_graphicsApi == QSGRendererInterface::OpenGL) {
        m_currentImage.mirror();
      }

      // m_currentImage.convertToFormat(QImage::Format)
      //
      // frame.pixelFormat();
      //
      // if (frame.isValid()) {
      //   frame.map(QVideoFrame::ReadOnly);
      //   printf("plane count: %d\n", frame.planeCount());
      //   printf("num bytes in plane 0: %d\n", frame.mappedBytes(0));
      //   printf("num bytes in plane 1: %d\n", frame.mappedBytes(1));
      //   printf("num bytes in plane 2: %d\n", frame.mappedBytes(2));
      //   printf("num bytes in plane 3: %d\n", frame.mappedBytes(3));
      //   printf("num bytes in plane 4: %d\n", frame.mappedBytes(4));
      //   printf("num bytes in plane 5: %d\n", frame.mappedBytes(5));
      //   printf("num bytes in plane 6: %d\n", frame.mappedBytes(6));
      //   printf("num bytes in plane 7: %d\n", frame.mappedBytes(7));
      //   printf("num bytes in plane 8: %d\n", frame.mappedBytes(8));
      //   printf("num bytes in plane 9: %d\n", frame.mappedBytes(9));
      //   printf("num bytes in plane 10: %d\n", frame.mappedBytes(10));
      //
      //
      //   const auto result = m_encoder->encode(frame.bits(0));
      //   printf("Encoded frame size: %llu bytes\n", result.size());
      //
      //   std::vector<uint8_t> decodedFrame;
      //   if (m_decoder->decode(result.data(), result.size(), decodedFrame)) {
      //     printf("Decoded frame size: %llu bytes\n", decodedFrame.size());
      //
      //     // Use decoded frame (YUV420P format)
      //     // For example, render it or save it
      //   } else {
      //     printf("No frame decoded yet\n");
      //   }
      // }

      // image.save("result.png");
      delete rbResult;
    };

    QRhiReadbackDescription rb(colorTexture());
    resourceUpdates->readBackTexture(rb, rbResult);
    // }

    m_currentUpdateBatch = nullptr;
    cb->endPass(resourceUpdates);
  }

  m_measureTime = true;

  // spdlog::error(
  //     "Finishing frame: {}",
  //     std::chrono::high_resolution_clock::now().time_since_epoch().count());

  // spdlog::info("Time to run frame: {}", m_emulationTimer.nsecsElapsed());
  // m_emulationWorkTimeBuffer.push_back(m_emulationTimer.nsecsElapsed());
  // if (m_emulationWorkTimeBuffer.size() > 40) {
  //   m_emulationWorkTimeBuffer.pop_front();
  // }
  //
  // // get average
  // auto sum = std::accumulate(m_emulationWorkTimeBuffer.begin(),
  //                            m_emulationWorkTimeBuffer.end(), 0);
  // m_averageEmulationTime = sum / m_emulationWorkTimeBuffer.size();
}

void EmulatorItemRenderer::save(const bool waitForFinish) const {
  spdlog::debug("Saving game data");
  firelight::saves::Savefile saveData(
      m_core->getMemoryData(libretro::SAVE_RAM));
  saveData.setImage(m_currentImage.copy());

  QFuture<bool> result = getSaveManager()->writeSaveData(
      m_contentHash, m_saveSlotNumber, saveData);

  if (waitForFinish) {
    result.waitForFinished();
  }
}
