#pragma once
#include <map>
#include <optional>
#include <SDL_gamecontroller.h>
#include <firelight/libretro/retropad.hpp>

namespace firelight::input {
    class InputMapping {
        enum InputType {
            BUTTON, AXIS
        };

        class InputDescription {
            InputType type{};

            SDL_GameControllerButton button{};
            SDL_GameControllerAxis axis{};
            bool axisPositive = true;
        };

    public:
        std::optional<bool> evaluateButtonMapping(SDL_Joystick *joystick, libretro::IRetroPad::Button button);

        std::optional<float> evaluateAxisMapping(SDL_Joystick *joystick, libretro::IRetroPad::Axis axis);

        std::optional<InputDescription> getButtonMapping(libretro::IRetroPad::Button button);

        std::optional<InputDescription> getAxisMapping(libretro::IRetroPad::Axis axis);

    private:
        std::map<libretro::IRetroPad::Button, InputDescription> m_buttonMappings;
        std::map<libretro::IRetroPad::Axis, InputDescription> m_axisMappings;
    };
}
