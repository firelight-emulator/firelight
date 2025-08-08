#pragma once
#include <string>

namespace firelight::input {

struct DeviceIdentifier {
  std::string deviceName;
  int vendorId = 0;
  int productId = 0;
  int productVersion = 0;
};
} // namespace firelight::input