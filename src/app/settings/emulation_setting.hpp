#pragma once
#include <string>
#include <vector>

namespace firelight::settings {
enum EmulationSettingType { BOOLEAN, OPTIONS };

struct EmulationSettingOption {
  std::string label;
  std::string value;
};

struct EmulationSetting {
  std::string label;
  std::string category;
  std::string key;
  std::string description;

  std::string defaultValue;

  EmulationSettingType type;

  bool requiresRestart = false;

  std::string trueStringValue = "true";
  std::string falseStringValue = "false";

  // Applicable if type is COMBOBOX
  std::vector<EmulationSettingOption> options;
};
} // namespace firelight::settings