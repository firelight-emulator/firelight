#include "library_entry_item.hpp"

namespace firelight {
    LibraryEntryItem::LibraryEntryItem(QQuickItem *parent) {
    }

    void LibraryEntryItem::setEntryId(const int entryId) {
        if (entryId != m_entryId) {
            m_entryId = entryId;
            emit entryIdChanged();

            const auto entry = getUserLibrary()->getEntry(m_entryId);
            if (!entry) {
              return;
            }

            m_name = entry->displayName;
            m_achievementSetId = entry->retroachievementsSetId;
            m_icon1x1SourceUrl = entry->icon1x1SourceUrl;
            emit icon1x1SourceUrlChanged();
            emit nameChanged();
            emit achievementSetIdChanged();
        }

        m_entryId = entryId;
    }

    int LibraryEntryItem::getEntryId() const {
        return m_entryId;
    }

    QString LibraryEntryItem::getName() const { return m_name; }

    int LibraryEntryItem::getAchievementSetId() const {
      return m_achievementSetId;
    }

    QString LibraryEntryItem::getIcon1x1SourceUrl() const {
      return m_icon1x1SourceUrl;
    }
    } // firelight
