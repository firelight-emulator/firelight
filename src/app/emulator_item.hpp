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
    Q_PROPERTY(float videoAspectRatio MEMBER m_coreAspectRatio NOTIFY videoAspectRatioChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(float audioBufferLevel READ audioBufferLevel NOTIFY audioBufferLevelChanged)

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

    // std::shared_ptr<libretro::Core> m_core = nullptr;
    std::shared_ptr<AudioManager> m_audioManager = nullptr;
    std::shared_ptr<CoreConfiguration> m_coreConfiguration = nullptr;

    [[nodiscard]] bool paused() const;

    void setPaused(bool paused);

    [[nodiscard]] float audioBufferLevel() const;

    Q_INVOKABLE void resetGame();

    Q_INVOKABLE void writeSuspendPoint(int index);

    Q_INVOKABLE void loadSuspendPoint(int index);

    Q_INVOKABLE void createRewindPoints();

protected:
    void hoverMoveEvent(QHoverEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

public slots:
    void startGame(const QByteArray &gameData, const QByteArray &saveData, const QString &corePath,
                   const QString &contentHash,
                   unsigned int saveSlotNumber, unsigned int platformId, const QString &contentPath);

signals:
    void pausedChanged();

    void videoWidthChanged();

    void videoHeightChanged();

    void videoAspectRatioChanged();

    void rewindPointsReady(QList<QJsonObject> points);

    void audioBufferLevelChanged();

protected:
    QQuickRhiItemRenderer *createRenderer() override;

private:
    QTimer m_autosaveTimer;
    QTimer m_rewindPointTimer;
    EmulatorItemRenderer *m_renderer = nullptr;
    bool m_mousePressed = false;

    void updateGeometry(unsigned int width, unsigned int height, float aspectRatio);
};
