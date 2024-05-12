#pragma once

#include "rcheevos/rc_client.h"
#include "rcheevos/rc_libretro.h"
#include <QTimer>
#include <firelight/library_entry.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>

namespace libretro {
class Core;
}

namespace firelight::achievements {

class RAClient : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool loggedIn READ loggedIn NOTIFY loginStatusChanged)
  Q_PROPERTY(QString displayName READ displayName NOTIFY loginSucceeded)
  // Q_PROPERTY(bool gameLoaded READ gameLoaded NOTIFY gameLoadSucceeded)

public:
  RAClient();
  ~RAClient() override;

  Q_INVOKABLE void logout();

  void logInUserWithToken(const std::string &username,
                          const std::string &token);

  void initializeMemory(::libretro::Core *core);

  [[nodiscard]] bool loggedIn() const;
  QString displayName();
  void doFrame(::libretro::Core *core, const db::LibraryEntry &currentEntry);
  rc_libretro_memory_regions_t m_memoryRegions{};
  bool m_memorySeemsGood = false;

  // bool gameLoaded() const;

signals:
  void loginSucceeded();
  void loginFailedWithInvalidCredentials();
  void loginFailedWithExpiredToken();
  void loginFailedWithAccessDenied();
  void loginFailedWithInternalError();
  void loginStatusChanged();

  void achievementUnlocked();

  void gameLoadSucceeded();
  void gameLoadFailed();
  void gameUnloaded();

  void achievementProgressUpdated(int achievementId, int current, int desired);
  void achievementProgressPercentUpdated(int achievementId, float percent);

public slots:
  void logInUserWithPassword(const QString &username, const QString &password);
  void loadGame(const QString &contentMd5);
  void unloadGame();

private:
  QString m_displayName;
  bool m_loggedIn = false;
  bool m_gameLoaded = false;
  rc_client_t *m_client;

  int m_frameNumber = 0;
  QTimer m_idleTimer;
};

} // namespace firelight::achievements