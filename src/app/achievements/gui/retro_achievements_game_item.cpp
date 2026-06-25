#include "retro_achievements_game_item.hpp"

#include <firelight/achievement_service.hpp>

#include <library/user_library.hpp>
#include <platforms/platform_service.hpp>

namespace firelight::achievements {
RetroAchievementsGameItem::RetroAchievementsGameItem(QObject *parent) {}

QString RetroAchievementsGameItem::getContentHash() const {
  return m_contentHash;
}

void RetroAchievementsGameItem::setContentHash(const QString &contentHash) {
  if (m_contentHash == contentHash) {
    return;
  }

  const auto entry = getLibraryService()->getEntryWithContentHash(contentHash);
  if (!entry) {
    m_achievementSets.clear();
    return;
  }

  const auto platform = getPlatformService()->getPlatform(entry->platformId);
  if (!platform) {
    m_achievementSets.clear();
    return;
  }

  m_platform = *platform;
  m_platformName = QString::fromStdString(m_platform.name);

  auto game =
      getAchievementService()->getGameForHash(contentHash.toStdString());
  if (!game) {
    m_achievementSets.clear();
    return;
  }

  m_title = QString::fromStdString(game->title);
  m_iconUrl = QString::fromStdString(game->imageIconUrl);

  m_achievementSets.clear();
  for (const auto &set : game->achievementSets) {
    m_achievementSets.emplace_back(new AchievementSetItem(set, this));
  }

  m_contentHash = contentHash;
  emit contentHashChanged();
}

QList<AchievementSetItem *>
RetroAchievementsGameItem::getAchievementSets() const {
  return m_achievementSets;
}
} // namespace firelight::achievements