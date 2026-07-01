#pragma once
#include <firelight/input/device_identifier.hpp>
#include <firelight/input/gamepad_profile.hpp>
#include <firelight/input/gamepad_type.hpp>
#include <firelight/input/input_mapping.hpp>
#include <firelight/libretro/retropad.hpp>
#include <string>

namespace firelight::input {
class IGamepad : public libretro::IRetroPad {
public:
  virtual std::shared_ptr<GamepadProfile> getProfile() const = 0;

  virtual void setProfile(const std::shared_ptr<GamepadProfile> &profile) = 0;

  virtual int16_t evaluateRawInput(const GamepadInput input) const = 0;

  virtual std::string getName() const = 0;

  virtual int getPlayerIndex() const = 0;

  virtual void setPlayerIndex(int playerIndex) = 0;

  virtual int getInstanceId() const = 0;

  virtual bool isWired() const = 0;

  virtual GamepadType getType() const = 0;

  virtual DeviceType getDeviceType() const = 0;

  virtual bool disconnect() = 0;

  virtual DeviceIdentifier getDeviceIdentifier() const = 0;
};
} // namespace firelight::input
