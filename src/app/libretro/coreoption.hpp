//
// Created by alexs on 10/14/2023.
//

#ifndef FIRELIGHT_COREOPTION_HPP
#define FIRELIGHT_COREOPTION_HPP

#include "libretro.h"
#include <string>
#include <vector>
// #include <nlohmann/json.hpp>

// using nlohmann::json;

namespace libretro {

class CoreOption {
public:
  explicit CoreOption(retro_core_option_definition definition);

  explicit CoreOption(retro_core_option_v2_definition definition);

  //        json dumpJson();

  const char *currentValue;
  bool displayToUser = true;
  const char *key;
  const char *description;
  const char *descCategorized = nullptr;
  const char *info;
  const char *infoCategorized = nullptr;
  const char *categoryKey = nullptr;
  std::vector<retro_core_option_value> values;
  const char *defaultValue;
  int defaultValueIndex;
};

} // namespace libretro

#endif // FIRELIGHT_COREOPTION_HPP
