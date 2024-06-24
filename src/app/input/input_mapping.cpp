#include "input_mapping.hpp"

SDL_GameControllerButton firelight::input::InputMapping::getMappedButton(libretro::IRetroPad::Button button) {
    switch (button) {
        case libretro::IRetroPad::NorthFace:
            return SDL_CONTROLLER_BUTTON_Y;
        case libretro::IRetroPad::SouthFace:
            return SDL_CONTROLLER_BUTTON_A;
        case libretro::IRetroPad::EastFace:
            return SDL_CONTROLLER_BUTTON_B;
        case libretro::IRetroPad::WestFace:
            return SDL_CONTROLLER_BUTTON_X;
        case libretro::IRetroPad::DpadUp:
            return SDL_CONTROLLER_BUTTON_DPAD_UP;
        case libretro::IRetroPad::DpadDown:
            return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
        case libretro::IRetroPad::DpadLeft:
            return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
        case libretro::IRetroPad::DpadRight:
            return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
        case libretro::IRetroPad::Start:
            return SDL_CONTROLLER_BUTTON_START;
        case libretro::IRetroPad::Select:
            return SDL_CONTROLLER_BUTTON_BACK;
        case libretro::IRetroPad::LeftBumper:
            return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
        case libretro::IRetroPad::RightBumper:
            return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
        case libretro::IRetroPad::LeftTrigger:
            return SDL_CONTROLLER_BUTTON_INVALID;
        case libretro::IRetroPad::RightTrigger:
            return SDL_CONTROLLER_BUTTON_INVALID;
        case libretro::IRetroPad::L3:
            return SDL_CONTROLLER_BUTTON_LEFTSTICK;
        case libretro::IRetroPad::R3:
            return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
        default:
            return SDL_CONTROLLER_BUTTON_INVALID;
    }
}

float firelight::input::InputMapping::getLeftStickXSensitivity() {
    return 0;
}

float firelight::input::InputMapping::getLeftStickYSensitivity() {
    return 0;
}

float firelight::input::InputMapping::getRightStickXSensitivity() {
    return 0;
}

float firelight::input::InputMapping::getRightStickYSensitivity() {
    return 0;
}

int16_t firelight::input::InputMapping::getLeftStickXDeadzone() {
    return 15000;
}

int16_t firelight::input::InputMapping::getLeftStickYDeadzone() {
    return 15000;
}

int16_t firelight::input::InputMapping::getRightStickXDeadzone() {
    return 15000;
}

int16_t firelight::input::InputMapping::getRightStickYDeadzone() {
    return 15000;
}
