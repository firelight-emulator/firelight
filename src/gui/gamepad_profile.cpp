#include "gamepad_profile.hpp"

namespace firelight::gui {
    GamepadProfile::GamepadProfile(QObject *parent) {
    }

    int GamepadProfile::profileId() const {
        return m_profileId;
    }

    void GamepadProfile::setProfileId(const int profileId) {
        m_profileId = profileId;
        emit profileIdChanged();
    }

    int GamepadProfile::currentPlatformId() const {
        return m_currentPlatformId;
    }

    void GamepadProfile::setCurrentPlatformId(const int platformId) {
        m_currentPlatformId = platformId;
        emit currentPlatformIdChanged();
    }
} // firelight::gui
