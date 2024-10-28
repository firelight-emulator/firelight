#include "emulator_item_renderer.hpp"

#include <QOpenGLPaintDevice>
#include <QPainter>
#include <libretro/libretro_vulkan.h>
#include <spdlog/spdlog.h>
#include <rhi/qrhi.h>

#include <QVulkanFunctions>
#include <QVulkanDeviceFunctions>
#include "emulator_item.hpp"

static EmulatorItemRenderer *globalRenderer = nullptr;
static QRhi *globalRhi = nullptr;
static QRhiCommandBuffer *globalCb = nullptr;

EmulatorItemRenderer::EmulatorItemRenderer(const QSGRendererInterface::GraphicsApi api,
                                           libretro::Core *core) : m_graphicsApi(api), m_core(core) {
    globalRenderer = this;
}

void EmulatorItemRenderer::submitCommand(const EmulatorCommand command) {
    m_commandQueue.enqueue(command);
}

EmulatorItemRenderer::~EmulatorItemRenderer() {
    // Don't need to destroy the context here as it is handled by the Core object.
}

void EmulatorItemRenderer::receive(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (data && data != RETRO_HW_FRAME_BUFFER_VALID && width > 0 && height > 0 && pitch > 0) {
        QImage image((uchar *) data, width, height, pitch, m_pixelFormat);
        if (m_graphicsApi == QSGRendererInterface::OpenGL) {
            image.mirror();
        }
        auto newImage = image.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
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

void EmulatorItemRenderer::getHwRenderContext(retro_hw_context_type &contextType, unsigned int &major,
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

        auto vulkanHandles = reinterpret_cast<const QRhiVulkanNativeHandles *>(rhi()->nativeHandles());
        auto addr = vulkanHandles->inst->functions()->vkGetDeviceProcAddr(vulkanHandles->dev, sym);

        // spdlog::info("Address: {}", addr);
        return addr;
    }

    return nullptr;
}

void EmulatorItemRenderer::setResetContextFunc(context_reset_func contextResetFunc) {
    m_resetContextFunction = contextResetFunc;
}

void EmulatorItemRenderer::setDestroyContextFunc(context_destroy_func contextDestroyFunc) {
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
        m_calculatedAspectRatio = static_cast<float>(m_coreBaseWidth) / static_cast<float>(m_coreBaseHeight);

        if (m_geometryChangedCallback) {
            m_geometryChangedCallback(m_coreBaseWidth, m_coreBaseHeight, m_coreAspectRatio);
        }
    }
}

void EmulatorItemRenderer::setPixelFormat(retro_pixel_format *format) {
    switch (*format) {
        case RETRO_PIXEL_FORMAT_0RGB1555:
            printf("Pixel format: 0RGB1555\n");
            break;
        case RETRO_PIXEL_FORMAT_XRGB8888:
            m_pixelFormat = QImage::Format_RGB32;
            break;
        case RETRO_PIXEL_FORMAT_RGB565:
            m_pixelFormat = QImage::Format_RGB16;
            break;
        case RETRO_PIXEL_FORMAT_UNKNOWN:
            printf("Pixel format: UNKNOWN\n");
            break;
    }
}

