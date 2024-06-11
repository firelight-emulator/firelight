#pragma once

#include "libretro/libretro.h"
#include <vector>

namespace libretro {

class CoreOption {
public:
  explicit CoreOption(retro_core_option_definition definition);
  explicit CoreOption(retro_core_option_v2_definition definition);

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
