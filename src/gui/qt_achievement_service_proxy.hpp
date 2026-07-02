#pragma once
#include <QObject>
#include <firelight/event_dispatcher.hpp>

namespace firelight::achievements {
class AchievementService;
}

namespace firelight::gui {

class QtAchievementServiceProxy : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool loggedIn READ isLoggedIn NOTIFY loggedInChanged)
  Q_PROPERTY(bool inHardcoreSession READ inHardcoreSession NOTIFY
                 inHardcoreSessionChanged)
  Q_PROPERTY(unsigned numCurrentSessionHardcoreUnlocks READ
                 numCurrentSessionHardcoreUnlocks NOTIFY
                     numCurrentSessionHardcoreUnlocksChanged)
public:
  explicit QtAchievementServiceProxy(
      achievements::AchievementService &achievementService,
      QObject *parent = nullptr);

  bool isLoggedIn() const;
  bool inHardcoreSession() const;
  unsigned numCurrentSessionHardcoreUnlocks() const;
signals:
  void loggedInChanged();
  void inHardcoreSessionChanged();
  void numCurrentSessionHardcoreUnlocksChanged();

private:
  achievements::AchievementService *m_achievementService;
  ScopedConnection m_sessionStartedConnection;
  ScopedConnection m_sessionEndedConnection;
};

} // namespace firelight::gui