#pragma once

#include "input2/device_identifier.hpp"
#include "input2/device_info.hpp"
#include "profiles/input_mapping.hpp"
#include <input2/gamepad_profile.hpp>
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

  // virtual std::shared_ptr<ControllerProfile>
  // getControllerProfile(std::string name, int vendorId, int productId,
  //                      int productVersion) = 0;

  // profile has:
  // name
  // controller type
  // input mappings
  // deadzones
  // sensitivities
  // [[nodiscard]] virtual std::shared_ptr<ControllerProfile>
  // getControllerProfile(int profileId) const = 0;

  // input mapping has:
  // platform id
  // profile id
  // list of retropad -> other mappings
  // [[nodiscard]] virtual std::shared_ptr<InputMapping>
  // getInputMapping(int mappingId) const = 0;
  //
  // [[nodiscard]] virtual std::shared_ptr<InputMapping>
  // getInputMapping(int profileId, int platformId) = 0;
};
} // namespace firelight::input
