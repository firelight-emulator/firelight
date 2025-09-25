#include "AchievementSetItem.hpp"

#include <cpr/cpr.h>
#include <rcheevos/ra_client.hpp>
#include <rcheevos/ra_constants.h>
#include <spdlog/spdlog.h>

namespace firelight::achievements {

QString AchievementSetItem::getContentHash() const { return m_contentHash; }

void AchievementSetItem::setContentHash(const QString &contentHash) {
  if (m_contentHash == contentHash) {
    return;
  }

  const auto set = getAchievementService()->getAchievementSetByContentHash(
      contentHash.toStdString());
  if (!set) {
    m_hasAchievements = false;
  } else {
    m_setId = set->id;
    m_setName = QString::fromStdString(set->name);
    m_iconUrl = QString::fromStdString(set->iconUrl);
    m_numAchievements = set->numAchievements;
    m_totalNumPoints = set->totalPoints;
    m_hasAchievements = m_numAchievements > 0;

    m_numAchievementsEarned = 0;
    for (auto achieve : set->achievements) {
      auto unlock =
          getAchievementService()->getUserUnlock("BiscuitCakes", achieve.id);
      if (!unlock) {
        continue;
      }

      if (unlock->earnedHardcore) {
        m_numAchievementsEarned++;
      }

      m_totalNumPoints += achieve.points;
    }
  }

  m_contentHash = contentHash;
  emit contentHashChanged();
}

gui::AchievementListModel *AchievementSetItem::getAchievements() const {
  return {};
}
} // namespace firelight::achievements