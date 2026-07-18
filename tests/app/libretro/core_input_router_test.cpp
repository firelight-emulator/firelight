#include <libretro/core_input_router.hpp>

#include "libretro/libretro.h"
#include <gtest/gtest.h>
#include <map>
#include <memory>

namespace firelight::libretro {
namespace {

// Scriptable retropad: preset buttons/axes + active virtual inputs
class FakePad : public IRetroPad {
public:
  uint16_t buttons = 0;
  int16_t lx = 0, ly = 0, rx = 0, ry = 0;
  std::map<int, bool> virtualInputs;

  bool isButtonPressed(int, int, Input b) override {
    const auto id = static_cast<unsigned>(b);
    return id < 16 && ((buttons >> id) & 1u) != 0;
  }
  bool isVirtualInputActive(int, int, int virtualInput) override {
    const auto it = virtualInputs.find(virtualInput);
    return it != virtualInputs.end() && it->second;
  }
  int16_t getLeftStickXPosition(int, int) override { return lx; }
  int16_t getLeftStickYPosition(int, int) override { return ly; }
  int16_t getRightStickXPosition(int, int) override { return rx; }
  int16_t getRightStickYPosition(int, int) override { return ry; }
  void setStrongRumble(int, uint16_t) override {}
  void setWeakRumble(int, uint16_t) override {}
};

// Hands the router one pad on player 0; every other slot is empty
class FakeProvider : public IRetropadProvider {
public:
  std::shared_ptr<FakePad> pad = std::make_shared<FakePad>();
  std::shared_ptr<IRetroPad> getRetropadForPlayerIndex(int player) override {
    return player == 0 ? pad : nullptr;
  }
};

class FakePointer : public IPointerInputProvider {
public:
  std::pair<int16_t, int16_t> position{0, 0};
  std::pair<int16_t, int16_t> motion{0, 0};
  bool pressed = false;
  bool offscreen = false;
  std::map<int, bool> mouseButtons;

  [[nodiscard]] std::pair<int16_t, int16_t> getPointerPosition() const override {
    return position;
  }
  [[nodiscard]] bool isPressed() const override { return pressed; }
  std::pair<int16_t, int16_t> getRelativeMotion() override { return motion; }
  [[nodiscard]] bool isMouseButtonPressed(int id) const override {
    const auto it = mouseButtons.find(id);
    return it != mouseButtons.end() && it->second;
  }
  [[nodiscard]] bool isPointerOffscreen() const override { return offscreen; }
};

class CoreInputRouterTest : public testing::Test {
protected:
  CoreInputRouter router;
  FakeProvider provider;
  FakePointer pointer;

  void SetUp() override {
    router.setRetropadProvider(&provider);
    router.setPointerInputProvider(&pointer);
  }
};

TEST_F(CoreInputRouterTest, UnsetPortDefaultsToJoypadClass) {
  EXPECT_EQ(router.getPortInputClass(0),
            static_cast<int>(firelight::input::GamepadInputClass::Joypad));
}

TEST_F(CoreInputRouterTest, JoypadButtonAndMaskReadFromSnapshot) {
  provider.pad->buttons = static_cast<uint16_t>(1u << RETRO_DEVICE_ID_JOYPAD_A);
  router.pollInput();

  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_JOYPAD, 0,
                                  RETRO_DEVICE_ID_JOYPAD_A),
            1);
  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_JOYPAD, 0,
                                  RETRO_DEVICE_ID_JOYPAD_B),
            0);
  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_JOYPAD, 0,
                                  RETRO_DEVICE_ID_JOYPAD_MASK),
            static_cast<int16_t>(1u << RETRO_DEVICE_ID_JOYPAD_A));
}

TEST_F(CoreInputRouterTest, AnalogAxesReadFromSnapshot) {
  provider.pad->lx = 1234;
  provider.pad->ry = -4321;
  router.pollInput();

  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_ANALOG,
                                  RETRO_DEVICE_INDEX_ANALOG_LEFT,
                                  RETRO_DEVICE_ID_ANALOG_X),
            1234);
  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_ANALOG,
                                  RETRO_DEVICE_INDEX_ANALOG_RIGHT,
                                  RETRO_DEVICE_ID_ANALOG_Y),
            -4321);
}

TEST_F(CoreInputRouterTest, EmptyPortIsInactive) {
  router.pollInput();
  // Player slot 3 has no pad, so any read is zero
  EXPECT_EQ(router.readInputState(3, RETRO_DEVICE_JOYPAD, 0,
                                  RETRO_DEVICE_ID_JOYPAD_A),
            0);
}

TEST_F(CoreInputRouterTest, MouseMotionAndButtonFromPointer) {
  pointer.motion = {5, -7};
  pointer.mouseButtons[RETRO_DEVICE_ID_MOUSE_LEFT] = true;
  router.pollInput();

  EXPECT_EQ(
      router.readInputState(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X),
      5);
  EXPECT_EQ(
      router.readInputState(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y),
      -7);
  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_MOUSE, 0,
                                  RETRO_DEVICE_ID_MOUSE_LEFT),
            1);
}

TEST_F(CoreInputRouterTest, DeviceIsInertWhenNeitherSourceAllowed) {
  // Mouse toggle off + port left on Joypad => the mouse device is inert
  router.setMouseControlsPointerDevices(false);
  pointer.motion = {9, 9};
  router.pollInput();

  EXPECT_EQ(
      router.readInputState(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X),
      0);
}

TEST_F(CoreInputRouterTest, LightgunAimAndTriggerFromPointer) {
  pointer.position = {100, 200};
  pointer.pressed = true;
  router.pollInput();

  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_LIGHTGUN, 0,
                                  RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X),
            100);
  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_LIGHTGUN, 0,
                                  RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y),
            200);
  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_LIGHTGUN, 0,
                                  RETRO_DEVICE_ID_LIGHTGUN_TRIGGER),
            1);
}

TEST_F(CoreInputRouterTest, GamepadMappedLightgunButtonFires) {
  // Port selected as a light gun => a mapped gamepad binding drives it even
  // with the physical mouse disabled
  router.setMouseControlsPointerDevices(false);
  router.setPortInputDeviceClass(
      0, static_cast<int>(firelight::input::GamepadInputClass::Lightgun));
  provider.pad->virtualInputs[static_cast<int>(
      firelight::input::LightgunAuxA)] = true;
  router.pollInput();

  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_LIGHTGUN, 0,
                                  RETRO_DEVICE_ID_LIGHTGUN_AUX_A),
            1);
}

} // namespace
} // namespace firelight::libretro
