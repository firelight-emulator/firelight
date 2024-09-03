#pragma once

#include <QAbstractItemModel>

#include "rcheevos/rc_client.h"
#include "ra_http_client.hpp"
#include "rcheevos/rc_libretro.h"
#include "offline/ra_cache.hpp"
#include <QTimer>
#include <QJsonObject>
#include <firelight/content_database.hpp>
#include <firelight/library_entry.hpp>
#include <qsettings.h>
#include <spdlog/spdlog.h>
#include <QMap>
#include <string>
#include <utility>

#include "regular_http_client.hpp"
#include "../../gui/achievement_list_sort_filter_model.hpp"

namespace libretro {
  class Core;
}

namespace firelight::achievements {
  class RAClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected MEMBER m_connected NOTIFY connectedChanged)
    Q_PROPERTY(bool expectToBeLoggedIn READ expectToBeLoggedIn NOTIFY loginStatusChanged)
    Q_PROPERTY(bool loggedIn MEMBER m_loggedIn NOTIFY loginStatusChanged)
    Q_PROPERTY(QString displayName MEMBER m_displayName NOTIFY loginSucceeded)
    Q_PROPERTY(QString avatarUrl READ avatarUrl NOTIFY loginStatusChanged)
    Q_PROPERTY(bool defaultToHardcore MEMBER m_defaultToHardcore WRITE
      setDefaultToHardcore NOTIFY defaultModeChanged)
    Q_PROPERTY(int points READ numPoints NOTIFY pointsChanged)
    Q_PROPERTY(
      bool unlockNotificationsEnabled MEMBER m_unlockNotificationsEnabled WRITE
      setUnlockNotificationsEnabled NOTIFY notificationSettingsChanged)
    Q_PROPERTY(
      bool progressNotificationsEnabled MEMBER m_progressNotificationsEnabled
      WRITE setProgressNotificationsEnabled NOTIFY
      notificationSettingsChanged)
    Q_PROPERTY(
      bool challengeIndicatorsEnabled MEMBER m_challengeIndicatorsEnabled WRITE
      setChallengeIndicatorsEnabled NOTIFY notificationSettingsChanged)
    // Q_PROPERTY(bool gameLoaded READ gameLoaded NOTIFY gameLoadSucceeded)

  public:
    explicit RAClient(db::IContentDatabase &contentDb);

    ~RAClient() override;

    int numPoints() const;

    QString avatarUrl() const;

    bool expectToBeLoggedIn() const;

    void doFrame(::libretro::Core *core, const db::LibraryEntry &currentEntry);

    rc_libretro_memory_regions_t m_memoryRegions{};
    bool m_memorySeemsGood = false;
    int m_consoleId = 0;
    bool m_connected = false;

    void setDefaultToHardcore(bool hardcore);

    void setUnlockNotificationsEnabled(bool enabled);

    void setProgressNotificationsEnabled(bool enabled);

    void setChallengeIndicatorsEnabled(bool enabled);

    Q_INVOKABLE QAbstractItemModel *getAchievementsModelForGameId(int gameId);

    Q_INVOKABLE void getAchievementsOverview(int gameId);

    Q_INVOKABLE void setOnlineForTesting(bool online) const;

    std::unique_ptr<RegularHttpClient> m_httpClient = nullptr;

    // bool gameLoaded() const;

  signals:
    void connectedChanged();

    void loginSucceeded();

    void loginFailedWithInvalidCredentials();

    void loginFailedWithExpiredToken();

    void loginFailedWithAccessDenied();

    void loginFailedWithInternalError();

    void loginStatusChanged();

    void achievementUnlocked(QString imageUrl, QString title,
                             QString description);

    void gameLoadSucceeded(QString imageUrl, QString title, int numEarned,
                           int numTotal);

    void achievementSummaryAvailable(QJsonObject summary);

    void gameLoadFailed();

    void gameUnloaded();

    void pointsChanged();

    void defaultModeChanged();

    void achievementProgressUpdated(QString imageUrl, int achievementId,
                                    QString title, QString description,
                                    int current, int desired);

    void achievementProgressPercentUpdated(int achievementId, float percent);

    void showChallengeIndicator(int id, QString imageUrl, QString title,
                                QString description);

    void hideChallengeIndicator(int id);

    void notificationSettingsChanged();

  public slots:
    void logout();

    void logInUserWithPassword(const QString &username, const QString &password);

    void logInUserWithToken(const QString &username, const QString &token);

    void loadGame(int platformId, const QString &contentMd5);

    void unloadGame();

  private:
    QString m_displayName;
    bool m_loggedIn = false;
    bool m_gameLoaded = false;
    bool m_unlockNotificationsEnabled = true;
    bool m_progressNotificationsEnabled = true;
    bool m_challengeIndicatorsEnabled = true;
    bool m_defaultToHardcore = true;
    bool m_expectToBeLoggedIn = false;

    db::IContentDatabase &m_contentDb;

    QHash<int, std::shared_ptr<gui::AchievementListSortFilterModel> > m_achievementModels;

    std::shared_ptr<RetroAchievementsCache> m_cache = nullptr;

    rc_client_t *m_client;

    int m_frameNumber = 0;
    QTimer m_idleTimer;

    std::unique_ptr<QSettings> m_settings;
  };
} // namespace firelight::achievements
