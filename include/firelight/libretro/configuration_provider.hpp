#pragma once

#include <optional>
#include <string>
#include <vector>

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

  // All options the core has declared so far (via registerOption). Lets the
  // frontend cache/persist a core's option definitions for pre-launch editing.
  [[nodiscard]] virtual std::vector<Option> getOptions() const = 0;

  virtual bool anyOptionValueHasChanged() = 0;

  virtual void setDefaultValue(const std::string &key,
                               const std::string &value) = 0;

  virtual std::optional<std::string> getOptionValue(const std::string &key) = 0;

  virtual void setOptionVisibility(const std::string &key, bool visible) = 0;
};
} // namespace firelight::libretro
