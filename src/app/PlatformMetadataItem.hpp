#pragma once

#include <QQuickItem>

#include "manager_accessor.hpp"


namespace firelight {
    class PlatformMetadataItem : public QQuickItem, public ManagerAccessor {
        Q_OBJECT
        Q_PROPERTY(int platformId READ getPlatformId WRITE setPlatformId NOTIFY platformIdChanged)
        Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
        Q_PROPERTY(QString abbreviation READ getAbbreviation NOTIFY abbreviationChanged)

    public:
        explicit PlatformMetadataItem(QQuickItem *parent = nullptr);

        ~PlatformMetadataItem() override = default;

        void setPlatformId(int playerNumber);

        [[nodiscard]] int getPlatformId() const;

        [[nodiscard]] QString getName() const;

        [[nodiscard]] QString getAbbreviation() const;

    signals:
        void platformIdChanged();

        void nameChanged();

        void abbreviationChanged();

    private:
        int m_platformId = 0;
    };
} // firelight
