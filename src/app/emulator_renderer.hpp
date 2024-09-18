#pragma once

#include "emulation_manager.hpp"
#include "libretro/core.hpp"
#include "manager_accessor.hpp"

#include <QOpenGLFunctions>
#include <QQuickFramebufferObject>

#include "audio_manager.hpp"
#include "../gui/models/suspend_point_list_model.hpp"
#include "libretro/core_configuration.hpp"

class EmulatorRenderer final : public QQuickFramebufferObject::Renderer,
                               public QOpenGLFunctions,
                               public firelight::ManagerAccessor,
                               public firelight::libretro::IVideoDataReceiver {
public:
    explicit EmulatorRenderer(const std::function<void()> &updateFunc);

    void receive(const void *data, unsigned width, unsigned height, size_t pitch) override;

    proc_address_t getProcAddress(const char *sym) override;

    void setResetContextFunc(context_reset_func) override;

    void setDestroyContextFunc(context_destroy_func) override;

    uintptr_t getCurrentFramebufferId() override;

    void setSystemAVInfo(retro_system_av_info *info) override;

    void setPixelFormat(retro_pixel_format *format) override;

protected:
    ~EmulatorRenderer() override;

    void synchronize(QQuickFramebufferObject *fbo) override;

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

    void render() override;

private:
    void save(bool waitForFinish = false);

    std::unique_ptr<libretro::Core> m_core = nullptr;
    std::shared_ptr<CoreConfiguration> m_coreConfiguration = nullptr;

    std::shared_ptr<AudioManager> m_audioManager = nullptr;

    QOpenGLFramebufferObject *m_fbo = nullptr;
    bool m_fboIsNew = true;

    QByteArray m_gameData;
    QByteArray m_saveData;
    QString m_corePath;
    firelight::db::LibraryEntry m_currentEntry;

    // Default according to libretro docs
    QImage::Format m_pixelFormat = QImage::Format_RGB16;

    bool m_paused = false;
    bool m_gameReady = false;

    bool m_running = false;

    std::unique_ptr<SuspendPoint> m_lastSuspendPoint = nullptr;

    QList<SuspendPoint> m_suspendPoints;

    bool m_usingHwRendering = false;

    std::function<void()> m_runAfterNextFrame = nullptr;

    QTimer suspendPointTimer;
    bool m_shouldCreateSuspendPoint = false;
    QTimer autosaveTimer;
    bool m_shouldSave = false;
    bool m_runAFrame = false;

    int m_nativeWidth = 0;
    int m_nativeHeight = 0;
    float m_nativeAspectRatio = 0.0f;

    float m_gameSpeed = 1.0f;

    bool m_doingFrameByFrame = false;
    bool m_pausedForFrameByFrame = false;

    std::function<void()> m_updateFunc = nullptr;

    std::function<void()> m_resetContextFunction = nullptr;
    std::function<void()> m_destroyContextFunction = nullptr;
};
