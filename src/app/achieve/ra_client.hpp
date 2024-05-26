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
    Q_PROPERTY(bool loggedIn READ loggedIn NOTIFY loginStatusChanged)
    Q_PROPERTY(QString displayName READ displayName NOTIFY loginSucceeded)
    Q_PROPERTY(QString avatarUrl READ avatarUrl NOTIFY loginStatusChanged)
    Q_PROPERTY(int points READ numPoints NOTIFY pointsChanged)
    // Q_PROPERTY(bool gameLoaded READ gameLoaded NOTIFY gameLoadSucceeded)

  public:
    RAClient();

    ~RAClient() override;

    Q_INVOKABLE void logout();

    [[nodiscard]] bool loggedIn() const;

    QString displayName();

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

    void achievementUnlocked(QString title, QString description);

    void gameLoadSucceeded();

    void gameLoadFailed();

    void gameUnloaded();

    void pointsChanged();

    void achievementProgressUpdated(QString imageUrl, int achievementId,
                                    QString title, QString description,
                                    int current, int desired);

    void achievementProgressPercentUpdated(int achievementId, float percent);

  public slots:
    void logInUserWithPassword(const QString &username, const QString &password);

    void logInUserWithToken(const QString &username, const QString &token);

    void loadGame(const QString &contentMd5);

    void unloadGame();

  private:
    QString m_displayName;
    bool m_loggedIn = false;
    bool m_gameLoaded = false;
    rc_client_t *m_client;

    int m_frameNumber = 0;
    QTimer m_idleTimer;

    std::unique_ptr<QSettings> m_settings;
  };
} // namespace firelight::achievements
