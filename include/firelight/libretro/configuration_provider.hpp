#pragma once

#include <string>

#include "libretro/libretro.h"

namespace firelight::libretro {
    class IConfigurationProvider {
    protected:
        ~IConfigurationProvider() = default;

    public:
        class OptionValue {
        public:
            std::string key;
            std::string label;
        };

        class Option {
        public:
            std::string key;
            std::string label;
            std::string description;
            std::vector<OptionValue> possibleValues;
            int defaultValueIndex = 0;
        };

        // TODO: Option categories

        virtual void registerOption(Option option) = 0;

        virtual bool anyOptionValueHasChanged() = 0;

        virtual OptionValue getOptionValue(std::string key) = 0;

        virtual void setOptionVisibility(std::string key, bool visible) = 0;
    };
} // namespace firelight::libretro
