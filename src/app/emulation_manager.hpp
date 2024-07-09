#pragma once

#include "library/library_scanner.hpp"
#include "libretro/core.hpp"
#include "manager_accessor.hpp"
#include "saves/suspend_point.hpp"

#include <QOpenGLTexture>
#include <QQuickFramebufferObject>
#include <firelight/play_session.hpp>

class EmulationManager : public QQuickFramebufferObject,
                         public firelight::ManagerAccessor {
    Q_OBJECT
    Q_PROPERTY(QString currentGameName READ currentGameName NOTIFY
        currentGameNameChanged)
    Q_PROPERTY(int nativeWidth READ nativeWidth NOTIFY nativeWidthChanged)
    Q_PROPERTY(int nativeHeight READ nativeHeight NOTIFY nativeHeightChanged)
    Q_PROPERTY(float nativeAspectRatio READ nativeAspectRatio NOTIFY
        nativeAspectRatioChanged)

public:
    [[nodiscard]] Renderer *createRenderer() const override;

    explicit EmulationManager(QQuickItem *parent = nullptr);

    ~EmulationManager() override;

    [[nodiscard]] QString currentGameName() const;

    [[nodiscard]] int nativeWidth() const;

    [[nodiscard]] int nativeHeight() const;

    [[nodiscard]] float nativeAspectRatio() const;

    void setGeometry(int nativeWidth, int nativeHeight, float nativeAspectRatio);

    QByteArray m_gameData;
    QByteArray m_saveData;
    QString m_corePath;
    firelight::db::LibraryEntry m_currentEntry;
    bool m_paused = false;

    bool m_running = false;

    bool m_gameReady = false;

    void setIsRunning(bool running);

public slots:
    void loadLibraryEntry(int entryId);

    void pauseGame();

    void resumeGame();

    void resetEmulation();

    void save(bool waitForFinish = false);

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

    void loadAchievements(QString contentId);

private:
    bool shouldUnload = false;

    std::shared_ptr<libretro::Core> m_core;

    bool m_gameLoadedSignalReady = false;
    bool m_achievementsLoadedSignalReady = false;

    std::unique_ptr<firelight::db::PlaySession> m_currentPlaySession;
    std::vector<SuspendPoint> m_suspendPoints;
    QElapsedTimer m_playtimeTimer;
    QTimer m_autosaveTimer;

    int m_nativeWidth = 0;
    int m_nativeHeight = 0;
    float m_nativeAspectRatio = 0;

    int numFramesRun = 0;
};
