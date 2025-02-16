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
                return "qrc:/images/controllers/xbox";
            case SONY_DUALSHOCK_3:
                return "";
            case SONY_DUALSHOCK_4:
                return "qrc:/images/controllers/dualshock4";
            case SONY_DUALSENSE:
                return "qrc:/images/controllers/dualsense";
            case NINTENDO_SWITCH_PRO:
                return "qrc:/images/controllers/switch-pro";
            case NINTENDO_NSO_N64:
                return "qrc:/images/controllers/n64";
            case NINTENDO_NSO_SNES:
                return "qrc:/images/controllers/snes";
            case NINTENDO_NSO_GENESIS:
              return "qrc:/images/controllers/genesis";
            case KEYBOARD:
              return "qrc:/images/controllers/keyboard";
            default:
                return "qrc:/images/controllers/xbox"; // Default icon
        }
    }
};
