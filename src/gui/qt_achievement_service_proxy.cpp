#include "qt_achievement_service_proxy.hpp"

#include "achievements/achievement_service.hpp"

namespace firelight::gui {
QtAchievementServiceProxy::QtAchievementServiceProxy(QObject *parent) {
  m_sessionStartedConnection =
      EventDispatcher::instance()
          .subscribe<achievements::AchievementSessionStartedEvent>(
              [this](
                  const achievements::AchievementSessionStartedEvent &event) {
                emit inHardcoreSessionChanged();
              });

  m_sessionEndedConnection =
      EventDispatcher::instance()
          .subscribe<achievements::AchievementSessionEndedEvent>(
              [this](const achievements::AchievementSessionEndedEvent &event) {
                emit inHardcoreSessionChanged();
              });
}
bool QtAchievementServiceProxy::isLoggedIn() const {
  return getAchievementService()->getLoggedInUsername() != "";
}
bool QtAchievementServiceProxy::inHardcoreSession() const {
  return getAchievementService()->inHardcoreSession();
}
unsigned QtAchievementServiceProxy::numCurrentSessionHardcoreUnlocks() const {
  return getAchievementService()->getNumCurrentSessionHardcoreUnlocks();
}
} // namespace firelight::gui