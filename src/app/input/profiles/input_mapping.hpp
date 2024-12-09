#pragma once
#include <map>
#include <optional>
#include <SDL_gamecontroller.h>
#include <firelight/libretro/retropad.hpp>

namespace firelight::input {
    class InputMapping {
    private:
        enum InputType {
            BUTTON, AXIS
        };

        struct InputDescription {
            InputType type{};

            SDL_GameControllerButton button{};
            SDL_GameControllerAxis axis{};
            bool axisPositive = true;
        };

    public:
        InputMapping() {
            m_buttonMappings.emplace(libretro::IRetroPad::Input::SouthFace,
                                     InputDescription{BUTTON, SDL_CONTROLLER_BUTTON_LEFTSHOULDER});

            m_buttonMappings.emplace(libretro::IRetroPad::Input::EastFace,
                                     InputDescription{
                                         AXIS, SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_AXIS_RIGHTY, false
                                     });

            // when being asked about axis, need to return
        }

        std::optional<bool> evaluateButtonMapping(SDL_Joystick *joystick, libretro::IRetroPad::Input button);

        std::optional<int16_t> evaluateAxisMapping(SDL_Joystick *joystick, libretro::IRetroPad::Axis axis);

    private:
        std::map<libretro::IRetroPad::Input, InputDescription> m_buttonMappings;
        std::map<libretro::IRetroPad::Axis, InputDescription> m_axisMappings;

        std::map<SDL_GameControllerAxis, bool> m_axisBeingHeld{};
        std::map<SDL_GameControllerAxis, bool> m_axisReleased{};

        std::optional<int16_t> evaluate(SDL_Joystick *joystick, const InputDescription &description);

        unsigned getId();

        unsigned getControllerProfileId();

        unsigned getPlatformId();
    };
}
