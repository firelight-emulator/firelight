//
// Created by alexs on 6/20/2024.
//

#include "gamepad_mapping.hpp"


namespace firelight::gui {
    GamepadMapping::GamepadMapping(QObject *parent) {
    }

    int GamepadMapping::leftStickXDeadzone() const {
        return 1;
    }

    void GamepadMapping::setLeftStickXDeadzone(int deadzone) {
        emit leftStickXDeadzoneChanged();
    }

    int GamepadMapping::rightStickXDeadzone() const {
        return 1;
    }

    void GamepadMapping::setRightStickXDeadzone(int deadzone) {
        emit rightStickXDeadzoneChanged();
    }
} // gui
// firelight
