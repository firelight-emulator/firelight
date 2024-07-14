#pragma once
#include <map>
#include <firelight/libretro/configuration_provider.hpp>

class CoreConfiguration final : public firelight::libretro::IConfigurationProvider {
public:
    CoreConfiguration() = default;

    virtual ~CoreConfiguration() = default;

    void registerOption(Option option) override;

    bool anyOptionValueHasChanged() override;

    void setOptionValue(std::string key, std::string value) override;

    std::optional<OptionValue> getOptionValue(std::string key) override;

    void setOptionVisibility(std::string key, bool visible) override;

private:
    std::map<std::string, int> m_optionValueIndices;
    std::map<std::string, Option> m_options;
    bool m_changedSinceLastChecked = false;
};
