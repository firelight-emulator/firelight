#pragma once

#include <firelight/input/gamepad_type.hpp>
#include <firelight/input/igamepad.hpp>

#include <SDL_gamecontroller.h>
#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace firelight::input {
class SdlController : public IGamepad {
public:
  SdlController() = default;

  SdlController(SDL_GameController *t_controller);

  ~SdlController() override;

  int16_t evaluateRawInput(const GamepadInput input) const override;

  [[nodiscard]] std::shared_ptr<GamepadProfile> getProfile() const override;

  void setProfile(const std::shared_ptr<GamepadProfile> &profile) override;

  bool isButtonPressed(int platformId, int controllerTypeId, Input t_button) override;

  bool isVirtualInputActive(int platformId, int controllerTypeId, int virtualInput) override;

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

  /** Stores the percent rumble is scaled by */
  void setRumbleScale(int percent) override;

  /** Stores the light color and sends it to the device */
  void setLightColor(std::optional<uint32_t> rgb) override;

  /** A rumble strength scaled by a percent clamped to 0..100 */
  [[nodiscard]] static uint16_t scaleRumble(uint16_t strength, int percent);

  /** SDL's light color for a player slot, as 0xRRGGBB */
  [[nodiscard]] static uint32_t playerLightColor(int playerIndex);

private:
  [[nodiscard]] int16_t evaluateMapping(GamepadInput input) const;

  /** How far a physical input has to travel to count as pressed */
  [[nodiscard]] int digitalThreshold(GamepadInput input, int platformId, int controllerTypeId) const;

  /**
   * Evaluates a single Binding to a digital pressed/not-pressed result, honoring its modifiers (all must be held) and
   * analog threshold
   */
  [[nodiscard]] bool evaluateBindingDigital(const Binding &binding) const;

  // Evaluates a Binding including toggle (latch on rising edge) and turbo (autofire while active) behavior. Mutates
  // per-binding state, so it must be called once per frame per binding
  bool evaluateBindingWithModes(GamepadInput target, std::size_t index, const Binding &binding);

  /** Sets the current light on the controller, if applicable  */
  void applyLightColor();

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

  std::atomic<int> m_rumblePercent{100};

  // The stored light colour that means the player colour
  static constexpr int64_t PLAYER_LIGHT_COLOR = -1;

  // 0xRRGGBB, or PLAYER_LIGHT_COLOR
  std::atomic<int64_t> m_lightColor{PLAYER_LIGHT_COLOR};
};
} // namespace firelight::input
