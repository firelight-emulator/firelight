#pragma once
#include <string>

namespace firelight::input {
struct DeviceInfo {
  std::string displayName;
  int profileId = -1;
};
} // namespace firelight::input