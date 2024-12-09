#include "gamepad_profile.hpp"

namespace firelight::gui {
    GamepadProfile::GamepadProfile(QObject *parent) {
    }

    int GamepadProfile::profileId() const {
        return m_profileId;
    }

    void GamepadProfile::setProfileId(const int profileId) {
        m_profileId = profileId;
        // const auto prof = getUserdataManager()->getControllerProfile(profileId);
        // if (prof.has_value()) {
        //     m_profile = prof.value();
        // }

        emit profileIdChanged();
    }

    int GamepadProfile::currentPlatformId() const {
        return m_currentPlatformId;
    }

    void GamepadProfile::setCurrentPlatformId(const int platformId) {
        m_currentPlatformId = platformId;
        emit currentPlatformIdChanged();
    }

    QVariant GamepadProfile::getCurrentMapping() const {
        // for (const auto mapping: m_profile.inputMappings) {
        //     if (mapping.platformId == m_currentPlatformId) {
        //         QJsonObject obj;
        //         obj["id"] = mapping.id;
        //         obj["controller_profile_id"] = mapping.controllerProfileId;
        //         obj["platform_id"] = mapping.platformId;
        //
        //         return QVariant::fromValue(obj);
        //     }
        // }
        return {};
    }
} // firelight::gui
