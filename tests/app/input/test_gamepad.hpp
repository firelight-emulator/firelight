#pragma once
#include <input2/gamepad.hpp>

namespace firelight::input {

class TestGamepad final : public IGamepad {
public:
  explicit TestGamepad(int instanceId = 0, int vendorId = 0, int productId = 0,
                       int productVersion = 0);
  bool isButtonPressed(int platformId, int controllerTypeId,
                       Input t_button) override;
  int16_t getLeftStickXPosition(int platformId, int controllerTypeId) override;
  int16_t getLeftStickYPosition(int platformId, int controllerTypeId) override;
  int16_t getRightStickXPosition(int platformId, int controllerTypeId) override;
  int16_t getRightStickYPosition(int platformId, int controllerTypeId) override;
  void setStrongRumble(int platformId, uint16_t t_strength) override;
  void setWeakRumble(int platformId, uint16_t t_strength) override;
  std::shared_ptr<GamepadProfile> getProfile() const override;
  void setProfile(const std::shared_ptr<GamepadProfile> &profile) override;
  std::vector<Shortcut> getToggledShortcuts(GamepadInput input) override;
  int16_t evaluateRawInput(const GamepadInput input) const override;
  std::string getName() const override;
  int getPlayerIndex() const override;
  void setPlayerIndex(int playerIndex) override;
  int getInstanceId() const override;
  bool isWired() const override;
  GamepadType getType() const override;
  bool disconnect() override;
  DeviceIdentifier getDeviceIdentifier() const override;

private:
  int m_instanceId;
  int m_vendorId = 0;
  int m_productId = 0;
  int m_productVersion = 0;
  int m_playerIndex = -1;
  std::shared_ptr<GamepadProfile> m_profile;
};

} // namespace firelight::input