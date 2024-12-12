#include "input_mapping.hpp"

namespace firelight::input {
    std::optional<bool>
    InputMapping::evaluateButtonMapping(SDL_GameController *controller, libretro::IRetroPad::Input button) {
        if (!m_buttonMappings.contains(button)) {
            return std::nullopt;
        }

        const auto desc = m_buttonMappings[button];

        if (desc.type == BUTTON) {
            return {
                // SDL_GameControllerGetButton(SDL_GameControllerFromInstanceID(SDL_JoystickInstanceID(joystick)),
                // description.button) * 32767 * (description.axisPositive ? 1 : -1)
                SDL_GameControllerGetButton(controller, desc.button)
            };
        }

        if (desc.type == AXIS) {
            return {
                SDL_GameControllerGetAxis(controller, desc.axis)
            };
        }

        return 0;
    }

    std::optional<int16_t> InputMapping::evaluateAxisMapping(SDL_GameController *controller,
                                                             libretro::IRetroPad::Axis axis) {
        if (!m_axisMappings.contains(axis)) {
            return std::nullopt;
        }

        return evaluate(controller, m_axisMappings[axis]);
    }

    std::optional<int16_t> InputMapping::evaluate(SDL_GameController *controller, const InputDescription &description) {
        if (description.type == BUTTON) {
            return {
                // SDL_GameControllerGetButton(SDL_GameControllerFromInstanceID(SDL_JoystickInstanceID(joystick)),
                // description.button) * 32767 * (description.axisPositive ? 1 : -1)
                SDL_GameControllerGetButton(controller, description.button)
            };
        }

        if (description.type == AXIS) {
            auto value = SDL_GameControllerGetAxis(controller, description.axis);
            if (value > 0 && description.axisPositive) {
                return value;
            }

            // // Ensure axis state tracking
            // if (!m_axisBeingHeld.contains(description.axis)) {
            //     m_axisBeingHeld[description.axis] = false;
            // }
            // if (!m_axisReleased.contains(description.axis)) {
            //     m_axisReleased[description.axis] = true; // Start in "released" state
            // }
            //
            // if (value > 500) { // Axis is pressed
            //     if (!m_axisBeingHeld[description.axis]) {
            //         // First press
            //         m_axisBeingHeld[description.axis] = true;
            //         m_axisReleased[description.axis] = false; // Mark as "pressed"
            //         return {value}; // Initial press
            //     }
            //     if (!m_axisReleased[description.axis]) {
            //         // Send the intermediate "release" signal
            //         m_axisReleased[description.axis] = true; // Mark as "released"
            //         return {0}; // Simulated release
            //     }
            //     return {value}; // Continue holding
            // }
            //
            // // Axis is released, reset states
            // m_axisBeingHeld[description.axis] = false;
            // m_axisReleased[description.axis] = true;
            return {value};
        }
        return {};
    }

    unsigned InputMapping::getId() {
    }

    unsigned InputMapping::getControllerProfileId() {
    }

    unsigned InputMapping::getPlatformId() {
    }
}

