#include "library_entry_item.hpp"
#include <firelight/library/user_library_service.hpp>

#include <firelight/platforms/platform_service.hpp>

namespace firelight {
LibraryEntryItem::LibraryEntryItem(QQuickItem *parent) {}

void LibraryEntryItem::setEntryId(const int entryId) {
  if (entryId != m_entryId) {
    m_entryId = entryId;
    emit entryIdChanged();

    const auto entry = getLibraryService()->getEntry(m_entryId);
    if (!entry) {
      return;
    }

    m_contentHash = QString::fromStdString(entry->contentHash);
    m_name = QString::fromStdString(entry->displayName);
    m_achievementSetId = entry->retroachievementsSetId;
    m_icon1x1SourceUrl = QString::fromStdString(entry->icon1x1SourceUrl);
    m_platformId = entry->platformId;

    auto platform =
        getPlatformService()->getPlatform(m_platformId);
    if (platform.has_value()) {
      m_platformIconName = QString::fromStdString(platform.value().slug);
      emit platformIconNameChanged();
    }

    emit platformIdChanged();
    emit icon1x1SourceUrlChanged();
    emit contentHashChanged();
    emit nameChanged();
    emit achievementSetIdChanged();
  }

  m_entryId = entryId;
}

int LibraryEntryItem::getEntryId() const { return m_entryId; }

QString LibraryEntryItem::getContentHash() { return m_contentHash; }

int LibraryEntryItem::getPlatformId() const { return m_platformId; }

QString LibraryEntryItem::getName() const { return m_name; }

int LibraryEntryItem::getAchievementSetId() const { return m_achievementSetId; }

QString LibraryEntryItem::getIcon1x1SourceUrl() const {
  return m_icon1x1SourceUrl;
}
QString LibraryEntryItem::getPlatformIconName() const {
  return m_platformIconName;
}
} // namespace firelight
