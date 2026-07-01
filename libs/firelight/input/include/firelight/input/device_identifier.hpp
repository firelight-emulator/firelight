#pragma once
#include <firelight/input/gamepad_type.hpp>

#include <string>

namespace firelight::input {

struct DeviceIdentifier {
  // Devices are identified by their stable USB-style ids (vendor, product,
  // version). Two identical models share one identity/profile; the keyboard
  // uses the sentinel (-1, -1, -1).
  std::string deviceName;
  DeviceType type = DeviceType::Gamepad;
  int vendorId = 0;
  int productId = 0;
  int productVersion = 0;
};
} // namespace firelight::input
