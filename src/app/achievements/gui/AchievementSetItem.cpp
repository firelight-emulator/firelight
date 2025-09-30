#include "AchievementSetItem.hpp"

#include <cpr/cpr.h>
#include <platforms/platform_service.hpp>
#include <rcheevos/ra_client.hpp>
#include <rcheevos/ra_constants.h>
#include <spdlog/spdlog.h>

namespace firelight::achievements {

QString AchievementSetItem::getContentHash() const { return m_contentHash; }

void AchievementSetItem::setContentHash(const QString &contentHash) {
  if (m_contentHash == contentHash) {
    return;
  }

  auto entry = getLibraryService()->getEntryWithContentHash(contentHash);
  if (!entry) {
    m_hasAchievements = false;
    return;
  }

  auto platform = getPlatformService()->getPlatform(entry->platformId);
  if (!platform) {
    m_hasAchievements = false;
    return;
  }

  m_platform = *platform;
  m_platformName = QString::fromStdString(m_platform.name);

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

    m_numEarned = 0;
    m_numEarnedHardcore = 0;
    QVector<gui::AchievementListModel::Item> items;
    for (const auto &achieve : set->achievements) {
      auto unlock = getAchievementService()->getUserUnlock(
          getAchievementService()->getLoggedInUsername(), achieve.id);
      if (!unlock) {
        continue;
      }

      items.emplace_back(gui::AchievementListModel::Item{
          .achievement = achieve, .unlockState = *unlock});

      if (unlock->earnedHardcore) {
        m_numEarnedHardcore++;
        m_numEarned++;
      } else if (unlock->earned) {
        m_numEarned++;
      }
    }

    m_achievementListModel =
        std::make_unique<gui::AchievementListModel>(items, this);
    m_achievementListModel->setHardcore(m_hardcore);
    m_sortFilterModel =
        std::make_unique<gui::AchievementListSortFilterModel>(this);
    m_sortFilterModel->setSourceModel(m_achievementListModel.get());
    m_sortFilterModel->setSortMethod("default");
    m_sortFilterModel->sort(0);
  }

  m_contentHash = contentHash;
  emit contentHashChanged();
}

gui::AchievementListSortFilterModel *
AchievementSetItem::getAchievements() const {
  return m_sortFilterModel.get();
}
} // namespace firelight::achievements