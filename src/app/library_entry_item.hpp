#pragma once

#include <QQuickItem>

#include "manager_accessor.hpp"

namespace firelight {
    class LibraryEntryItem : public QQuickItem, public ManagerAccessor {
        Q_OBJECT
        Q_PROPERTY(int entryId READ getEntryId WRITE setEntryId NOTIFY entryIdChanged)
        Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
        //        Q_PROPERTY(QString abbreviation READ getAbbreviation NOTIFY abbreviationChanged)

    public:
        explicit LibraryEntryItem(QQuickItem *parent = nullptr);

        ~LibraryEntryItem() override = default;

        void setEntryId(int entryId);

        [[nodiscard]] int getEntryId() const;

        [[nodiscard]] QString getName() const;

        //        [[nodiscard]] QString getAbbreviation() const;

    signals:
        void entryIdChanged();

        void nameChanged();

        //        void abbreviationChanged();

    private:
        int m_entryId = 0;
        QString m_name;
    };
} // firelight
