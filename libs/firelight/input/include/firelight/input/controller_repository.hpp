#pragma once

#include <firelight/input/device_identifier.hpp>
#include <firelight/input/device_info.hpp>
#include <firelight/input/gamepad_profile.hpp>
#include <optional>
#include <vector>

namespace firelight::input {
class IControllerRepository {
public:
  virtual ~IControllerRepository() = default;

  virtual std::optional<DeviceInfo>
  getDeviceInfo(DeviceIdentifier identifier) const = 0;

  virtual void updateDeviceInfo(DeviceIdentifier identifier,
                                const DeviceInfo &info) = 0;

  virtual std::shared_ptr<GamepadProfile> getProfile(int id) = 0;

  virtual std::shared_ptr<GamepadProfile> createProfile(std::string name) = 0;
};
} // namespace firelight::input
