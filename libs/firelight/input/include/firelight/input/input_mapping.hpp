#pragma once
#include <firelight/input/analog_settings.hpp>
#include <firelight/input/binding.hpp>
#include <firelight/input/gamepad_input.hpp>

#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace firelight::input {
    // Holds the bindings for one (platform, controller type) within a profile. For
    // each emulated input (GamepadInput) it stores a list of physical Bindings
    // whose evaluated results are combined. It may also carry an analog-tuning
    // override that takes precedence over the profile's default AnalogSettings on
    // this platform
    class InputMapping {
    public:
        explicit InputMapping(
            int id, int profileId, int platformId, int controllerType,
            std::function<void(InputMapping &)> syncCallback = nullptr);

        virtual ~InputMapping() = default;

        unsigned getId() const;

        unsigned getControllerProfileId() const;

        unsigned getPlatformId() const;

        unsigned getControllerType() const;

        void setId(unsigned id);

        void setControllerProfileId(unsigned controllerProfileId);

        void setPlatformId(unsigned platformId);

        void setControllerType(int controllerType);

        // --- Binding API ---
        // Returns the bindings for an emulated input, or an empty list if unbound
        const std::vector<Binding> &getBindings(GamepadInput input) const;

        void setBindings(GamepadInput input, std::vector<Binding> bindings);

        void addBinding(GamepadInput input, const Binding &binding);

        void removeBinding(GamepadInput input, std::size_t index);

        const std::map<GamepadInput, std::vector<Binding> > &getAllBindings() const;

        // --- Analog override (nullopt means "use the profile default") ---
        const std::optional<AnalogSettings> &getAnalogOverride() const;

        void setAnalogOverride(std::optional<AnalogSettings> settings);

        // --- Compatibility helpers: operate on the primary (first) binding as a
        //     single source code, preserving the original simple-remap behavior. ---
        void addMapping(GamepadInput input, int mappedInput);

        std::optional<int> getMappedInput(GamepadInput input);

        virtual void removeMapping(GamepadInput input);

        virtual std::string serialize();

        virtual void deserialize(const std::string &data);

        void sync();

    private:
        std::function<void(InputMapping &)> m_syncCallback;
        int m_id = 0;
        int m_controllerProfileId = 0;
        int m_platformId = 0;
        int m_controllerType = 0;
        std::map<GamepadInput, std::vector<Binding> > m_bindings{};
        std::optional<AnalogSettings> m_analogOverride;
    };
} // namespace firelight::input
