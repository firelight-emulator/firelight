#pragma once

#include <map>
#include <QString>
#include <firelight/libretro/retropad.hpp>

class IdkStuff {
public:
    struct ButtonThingy {
        firelight::libretro::IRetroPad::Button button;
        QString iconUrl;
        QString displayName;
    };

private:
    static std::map<firelight::libretro::IRetroPad::Button, int> buttonMap;
};
