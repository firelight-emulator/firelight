#pragma once
#include <firelight/input/device_identifier.hpp>
#include <firelight/input/gamepad_profile.hpp>
#include <firelight/input/gamepad_type.hpp>
#include <firelight/input/input_mapping.hpp>
#include <firelight/input/input_suppressor.hpp>
#include <firelight/libretro/retropad.hpp>
#include <string>

namespace firelight::input {
class IGamepad : public libretro::IRetroPad {
public:
  // The inputs this device is withholding from the game because a shortcut is
  // using them. Concrete state on the interface so every device shares one
  // implementation and the shortcut engine, which already holds an IGamepad*,
  // can reach it without a lookup.
  InputSuppressor &suppressor() { return m_suppressor; }
  [[nodiscard]] const InputSuppressor &suppressor() const {
    return m_suppressor;
  }

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

private:
  InputSuppressor m_suppressor;
};
} // namespace firelight::input
