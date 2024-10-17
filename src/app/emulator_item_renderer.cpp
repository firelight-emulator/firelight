#include "emulator_item_renderer.hpp"

#include <QOpenGLPaintDevice>
#include <QPainter>
#include <spdlog/spdlog.h>
#include <rhi/qrhi.h>

#include "emulator_item.hpp"

EmulatorItemRenderer::EmulatorItemRenderer(const QSGRendererInterface::GraphicsApi api,
                                           libretro::Core *core) : m_graphicsApi(api), m_core(core) {
}

EmulatorItemRenderer::~EmulatorItemRenderer() {
    // Don't need to destroy the context here as it is handled by the Core object.
}

void EmulatorItemRenderer::receive(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (data && data != RETRO_HW_FRAME_BUFFER_VALID && width > 0 && height > 0 && pitch > 0) {
        if (m_graphicsApi == QSGRendererInterface::OpenGL) {
            QOpenGLPaintDevice paint_device;
            auto size = renderTarget()->pixelSize();
            paint_device.setSize({static_cast<int>(m_coreBaseWidth), static_cast<int>(m_coreBaseHeight)});
            QPainter painter(&paint_device);

            painter.beginNativePainting();
            const QImage image((uchar *) data, width, height, pitch, m_pixelFormat);
            painter.drawImage(
                QRect(0, 0, m_coreBaseWidth, m_coreBaseHeight), image,
                image.rect());

            painter.endNativePainting();
        }
    }
}

retro_hw_context_type EmulatorItemRenderer::getPreferredHwRender() {
    return RETRO_HW_CONTEXT_OPENGLES_VERSION;
}

void EmulatorItemRenderer::getHwRenderContext(retro_hw_context_type &contextType, unsigned int &major,
                                              unsigned int &minor) {
    // contextType = RETRO_HW_CONTEXT_OPENGLES_VERSION;
    // major = 3;
    // minor = 1;
}

proc_address_t EmulatorItemRenderer::getProcAddress(const char *sym) {
    return QOpenGLContext::currentContext()->getProcAddress(sym);
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

void EmulatorItemRenderer::initialize(QRhiCommandBuffer *cb) {
    if (m_graphicsApi == QSGRendererInterface::OpenGL) {
        if (!m_openGlInitialized) {
            initializeOpenGLFunctions();
            m_openGlInitialized = true;
        }

        if (m_resetContextFunction) {
            QRhiResourceUpdateBatch *resourceUpdates = rhi()->nextResourceUpdateBatch();
            cb->beginPass(renderTarget(), {0, 0, 0, 0}, {1.0f, 0}, resourceUpdates, QRhiCommandBuffer::ExternalContent);
            cb->beginExternal();

            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_currentFramebufferId);

            m_resetContextFunction();
            m_resetContextFunction = nullptr;

            cb->endExternal();
            cb->endPass(resourceUpdates);
        }
    }
}

void EmulatorItemRenderer::synchronize(QQuickRhiItem *item) {
    auto emulatorItem = dynamic_cast<EmulatorItem *>(item);

    m_paused = emulatorItem->paused();

    m_gameData = emulatorItem->m_gameData;
    m_saveData = emulatorItem->m_saveData;
    m_corePath = emulatorItem->m_corePath;
    m_contentHash = emulatorItem->m_contentHash;
    m_saveSlotNumber = emulatorItem->m_saveSlotNumber;
    m_platformId = emulatorItem->m_platformId;
    m_contentPath = emulatorItem->m_contentPath;
    m_gameReady = emulatorItem->m_gameReady;
}

void EmulatorItemRenderer::render(QRhiCommandBuffer *cb) {
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
        libretro::Game game(
            m_contentPath.toStdString(),
            vector<unsigned char>(m_gameData.begin(), m_gameData.end()));
        m_core->loadGame(&game);

        if (m_saveData.size() > 0) {
            m_core->writeMemoryData(libretro::SAVE_RAM,
                                    vector(m_saveData.begin(), m_saveData.end()));
        }

        m_coreInitialized = true;
    } else if (m_core && m_coreInitialized) {
        const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
        QRhiResourceUpdateBatch *resourceUpdates = rhi()->nextResourceUpdateBatch();
        cb->beginPass(renderTarget(), clearColor, {1.0f, 0}, resourceUpdates, QRhiCommandBuffer::ExternalContent);
        cb->beginExternal();
        m_core->run(0);
        update();

        cb->endExternal();
        cb->endPass(resourceUpdates);
    }
}
