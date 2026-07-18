#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <SDL_gamecontroller.h>
#include <string>

#include <firelight/input/gamepad_type.hpp>
#include <firelight/input/igamepad.hpp>

namespace firelight::input {
class SdlController : public IGamepad {
public:
  SdlController() = default;

  SdlController(SDL_GameController *t_controller);

  ~SdlController() override;

  int16_t evaluateRawInput(const GamepadInput input) const override;

  [[nodiscard]] std::shared_ptr<GamepadProfile> getProfile() const override;

  void setProfile(const std::shared_ptr<GamepadProfile> &profile) override;

  bool isButtonPressed(int platformId, int controllerTypeId,
                       Input t_button) override;

  bool isVirtualInputActive(int platformId, int controllerTypeId,
                            int virtualInput) override;

  int16_t getLeftStickXPosition(int platformId, int controllerTypeId) override;

  int16_t getLeftStickYPosition(int platformId, int controllerTypeId) override;

  int16_t getRightStickXPosition(int platformId, int controllerTypeId) override;

  int16_t getRightStickYPosition(int platformId, int controllerTypeId) override;

  [[nodiscard]] int32_t getInstanceId() const override;

  [[nodiscard]] std::string getName() const override;

  void setPlayerIndex(int t_newPlayerIndex) override;

  [[nodiscard]] int getPlayerIndex() const override;

  void setStrongRumble(int platformId, uint16_t t_strength) override;

  void setWeakRumble(int platformId, uint16_t t_strength) override;

  [[nodiscard]] bool isWired() const override;

  [[nodiscard]] GamepadType getType() const override;

  [[nodiscard]] DeviceType getDeviceType() const override;

  [[nodiscard]] DeviceIdentifier getDeviceIdentifier() const override;

  bool disconnect() override;

private:
  [[nodiscard]] int16_t evaluateMapping(GamepadInput input) const;

  // How far a physical input has to travel to count as pressed: the profile's
  // tunable threshold for a trigger, half range for anything else
  [[nodiscard]] int digitalThreshold(GamepadInput input, int platformId,
                                     int controllerTypeId) const;
  // Evaluates a single Binding to a digital pressed/not-pressed result,
  // honoring its modifiers (all must be held) and analog threshold
  [[nodiscard]] bool evaluateBindingDigital(const Binding &binding) const;
  // Evaluates a Binding including toggle (latch on rising edge) and turbo
  // (autofire while active) behavior. Mutates per-binding state, so it must be
  // called once per frame per binding
  bool evaluateBindingWithModes(GamepadInput target, std::size_t index,
                                const Binding &binding);
  std::shared_ptr<GamepadProfile> m_profile = nullptr;

  // Per-binding state for toggle/turbo, keyed by (target << 8 | bindingIndex)
  std::map<uint64_t, bool> m_togglePrevRaw;
  std::map<uint64_t, bool> m_toggleLatch;

  SDL_GameController *m_SDLController = nullptr;
  SDL_Joystick *m_SDLJoystick = nullptr;
  int32_t m_SDLJoystickInstanceId = -1;

  int m_playerIndex = -1;

  std::string m_deviceName;
  int m_vendorId = 0;
  int m_productId = 0;
  int m_productVersion = 0;
};
} // namespace firelight::input
