#include "test_gamepad.hpp"

namespace firelight::input {
TestGamepad::TestGamepad(const int instanceId, const int vendorId,
                         const int productId, const int productVersion)
    : m_instanceId(instanceId), m_vendorId(vendorId), m_productId(productId),
      m_productVersion(productVersion) {}

bool TestGamepad::isButtonPressed(int platformId, int controllerTypeId,
                                  Input t_button) {
  return false;
}
int16_t TestGamepad::getLeftStickXPosition(int platformId,
                                           int controllerTypeId) {
  return 0;
}
int16_t TestGamepad::getLeftStickYPosition(int platformId,
                                           int controllerTypeId) {
  return 0;
}
int16_t TestGamepad::getRightStickXPosition(int platformId,
                                            int controllerTypeId) {
  return 0;
}
int16_t TestGamepad::getRightStickYPosition(int platformId,
                                            int controllerTypeId) {
  return 0;
}
void TestGamepad::setStrongRumble(int platformId, uint16_t t_strength) {}
void TestGamepad::setWeakRumble(int platformId, uint16_t t_strength) {}

std::shared_ptr<GamepadProfile> TestGamepad::getProfile() const {
  return m_profile;
}
void TestGamepad::setProfile(const std::shared_ptr<GamepadProfile> &profile) {
  m_profile = profile;
}

std::vector<Shortcut> TestGamepad::getToggledShortcuts(GamepadInput input) {
  return {};
}
int16_t TestGamepad::evaluateRawInput(const GamepadInput input) const {
  return 0;
}

std::string TestGamepad::getName() const { return "TestGamepad"; }

int TestGamepad::getPlayerIndex() const { return m_playerIndex; }

void TestGamepad::setPlayerIndex(int playerIndex) {
  m_playerIndex = playerIndex;
}

int TestGamepad::getInstanceId() const { return m_instanceId; }

bool TestGamepad::isWired() const { return true; }
GamepadType TestGamepad::getType() const { return MICROSOFT_XBOX_ONE; }

bool TestGamepad::disconnect() { return true; }

DeviceIdentifier TestGamepad::getDeviceIdentifier() const {
  return DeviceIdentifier{
      .deviceName = "TestGamepad",
      .vendorId = m_vendorId,
      .productId = m_productId,
      .productVersion = m_productVersion,
  };
}

} // namespace firelight::input