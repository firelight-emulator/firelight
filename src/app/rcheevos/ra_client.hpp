#pragma once

#include <QAbstractItemModel>

#include "offline/ra_cache.hpp"
#include "ra_http_client.hpp"
#include "rcheevos/rc_client.h"
#include "rcheevos/rc_libretro.h"
#include <QJsonObject>
#include <QMap>
#include <QTimer>
#include <firelight/content_database.hpp>
#include <library/library_entry.hpp>
#include <qsettings.h>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>

#include "achievements/gui/achievement_list_sort_filter_model.hpp"
#include "regular_http_client.hpp"

namespace libretro {
class Core;
}

namespace firelight::achievements {
class RAClient : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool connected MEMBER m_connected NOTIFY connectedChanged)
  Q_PROPERTY(
      bool expectToBeLoggedIn READ expectToBeLoggedIn NOTIFY loginStatusChanged)
  Q_PROPERTY(bool loggedIn MEMBER m_loggedIn NOTIFY loginStatusChanged)
  Q_PROPERTY(QString displayName MEMBER m_displayName NOTIFY loginSucceeded)
  Q_PROPERTY(QString avatarUrl READ avatarUrl NOTIFY loginStatusChanged)
  Q_PROPERTY(bool inHardcoreMode MEMBER m_defaultToHardcore WRITE
                 setDefaultToHardcore NOTIFY defaultModeChanged)
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
  explicit RAClient(RetroAchievementsOfflineClient &offlineClient,
                    RetroAchievementsCache &cache);

  ~RAClient() override;

  int numPoints() const;

  QString avatarUrl() const;

  bool expectToBeLoggedIn() const;

  bool loggedIn() const { return m_loggedIn; }

  void doFrame(::libretro::Core *core);

  std::vector<uint8_t> serializeState();

  void reset();

  void deserializeState(const std::vector<uint8_t> &state);

  rc_libretro_memory_regions_t m_memoryRegions{};
  bool m_memorySeemsGood = false;
  bool m_canStartReadingMemory = false;
  int m_consoleId = 0;
  bool m_connected = false;

  void setDefaultToHardcore(bool hardcore);

  void setUnlockNotificationsEnabled(bool enabled);

  void setProgressNotificationsEnabled(bool enabled);

  void setChallengeIndicatorsEnabled(bool enabled);

  Q_INVOKABLE void setOnlineForTesting(bool online) const;

  std::unique_ptr<RegularHttpClient> m_httpClient = nullptr;

  RetroAchievementsCache &cache() { return m_cache; }

  std::optional<User> getCurrentUser() const;

  bool hardcoreModeActive() const { return m_defaultToHardcore; }

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

  void gameMastered(QString imageUrl, QString title, QString description);
  void gameBeaten(QString imageUrl, QString title, QString description);

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

  void unsupportedEmulatorError();

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

  RetroAchievementsOfflineClient &m_offlineClient;
  RetroAchievementsCache &m_cache;

  rc_client_t *m_client;

  int m_frameNumber = 0;
  QTimer m_idleTimer;

  std::unique_ptr<QSettings> m_settings;
};
} // namespace firelight::achievements
