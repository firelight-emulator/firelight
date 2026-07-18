#include <firelight/input/input_frame.hpp>

#include <gtest/gtest.h>

namespace firelight::input {

namespace {
// A scriptable IRetroPad: returns preset button/analog values so the capture
// helper can be tested without SDL or a real controller
class FakePad : public libretro::IRetroPad {
public:
  uint16_t buttons = 0;
  int16_t lx = 0, ly = 0, rx = 0, ry = 0;

  bool isButtonPressed(int, int, Input b) override {
    const auto id = static_cast<unsigned>(b);
    return id < 16 && ((buttons >> id) & 1u) != 0;
  }

  int16_t getLeftStickXPosition(int, int) override { return lx; }

  int16_t getLeftStickYPosition(int, int) override { return ly; }

  int16_t getRightStickXPosition(int, int) override { return rx; }

  int16_t getRightStickYPosition(int, int) override { return ry; }

  void setStrongRumble(int, uint16_t) override {}

  void setWeakRumble(int, uint16_t) override {}
};
} // namespace

TEST(InputFrameTest, ButtonBitmaskAccessors) {
  InputFrame f;
  EXPECT_FALSE(f.button(0));
  f.setButton(3, true);
  f.setButton(15, true);
  EXPECT_TRUE(f.button(3));
  EXPECT_TRUE(f.button(15));
  EXPECT_FALSE(f.button(4));
  f.setButton(3, false);
  EXPECT_FALSE(f.button(3));
  // Out-of-range ids are ignored and never read as pressed
  f.setButton(20, true);
  EXPECT_FALSE(f.button(20));
}

TEST(InputFrameTest, ButtonMaskMatchesButtons) {
  InputFrame f;
  EXPECT_EQ(f.buttonMask(), 0);
  f.setButton(0, true);
  f.setButton(4, true);
  EXPECT_EQ(f.buttonMask(), static_cast<int16_t>((1u << 0) | (1u << 4)));
  EXPECT_EQ(f.buttonMask(), static_cast<int16_t>(f.buttons));
}

TEST(InputFrameTest, SerializeRoundTrip) {
  InputFrame f;
  f.setButton(0, true);
  f.setButton(9, true);
  f.leftStickX = 12345;
  f.leftStickY = -12345;
  f.rightStickX = 32767;
  f.rightStickY = -32768;

  const auto bytes = f.serialize();
  EXPECT_EQ(bytes.size(), InputFrame::SERIALIZED_SIZE);
  const auto back = InputFrame::deserialize(bytes);
  EXPECT_EQ(back, f);
}

TEST(InputFrameTest, CaptureReadsButtonsAndAxes) {
  FakePad pad;
  pad.buttons = static_cast<uint16_t>(1u << 1); // button id 1 pressed
  pad.lx = 1000;
  pad.ly = -2000;
  pad.rx = 3000;
  pad.ry = -4000;

  const auto f = captureJoypadFrame(pad, /*platformId=*/7, /*controllerTypeId=*/1);
  EXPECT_TRUE(f.button(1));
  EXPECT_FALSE(f.button(0));
  EXPECT_EQ(f.leftStickX, 1000);
  EXPECT_EQ(f.leftStickY, -2000);
  EXPECT_EQ(f.rightStickX, 3000);
  EXPECT_EQ(f.rightStickY, -4000);
}

} // namespace firelight::input