void EmulatorItemRenderer::setHwRenderContextNegotiationInterface(
    retro_hw_render_context_negotiation_interface *inter) {
    if (m_graphicsApi == QSGRendererInterface::Vulkan) {
        spdlog::info("Doing the vulkan stuff");
        auto iface = reinterpret_cast<retro_hw_render_context_negotiation_interface_vulkan *>(inter);


        auto thingggg = *iface;
        // iface->create_device = [](retro_vulkan_context *context, VkInstance instance, VkPhysicalDevice gpu,
        //                           VkSurfaceKHR surface, PFN_vkGetInstanceProcAddr get_instance_proc_addr,
        //                           const char **required_device_extensions, unsigned num_required_device_extensions,
        //                           const char **required_device_layers, unsigned num_required_device_layers,
        //                           const VkPhysicalDeviceFeatures *required_features) {
        //     spdlog::info("Calling create_device");
        //     return false;
        // };
        // iface->create_device2 = [](retro_vulkan_context *context, VkInstance instance, VkPhysicalDevice gpu,
        //                            VkSurfaceKHR surface, PFN_vkGetInstanceProcAddr get_instance_proc_addr,
        //                            retro_vulkan_create_device_wrapper_t create_device_wrapper, void *opaque) {
        //     printf("Calling create device 2***************\n");
        //     // auto vulkanHandles = reinterpret_cast<const QRhiVulkanNativeHandles *>(rhi()->nativeHandles());
        //     // return vulkanHandles-;
        //     return false;
        // };
        //
        // iface->create_instance = [](PFN_vkGetInstanceProcAddr get_instance_proc_addr, const VkApplicationInfo *app,
        //                             retro_vulkan_create_instance_wrapper_t create_instance_wrapper, void *opaque) {
        //     spdlog::info("Calling create_instance");
        //     printf("Calling create instance***************\n");
        //     auto vulkanHandles = reinterpret_cast<const QRhiVulkanNativeHandles *>(globalRenderer->rhi()->
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

void EmulatorItemRenderer::setHwRenderInterface(retro_hw_render_interface **iface) {
    if (m_graphicsApi == QSGRendererInterface::Vulkan) {
        spdlog::info("Setting Vulkan hardware render interface");
        const auto ptr = reinterpret_cast<retro_hw_render_interface_vulkan **>(iface);
        *ptr = new retro_hw_render_interface_vulkan;

        auto vulkanHandles = reinterpret_cast<const QRhiVulkanNativeHandles *>(globalRhi->nativeHandles());

        (*ptr)->device = vulkanHandles->dev;
        (*ptr)->instance = vulkanHandles->inst->vkInstance();
        (*ptr)->gpu = vulkanHandles->physDev;
        (*ptr)->queue = vulkanHandles->gfxQueue;
        (*ptr)->queue_index = vulkanHandles->gfxQueueIdx;
        (*ptr)->get_device_proc_addr = [](VkDevice device, const char *pName) {
            spdlog::info("Getting device proc addr: {}", pName);
            auto handles = reinterpret_cast<const QRhiVulkanNativeHandles *>(globalRhi->nativeHandles());
            return handles->inst->functions()->vkGetDeviceProcAddr(device, pName);
        };

        (*ptr)->get_instance_proc_addr = [](VkInstance instance, const char *pName) {
            spdlog::info("Getting instance proc addr: {}", pName);
            auto handles = reinterpret_cast<const QRhiVulkanNativeHandles *>(globalRhi->nativeHandles());
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
            return (uint32_t) 0;
        };
        (*ptr)->get_sync_index_mask = [](void *handle) {
            spdlog::info("Calling get sync index mask");
            return (uint32_t) 4;
        };
        (*ptr)->lock_queue = [](void *handle) {
            spdlog::info("Calling lock queue");
        };
        (*ptr)->set_command_buffers = [](void *handle, uint32_t num_cmd, const VkCommandBuffer *cmd) {
            spdlog::info("Calling set command buffers");
        };
        (*ptr)->set_image = [](void *handle, const retro_vulkan_image *image, uint32_t num_semaphores,
                               const VkSemaphore *semaphores, uint32_t src_queue_family) {
            spdlog::info("Calling set image");
        };
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
    //     auto handles = reinterpret_cast<const QRhiVulkanNativeHandles *>(globalRhi->nativeHandles());
    //     retro_vulkan_context ctx{};
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
    //     m_createDeviceFunction1(&ctx, handles->inst->vkInstance(), handles->physDev,
    //                             QVulkanInstance::surfaceForWindow(m_window),
    //                             [](VkInstance instance, const char *pName) {
    //                                 spdlog::info("Getting instance proc addr: {}", pName);
    //                                 auto handles = reinterpret_cast<const QRhiVulkanNativeHandles *>(globalRhi->
    //                                     nativeHandles());
    //                                 return handles->inst->getInstanceProcAddr(pName);
    //                             }, {}, 0, {}, 0, {});
    //     m_createDeviceFunction1 = nullptr;
    // }

    // if (m_createDeviceFunction) {
    //     QRhiResourceUpdateBatch *resourceUpdates = rhi()->nextResourceUpdateBatch();
    //     cb->beginPass(renderTarget(), {0, 0, 0, 0}, {1.0f, 0}, resourceUpdates, QRhiCommandBuffer::ExternalContent);
    //     cb->beginExternal();
    //     auto handles = reinterpret_cast<const QRhiVulkanNativeHandles *>(globalRhi->nativeHandles());
    //     retro_vulkan_context ctx{};
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
    //     // auto getInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(handles->inst->getInstanceProcAddr);
    //
    //     m_createDeviceFunction(&ctx, handles->inst->vkInstance(), handles->physDev,
    //                            QVulkanInstance::surfaceForWindow(m_window),
    //                            [](VkInstance instance, const char *pName) {
    //                                spdlog::info("Getting instance proc addr: {}", pName);
    //                                auto handles = reinterpret_cast<const QRhiVulkanNativeHandles *>(globalRhi->
    //                                    nativeHandles());
    //                                return handles->inst->getInstanceProcAddr(pName);
    //                            },
    //                            [](VkPhysicalDevice gpu, void *opaque, const VkDeviceCreateInfo *create_info) {
    //                                auto myHandles = reinterpret_cast<const QRhiVulkanNativeHandles *>(globalRhi->
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
        cb->beginPass(renderTarget(), {0, 0, 0, 0}, {1.0f, 0}, resourceUpdates, QRhiCommandBuffer::ExternalContent);
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
            case ResetGame:
                m_core->reset();
                break;
            case WriteSaveFile:
                m_shouldSave = true;
                break;
            case EmitRewindPoints:
                // Handle emit rewind points
                break;
            case LoadRewindPoint:
                // Handle load rewind point
                break;
            case WriteSuspendPoint:
                // Handle write suspend point
                break;
            case LoadSuspendPoint:
                // Handle load suspend point
                break;
            case UndoLoadSuspendPoint:
                // Handle undo load suspend point
                break;
        }
    }
}

void EmulatorItemRenderer::render(QRhiCommandBuffer *cb) {
    globalCb = cb;
    // auto nativeHandles = reinterpret_cast<const QRhiGles2NativeHandles *>(rhi()->nativeHandles());
    if (m_paused) {
        return;
    }

    if (m_core && m_coreInitialized && m_currentWaitFrames > 0) {
        m_currentWaitFrames--;
        update();
        return;
    }

    m_currentWaitFrames = m_waitFrames;

    if (m_core && !m_coreInitialized) {
        const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
        QRhiResourceUpdateBatch *resourceUpdates = rhi()->nextResourceUpdateBatch();
        cb->beginPass(renderTarget(), clearColor, {1.0f, 0}, resourceUpdates, QRhiCommandBuffer::ExternalContent);
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

        cb->endExternal();
        cb->endPass(resourceUpdates);

        m_coreInitialized = true;
    } else if (m_core && m_coreInitialized) {
        // m_frameNumber++;

        if (m_shouldSave) {
            save(false);
            m_shouldSave = false;
        }

        const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
        QRhiResourceUpdateBatch *resourceUpdates = rhi()->nextResourceUpdateBatch();
        cb->beginPass(renderTarget(), clearColor, {1.0f, 0}, resourceUpdates, QRhiCommandBuffer::ExternalContent);
        m_currentUpdateBatch = resourceUpdates;

        cb->beginExternal();
        m_core->run(0);
        update();

        cb->endExternal();

        // if (m_frameNumber == 1000) {
        auto *rbResult = new QRhiReadbackResult;
        rbResult->completed = [this, rbResult] {
            {
                const QImage::Format fmt = QImage::Format_RGBA8888_Premultiplied; // fits QRhiTexture::RGBA8
                const auto *p = reinterpret_cast<const uchar *>(rbResult->data.constData());
                m_currentImage = QImage(p, rbResult->pixelSize.width(), rbResult->pixelSize.height(), fmt);

                if (m_graphicsApi == QSGRendererInterface::OpenGL) {
                    m_currentImage.mirror();
                }
                // image.save("result.png");
            }
            delete rbResult;
        };

        QRhiReadbackDescription rb(colorTexture());
        resourceUpdates->readBackTexture(rb, rbResult);
        // }

        m_currentUpdateBatch = nullptr;
        cb->endPass(resourceUpdates);
    }
}

void EmulatorItemRenderer::save(const bool waitForFinish) const {
    spdlog::debug("Saving game data\n");
    firelight::saves::Savefile saveData(
        m_core->getMemoryData(libretro::SAVE_RAM));
    saveData.setImage(m_currentImage.copy());

    QFuture<bool> result =
            getSaveManager()->writeSaveData(m_contentHash, m_saveSlotNumber, saveData);

    if (waitForFinish) {
        result.waitForFinished();
    }
}
