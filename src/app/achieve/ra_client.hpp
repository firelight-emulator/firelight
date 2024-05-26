#pragma once

#include "rcheevos/rc_client.h"
#include "rcheevos/rc_libretro.h"
#include <QTimer>
#include <firelight/library_entry.hpp>
#include <qsettings.h>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>
#include <firelight/content_database.hpp>

namespace libretro {
  class Core;
}

namespace firelight::achievements {
  class RAClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool loggedIn MEMBER m_loggedIn NOTIFY loginStatusChanged)
    Q_PROPERTY(QString displayName MEMBER m_displayName NOTIFY loginSucceeded)
    Q_PROPERTY(QString avatarUrl READ avatarUrl NOTIFY loginStatusChanged)
    Q_PROPERTY(int points READ numPoints NOTIFY pointsChanged)
    Q_PROPERTY(bool unlockNotificationsEnabled MEMBER m_unlockNotificationsEnabled NOTIFY notificationSettingsChanged)
    Q_PROPERTY(
      bool progressNotificationsEnabled MEMBER m_progressNotificationsEnabled NOTIFY notificationSettingsChanged)
    Q_PROPERTY(bool challengeIndicatorsEnabled MEMBER m_challengeIndicatorsEnabled NOTIFY notificationSettingsChanged)
    // Q_PROPERTY(bool gameLoaded READ gameLoaded NOTIFY gameLoadSucceeded)

  public:
    RAClient();

    ~RAClient() override;

    Q_INVOKABLE void logout();

    int numPoints() const;

    QString avatarUrl() const;

    void doFrame(::libretro::Core *core, const db::LibraryEntry &currentEntry);

    rc_libretro_memory_regions_t m_memoryRegions{};
    bool m_memorySeemsGood = false;
    int m_consoleId = 0;

    // bool gameLoaded() const;

  signals:
    void loginSucceeded();

    void loginFailedWithInvalidCredentials();

    void loginFailedWithExpiredToken();

    void loginFailedWithAccessDenied();

    void loginFailedWithInternalError();

    void loginStatusChanged();

    void achievementUnlocked(QString imageUrl, QString title, QString description);

    void gameLoadSucceeded();

    void gameLoadFailed();

    void gameUnloaded();

    void pointsChanged();

    void achievementProgressUpdated(QString imageUrl, int achievementId,
                                    QString title, QString description,
                                    int current, int desired);

    void achievementProgressPercentUpdated(int achievementId, float percent);

    void showChallengeIndicator(int id, QString imageUrl, QString title, QString description);

    void hideChallengeIndicator(int id);

    void notificationSettingsChanged();

  public slots:
    void logInUserWithPassword(const QString &username, const QString &password);

    void logInUserWithToken(const QString &username, const QString &token);

    void loadGame(const QString &contentMd5);

    void unloadGame();

  private:
    QString m_displayName;
    bool m_loggedIn = false;
    bool m_gameLoaded = false;
    bool m_unlockNotificationsEnabled = true;
    bool m_progressNotificationsEnabled = true;
    bool m_challengeIndicatorsEnabled = true;

    rc_client_t *m_client;


    int m_frameNumber = 0;
    QTimer m_idleTimer;

    std::unique_ptr<QSettings> m_settings;
  };
} // namespace firelight::achievements
