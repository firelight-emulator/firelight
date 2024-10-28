#pragma once

#include <map>
#include <string>
#include <QString>

#include "gamepad_type.hpp"

class ControllerIcons {
    static const std::map<int, std::string> controllerIcons;

public:
    static QString sourceUrlFromType(const GamepadType type) {
        switch (type) {
            case MICROSOFT_XBOX_360:
                return "";
            case MICROSOFT_XBOX_ONE:
                return "file:system/_img/Xbox.svg";
            case SONY_DUALSHOCK_3:
                return "";
            case SONY_DUALSHOCK_4:
                return "file:system/_img/Dualshock4.svg";
            case SONY_DUALSENSE:
                return "file:system/_img/DualSense.svg";
            case NINTENDO_SWITCH_PRO:
                return "file:system/_img/SwitchPro.svg";
            case NINTENDO_NSO_N64:
                return "file:system/_img/N64.svg";
            case NINTENDO_NSO_SNES:
                return "file:system/_img/SNES.svg";
            case NINTENDO_NSO_GENESIS:
                return "file:system/_img/genesis.svg";
            default:
                return "file:system/_img/Xbox.svg"; // Default icon
        }
    }
};
