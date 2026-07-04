#include <gtest/gtest.h>

#include <firelight/input/gamepad_input.hpp>
#include <firelight/input/sdl_input_service.hpp>
#include <firelight/input/sqlite_controller_repository.hpp>
#include <firelight/libretro/pointer_input_provider.hpp>

namespace firelight::input {

// --- cursorGlideDelta: per-frame velocity glide ------------------------------

TEST(AnalogCursorTest, GlideDeltaIsZeroWhenStickCentered) {
  EXPECT_EQ(libretro::cursorGlideDelta(0, 0.025), 0);
}

TEST(AnalogCursorTest, GlideDeltaScalesWithDeflection) {
  const int full = libretro::cursorGlideDelta(32767, 0.025);
  const int half = libretro::cursorGlideDelta(16384, 0.025);
  EXPECT_GT(full, 0);
  // Roughly proportional to how far the stick is pushed.
  EXPECT_NEAR(half, full / 2, 2);
}

TEST(AnalogCursorTest, GlideDeltaSignFollowsStick) {
  EXPECT_LT(libretro::cursorGlideDelta(-32767, 0.025), 0);
  EXPECT_GT(libretro::cursorGlideDelta(32767, 0.025), 0);
}

TEST(AnalogCursorTest, GlideDeltaFasterSpeedMovesFarther) {
  EXPECT_GT(libretro::cursorGlideDelta(32767, 0.040),
            libretro::cursorGlideDelta(32767, 0.015));
}

// --- defaultPhysicalBinding: sensible out-of-the-box gamepad buttons ---------

TEST(AnalogCursorTest, AbstractGunAndMouseButtonsDefaultToFaceButtons) {
  EXPECT_EQ(defaultPhysicalBinding(LightgunTrigger), SouthFace);
  EXPECT_EQ(defaultPhysicalBinding(MouseLeft), SouthFace);
  EXPECT_EQ(defaultPhysicalBinding(LightgunReload), EastFace);
  EXPECT_EQ(defaultPhysicalBinding(MouseRight), EastFace);
}

TEST(AnalogCursorTest, JoypadInputsDefaultToThemselves) {
  EXPECT_EQ(defaultPhysicalBinding(EastFace), EastFace);
  EXPECT_EQ(defaultPhysicalBinding(DpadUp), DpadUp);
  EXPECT_EQ(defaultPhysicalBinding(Start), Start);
}

// --- SDLInputService::nudgeCursor: the shared cursor mutation -----------------

class NudgeCursorTest : public testing::Test {
protected:
  SqliteControllerRepository m_repo{":memory:"};
  SDLInputService m_service{m_repo};
};

TEST_F(NudgeCursorTest, MovesCursorAndClearsOffscreen) {
  m_service.updateMouseOffscreen(true);
  m_service.nudgeCursor(100, -50);
  const auto [x, y] = m_service.getPointerPosition();
  EXPECT_EQ(x, 100);
  EXPECT_EQ(y, -50);
  EXPECT_FALSE(m_service.isPointerOffscreen());
}

TEST_F(NudgeCursorTest, AccumulatesAcrossFrames) {
  m_service.nudgeCursor(100, 100);
  m_service.nudgeCursor(100, 100);
  const auto [x, y] = m_service.getPointerPosition();
  EXPECT_EQ(x, 200);
  EXPECT_EQ(y, 200);
}

TEST_F(NudgeCursorTest, ClampsToPointerRange) {
  m_service.nudgeCursor(40000, -40000); // beyond the ±32767 pointer range
  const auto [x, y] = m_service.getPointerPosition();
  EXPECT_EQ(x, 32767);
  EXPECT_EQ(y, -32767);
}

TEST_F(NudgeCursorTest, FeedsRelativeMotionForMouseDevices) {
  m_service.nudgeCursor(30, -20);
  const auto [dx, dy] = m_service.getRelativeMotion();
  EXPECT_EQ(dx, 30);
  EXPECT_EQ(dy, -20);
}

TEST_F(NudgeCursorTest, ZeroNudgeLeavesTheCursorAlone) {
  m_service.updateMouseState(0.5, 0.5, false); // absolute set by the mouse
  m_service.nudgeCursor(0, 0);
  const auto [x, y] = m_service.getPointerPosition();
  EXPECT_EQ(x, static_cast<int16_t>(0.5 * 32767));
  EXPECT_EQ(y, static_cast<int16_t>(0.5 * 32767));
}

} // namespace firelight::input
