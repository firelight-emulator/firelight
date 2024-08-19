#pragma once
#include "../saves/suspend_point.hpp"

namespace firelight::emu {
    class RewindBro {
        void pushState(SuspendPoint &state);
    };
} // firelight::emu
