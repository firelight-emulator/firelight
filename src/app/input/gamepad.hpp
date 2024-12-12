#pragma once
#include <firelight/libretro/retropad.hpp>
#include <string>

#include "gamepad_type.hpp"

namespace firelight::input {
    class IGamepad : public libretro::IRetroPad {
    public:
        virtual std::string getName() const = 0;

        virtual int getPlayerIndex() const = 0;

        virtual void setPlayerIndex(int playerIndex) = 0;

        virtual bool isWired() const = 0;

        virtual GamepadType getType() const = 0;
    };
}
