#include "AchievementSetItem.hpp"

#include <firelight/achievement_service.hpp>

#include <spdlog/spdlog.h>

namespace firelight::achievements {

AchievementSetItem::AchievementSetItem(const AchievementSet &set,
                                       QObject *parent)
    : QObject(parent) {
  m_setId = set.id;
  if (set.title.empty()) {
    m_setName = "Core Achievements";
  } else {
    m_setName = QString::fromStdString(set.title);
  }
  m_iconUrl = QString::fromStdString(set.imageIconUrl);
  m_numAchievements = set.numAchievements;
  m_totalNumPoints = set.totalPoints;
  m_hasAchievements = m_numAchievements > 0;

  m_numEarned = 0;
  m_numEarnedHardcore = 0;
  QVector<gui::AchievementListModel::Item> items;
  for (const auto &achieve : set.achievements) {
    auto unlock = getAchievementService()->getUserUnlock(
        getAchievementService()->getLoggedInUsername(), achieve.id);
    if (!unlock) {
      continue;
    }

    items.emplace_back(gui::AchievementListModel::Item{.achievement = achieve,
                                                       .unlockState = *unlock});

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

  emit setIdChanged();
}

gui::AchievementListSortFilterModel *
AchievementSetItem::getAchievements() const {
  return m_sortFilterModel.get();
}
} // namespace firelight::achievements
