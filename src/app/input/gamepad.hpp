#pragma once
#include <firelight/libretro/retropad.hpp>
#include <string>

#include "gamepad_type.hpp"
#include "profiles/input_mapping.hpp"

namespace firelight::input {
    class IGamepad : public libretro::IRetroPad {
    public:
        virtual int getProfileId() const = 0;

        virtual void setActiveMapping(const std::shared_ptr<InputMapping> &mapping) = 0;

        virtual std::string getName() const = 0;

        virtual int getPlayerIndex() const = 0;

        virtual void setPlayerIndex(int playerIndex) = 0;

        virtual bool isWired() const = 0;

        virtual GamepadType getType() const = 0;
    };
}
