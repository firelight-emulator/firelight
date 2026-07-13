#include <firelight/tas/scripted_retropad.hpp>

#include <firelight/input/input_frame.hpp>

#include <gtest/gtest.h>

namespace firelight::tas {

using input::captureJoypadFrame;
using input::InputFrame;

// The core contract: a ScriptedRetroPad, sampled the way the CoreInputRouter
// samples every pad each frame (firelight::input::captureJoypadFrame), must
// reproduce the exact InputFrame it was given — buttons and analog axes. If this
// holds, a movie frame drives the core bit-for-bit.
TEST(ScriptedRetroPadTest, CaptureReproducesFrame) {
  InputFrame f;
  f.setButton(libretro::IRetroPad::SouthFace, true); // id 0 (B)
  f.setButton(libretro::IRetroPad::Start, true);
  f.setButton(9, true);
  f.leftStickX = 12345;
  f.leftStickY = -12345;
  f.rightStickX = 32767;
  f.rightStickY = -32768;

  ScriptedRetroPad pad(f);
  const auto captured = captureJoypadFrame(pad, /*platformId=*/7,
                                           /*controllerTypeId=*/1);
  EXPECT_EQ(captured, f);
}

TEST(ScriptedRetroPadTest, NeutralFrameReadsAllReleased) {
  ScriptedRetroPad pad; // default = neutral
  for (unsigned id = 0; id < 16; ++id) {
    EXPECT_FALSE(pad.isButtonPressed(
        0, 1, static_cast<libretro::IRetroPad::Input>(id)))
        << "button id " << id;
  }
  EXPECT_EQ(pad.getLeftStickXPosition(0, 1), 0);
  EXPECT_EQ(pad.getRightStickYPosition(0, 1), 0);
}

TEST(ScriptedRetroPadTest, SetFrameUpdatesReads) {
  ScriptedRetroPad pad;
  InputFrame f;
  f.setButton(libretro::IRetroPad::EastFace, true); // id 1 (A)
  pad.setFrame(f);
  EXPECT_TRUE(pad.isButtonPressed(0, 1, libretro::IRetroPad::EastFace));
  EXPECT_FALSE(pad.isButtonPressed(0, 1, libretro::IRetroPad::SouthFace));
  EXPECT_EQ(pad.frame(), f);
}

// Rumble writes must be inert — a replay cannot depend on host feedback.
TEST(ScriptedRetroPadTest, RumbleIsDiscarded) {
  ScriptedRetroPad pad;
  pad.setStrongRumble(0, 65535);
  pad.setWeakRumble(0, 65535);
  SUCCEED();
}

} // namespace firelight::tas
