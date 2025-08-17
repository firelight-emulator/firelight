#pragma once
#include "device_identifier.hpp"

#include <firelight/libretro/retropad.hpp>
#include <string>

#include "input/gamepad_type.hpp"
#include "input/profiles/input_mapping.hpp"
#include "input/shortcuts.hpp"
#include "input2/gamepad_profile.hpp"

namespace firelight::input {
class IGamepad : public libretro::IRetroPad {
public:
  virtual std::shared_ptr<GamepadProfile> getProfile() const = 0;

  virtual void setProfile(const std::shared_ptr<GamepadProfile> &profile) = 0;

  virtual std::vector<Shortcut> getToggledShortcuts(GamepadInput input) = 0;

  virtual int16_t evaluateRawInput(const GamepadInput input) const = 0;

  virtual std::string getName() const = 0;

  virtual int getPlayerIndex() const = 0;

  virtual void setPlayerIndex(int playerIndex) = 0;

  virtual int getInstanceId() const = 0;

  virtual bool isWired() const = 0;

  virtual GamepadType getType() const = 0;

  virtual bool disconnect() = 0;

  virtual DeviceIdentifier getDeviceIdentifier() const = 0;
};
} // namespace firelight::input
