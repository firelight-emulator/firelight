#pragma once
#include <QObject>

#include "firelight/controller_profile.hpp"
#include "gamepad_mapping.hpp"
#include "../app/manager_accessor.hpp"

namespace firelight::gui {
    class GamepadProfile : public QObject, public ManagerAccessor {
        Q_OBJECT
        // Q_PROPERTY(QVariantMap mappings READ mappings WRITE setMappings NOTIFY mappingsChanged)
        Q_PROPERTY(int profileId READ profileId WRITE setProfileId NOTIFY profileIdChanged)
        Q_PROPERTY(
            int currentPlatformId READ currentPlatformId WRITE setCurrentPlatformId NOTIFY currentPlatformIdChanged)

    public:
        explicit GamepadProfile(QObject *parent = nullptr);

        int profileId() const;

        void setProfileId(int profileId);

        int currentPlatformId() const;

        void setCurrentPlatformId(int platformId);

        Q_INVOKABLE [[nodiscard]] QVariant getCurrentMapping() const;

    signals:
        void profileIdChanged();

        void currentPlatformIdChanged();

    private:
        int m_profileId = -1;
        int m_currentPlatformId = -1;

        ControllerProfile m_profile = {};
    };
} // firelight::gui
