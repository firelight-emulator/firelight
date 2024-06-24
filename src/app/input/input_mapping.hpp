#pragma once
#include <SDL_gamecontroller.h>
#include <firelight/libretro/retropad.hpp>

namespace firelight::input {
    class InputMapping {
    public:
        SDL_GameControllerButton getMappedButton(libretro::IRetroPad::Button button);

        float getLeftStickXSensitivity();

        float getLeftStickYSensitivity();

        float getRightStickXSensitivity();

        float getRightStickYSensitivity();

        int16_t getLeftStickXDeadzone();

        int16_t getLeftStickYDeadzone();

        int16_t getRightStickXDeadzone();

        int16_t getRightStickYDeadzone();
    };
}
