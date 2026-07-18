#include "qt_achievement_service_proxy.hpp"

#include <firelight/achievement_service.hpp>

namespace firelight::gui {
QtAchievementServiceProxy::QtAchievementServiceProxy(achievements::AchievementService &achievementService,
                                                     QObject *parent)
    : QObject(parent), m_achievementService(&achievementService) {
  m_sessionStartedConnection = EventDispatcher::instance().subscribe<achievements::AchievementSessionStartedEvent>(
      [this](const achievements::AchievementSessionStartedEvent &event) { emit inHardcoreSessionChanged(); });

  m_sessionEndedConnection = EventDispatcher::instance().subscribe<achievements::AchievementSessionEndedEvent>(
      [this](const achievements::AchievementSessionEndedEvent &event) { emit inHardcoreSessionChanged(); });
}

bool QtAchievementServiceProxy::isLoggedIn() const { return m_achievementService->getLoggedInUsername() != ""; }

bool QtAchievementServiceProxy::inHardcoreSession() const { return m_achievementService->inHardcoreSession(); }

unsigned QtAchievementServiceProxy::numCurrentSessionHardcoreUnlocks() const {
  return m_achievementService->getNumCurrentSessionHardcoreUnlocks();
}
} // namespace firelight::gui
