#pragma once

#include <QQuickItem>
#include <QQuickRhiItem>
#include <firelight/libretro/video_data_receiver.hpp>

#include "audio_manager.hpp"
#include "emulator_item_renderer.hpp"
#include "manager_accessor.hpp"
#include "libretro/core_configuration.hpp"

class EmulatorItem : public QQuickRhiItem, public firelight::ManagerAccessor {
protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    Q_OBJECT
    Q_PROPERTY(int videoWidth MEMBER m_coreBaseWidth NOTIFY videoWidthChanged)
    Q_PROPERTY(int videoHeight MEMBER m_coreBaseHeight NOTIFY videoHeightChanged)
    Q_PROPERTY(float videoAspectRatio MEMBER m_calculatedAspectRatio NOTIFY videoAspectRatioChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)

public:
    explicit EmulatorItem(QQuickItem *parent = nullptr);

    ~EmulatorItem() override;

    QByteArray m_gameData;
    QByteArray m_saveData;
    QString m_corePath;
    QString m_contentHash;
    unsigned int m_saveSlotNumber;
    unsigned int m_platformId;
    QString m_contentPath;
    bool m_gameReady;

    // Emulator state
    bool m_paused = false;

    uint m_coreBaseWidth = 0;
    uint m_coreBaseHeight = 0;
    uint m_coreMaxWidth = 0;
    uint m_coreMaxHeight = 0;
    float m_coreAspectRatio = 0.0f;
    float m_calculatedAspectRatio = 0.0f;

    std::shared_ptr<libretro::Core> m_core = nullptr;
    std::shared_ptr<AudioManager> m_audioManager = nullptr;
    std::shared_ptr<CoreConfiguration> m_coreConfiguration = nullptr;

    [[nodiscard]] bool paused() const;

    void setPaused(bool paused);

public slots:
    void startGame(const QByteArray &gameData, const QByteArray &saveData, const QString &corePath,
                   const QString &contentHash,
                   unsigned int saveSlotNumber, unsigned int platformId, const QString &contentPath);

signals:
    void pausedChanged();

    void videoWidthChanged();

    void videoHeightChanged();

    void videoAspectRatioChanged();

protected:
    QQuickRhiItemRenderer *createRenderer() override;

private:
    EmulatorItemRenderer *m_renderer = nullptr;

    void updateGeometry(int width, int height, float aspectRatio);
};
