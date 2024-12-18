#pragma once
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <firelight/libretro/retropad.hpp>

namespace firelight::input {
    class InputMapping {
    public:
        explicit InputMapping(std::function<void(InputMapping &)> syncCallback = nullptr);

        unsigned getId() const;

        unsigned getControllerProfileId();

        unsigned getPlatformId() const;

        void setId(unsigned id);

        void setControllerProfileId(unsigned controllerProfileId);

        void setPlatformId(unsigned platformId);

        void addMapping(libretro::IRetroPad::Input input, libretro::IRetroPad::Input mappedInput);

        std::optional<libretro::IRetroPad::Input> getMappedInput(libretro::IRetroPad::Input input);

        std::map<libretro::IRetroPad::Input, libretro::IRetroPad::Input> &getMappings();

        void removeMapping(libretro::IRetroPad::Input input);

        std::string serialize();

        void deserialize(const std::string &data);

        void sync();

    private:
        std::function<void(InputMapping &)> m_syncCallback;
        int m_id = 0;
        int m_controllerProfileId = 0;
        int m_platformId = 0;
        std::map<libretro::IRetroPad::Input, libretro::IRetroPad::Input> m_mappings{};
    };
}
