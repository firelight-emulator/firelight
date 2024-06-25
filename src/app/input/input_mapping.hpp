#pragma once
#include <SDL_gamecontroller.h>
#include <firelight/libretro/retropad.hpp>

namespace firelight::input {
    class InputMapping {
    public:
        enum Things {
            NORTH_FACE,
            SOUTH_FACE,
            EAST_FACE,
            WEST_FACE,
            DPAD_UP,
            DPAD_DOWN,
            DPAD_LEFT,
            DPAD_RIGHT,
            START,
            SELECT,
            GUIDE,
            MISC1,
            PADDLE1,
            PADDLE2,
            PADDLE3,
            PADDLE4,
            TOUCHPAD,
            R1,
            R2,
            R3,
            L1,
            L2,
            L3,
            LEFT_STICK_UP,
            LEFT_STICK_DOWN,
            LEFT_STICK_LEFT,
            LEFT_STICK_RIGHT,
            RIGHT_STICK_UP,
            RIGHT_STICK_DOWN,
            RIGHT_STICK_LEFT,
            RIGHT_STICK_RIGHT
        };

        bool isButtonPressed(firelight::libretro::IRetroPad::Button t_button);

        int16_t getLeftStickXPosition();

        int16_t getLeftStickYPosition();

        int16_t getRightStickXPosition();

        int16_t getRightStickYPosition();
    };
}
