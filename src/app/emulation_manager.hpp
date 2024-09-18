#pragma once

#include "library/library_scanner.hpp"
#include "libretro/core.hpp"
#include "manager_accessor.hpp"
#include "saves/suspend_point.hpp"

#include <QOpenGLTexture>
#include <QQuickFramebufferObject>
#include <firelight/play_session.hpp>

#include "libretro/core_configuration.hpp"

namespace firelight::emulation {
    class SuspendPointListModel;
}

class EmulationManager : public QQuickFramebufferObject,
                         public firelight::ManagerAccessor {
    Q_OBJECT
    Q_PROPERTY(QString currentGameName READ currentGameName NOTIFY
        currentGameNameChanged)
    Q_PROPERTY(int nativeWidth READ nativeWidth NOTIFY nativeWidthChanged)
    Q_PROPERTY(int nativeHeight READ nativeHeight NOTIFY nativeHeightChanged)
    Q_PROPERTY(float nativeAspectRatio READ nativeAspectRatio NOTIFY
        nativeAspectRatioChanged)
    Q_PROPERTY(float playSpeedMultiplier READ playSpeed WRITE setPlaySpeed NOTIFY
        playSpeedMultiplierChanged)

public:
    [[nodiscard]] Renderer *createRenderer() const override;

    explicit EmulationManager(QQuickItem *parent = nullptr);

    ~EmulationManager() override;

    [[nodiscard]] QString currentGameName() const;

    [[nodiscard]] int nativeWidth() const;

    [[nodiscard]] int nativeHeight() const;

    [[nodiscard]] float nativeAspectRatio() const;

    [[nodiscard]] float playSpeed() const;

    void setPlaySpeed(float speed);

    void setGeometry(int nativeWidth, int nativeHeight, float nativeAspectRatio);

    void setIsRunning(bool running);

    void setRewindModel(firelight::emulation::SuspendPointListModel &model);

    QByteArray m_gameData;
    QByteArray m_saveData;
    QString m_corePath;
    firelight::db::LibraryEntry m_currentEntry;
    std::shared_ptr<CoreConfiguration> m_coreConfiguration = nullptr;
    bool m_paused = false;
    bool m_shouldReset = false;

    bool m_running = false;

    bool m_gameReady = false;

    bool m_shouldCreateSuspendPoint = false;

    bool m_shouldGetRewindPoints = false;

    int m_rewindPointIndex = -1;

    int m_writeSuspendPointIndex = -1;
    int m_loadSuspendPointIndex = -1;
    bool m_shouldLoadLastSuspendPoint = false;

    float m_playSpeed = 1.0;

public slots:
    void loadLibraryEntry(int entryId);

    void pauseGame();

    void resumeGame();

    void resetEmulation();

    void createSuspendPoint();

    void createRewindPoints();

    void loadRewindPoint(int index);

    void writeSuspendPoint(int index);

    void loadSuspendPoint(int index);

    void loadLastSuspendPoint();

signals:
    void gameLoadSucceeded();

    void gameLoadFailed();

    void readyToStart();

    void gamePaused();

    void gameResumed();

    void emulationStarted();

    void emulationStopped();

    void currentGameNameChanged();

    void nativeWidthChanged();

    void nativeHeightChanged();

    void nativeAspectRatioChanged();

    void rewindPointsReady(QList<QJsonObject> points);

    void rewindPointLoaded();

    void playSpeedMultiplierChanged();

private:
    std::unique_ptr<firelight::db::PlaySession> m_currentPlaySession;
    std::vector<SuspendPoint> m_suspendPoints;
    QElapsedTimer m_playtimeTimer;
    QTimer m_autosaveTimer;

    int m_nativeWidth = 0;
    int m_nativeHeight = 0;
    float m_nativeAspectRatio = 0;
};
