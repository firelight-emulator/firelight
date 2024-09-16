#pragma once

#include <string>
#include <vector>
#include <optional>

namespace firelight::libretro {
    class IConfigurationProvider {
    protected:
        ~IConfigurationProvider() = default;

    public:
        class OptionValue final {
        public:
            std::string key;
            std::string label;
        };

        class Option final {
        public:
            std::string key;
            std::string label;
            std::string description;
            std::vector<OptionValue> possibleValues;
            std::string defaultValueKey;
        };

        // TODO: Option categories

        virtual void registerOption(Option option) = 0;

        virtual bool anyOptionValueHasChanged() = 0;

        virtual void setDefaultValue(const std::string &key, const std::string &value) = 0;

        virtual std::optional<OptionValue> getOptionValue(const std::string &key) = 0;

        virtual void setOptionVisibility(const std::string &key, bool visible) = 0;
    };
} // namespace firelight::libretro
