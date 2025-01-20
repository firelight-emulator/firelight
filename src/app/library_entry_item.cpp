#pragma once

#include <QQuickItem>

#include "library_entry_item.hpp"


namespace firelight {
    LibraryEntryItem::LibraryEntryItem(QQuickItem *parent) {
    }

    void LibraryEntryItem::setEntryId(const int entryId) {
        if (entryId != m_entryId) {
            emit entryIdChanged();
        }

        m_entryId = entryId;
    }

    int LibraryEntryItem::getEntryId() const {
        return m_entryId;
    }

    QString LibraryEntryItem::getName() const {
        return m_name;
    }
} // firelight
