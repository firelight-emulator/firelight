#pragma once

#include <QSGRenderNode>
#include <QOpenGLFunctions>
#include <QImage>
#include <QOpenGLFramebufferObject>
#include <QQuickRhiItemRenderer>
#include <qsgrendererinterface.h>
#include <firelight/libretro/video_data_receiver.hpp>

#include "audio_manager.hpp"
#include "manager_accessor.hpp"
#include "libretro/core.hpp"
#include "libretro/core_configuration.hpp"

class EmulatorItemRenderer : public QQuickRhiItemRenderer, public QOpenGLFunctions,
                             public firelight::libretro::IVideoDataReceiver, public firelight::ManagerAccessor {
public:
    explicit EmulatorItemRenderer(QSGRendererInterface::GraphicsApi api, libretro::Core *core);

    void onGeometryChanged(const std::function<void(int, int, float)> &callback) {
        m_geometryChangedCallback = callback;
    }

    void receive(const void *data, unsigned width, unsigned height, size_t pitch) override;

    retro_hw_context_type getPreferredHwRender() override;

    void getHwRenderContext(retro_hw_context_type &contextType, unsigned int &major, unsigned int &minor) override;

    proc_address_t getProcAddress(const char *sym) override;

    void setResetContextFunc(context_reset_func contextResetFunc) override;

    void setDestroyContextFunc(context_destroy_func contextDestroyFunc) override;

    uintptr_t getCurrentFramebufferId() override;

    void setSystemAVInfo(retro_system_av_info *info) override;

    void setPixelFormat(retro_pixel_format *format) override;

    QByteArray m_gameData;
    QByteArray m_saveData;
    QString m_corePath;
    QString m_contentHash;
    int m_saveSlotNumber;
    int m_platformId;
    QString m_contentPath;
    bool m_gameReady;

protected:
    ~EmulatorItemRenderer() override;

    void initialize(QRhiCommandBuffer *cb) override;

    void synchronize(QQuickRhiItem *item) override;

    void render(QRhiCommandBuffer *cb) override;

private:
    const QSGRendererInterface::GraphicsApi m_graphicsApi;

    int m_currentWaitFrames = 0;
    int m_waitFrames = 0;

    libretro::Core *m_core = nullptr;
    bool m_coreInitialized = false;

    std::function<void (int, int, float)> m_geometryChangedCallback = nullptr;

    // OpenGL members
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
};
