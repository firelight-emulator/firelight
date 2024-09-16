#pragma once
#include <map>
#include <firelight/libretro/configuration_provider.hpp>

class CoreConfiguration final : public firelight::libretro::IConfigurationProvider {
public:
    CoreConfiguration() = default;

    virtual ~CoreConfiguration() = default;

    void registerOption(Option option) override;

    bool anyOptionValueHasChanged() override;

    void setDefaultValue(const std::string &key, const std::string &value) override;

    std::optional<OptionValue> getOptionValue(const std::string &key) override;

    void setOptionVisibility(const std::string &key, bool visible) override;

    void setPlatformValue(const std::string &key, const std::string &value);

    void setGameValue(const std::string &key, const std::string &value);

private:
    std::map<std::string, Option> m_options;
    bool m_changedSinceLastChecked = false;

    std::map<std::string, std::string> m_defaultValues;
    std::map<std::string, std::string> m_platformValues;
    std::map<std::string, std::string> m_gameValues;
};
