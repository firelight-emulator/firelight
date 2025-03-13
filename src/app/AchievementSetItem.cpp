#include "AchievementSetItem.hpp"

namespace firelight::achievements {

QString AchievementSetItem::getContentHash() const { return m_contentHash; }

void AchievementSetItem::setContentHash(const QString &contentHash) {
  if (m_contentHash == contentHash) {
    return;
  }

  m_contentHash = contentHash;
  emit contentHashChanged();
}
int AchievementSetItem::getNumAchievements() const { return m_numAchievements; }
int AchievementSetItem::getTotalNumPoints() const { return m_totalNumPoints; }
QString AchievementSetItem::getSetName() const {
  return m_setName;
}
} // namespace firelight::achievements