#include <firelight/input/gamepad_input.hpp>

#include <gtest/gtest.h>
#include <string_view>

namespace firelight::input {

// Device-neutral labels the mapping UI renders for each joypad input.
TEST(GamepadInputDisplayNameTest, JoypadLabels) {
  EXPECT_EQ(std::string_view(displayName(SouthFace)), "South Face");
  EXPECT_EQ(std::string_view(displayName(EastFace)), "East Face");
  EXPECT_EQ(std::string_view(displayName(WestFace)), "West Face");
  EXPECT_EQ(std::string_view(displayName(NorthFace)), "North Face");
  EXPECT_EQ(std::string_view(displayName(LeftBumper)), "Left Bumper");
  EXPECT_EQ(std::string_view(displayName(RightBumper)), "Right Bumper");
  EXPECT_EQ(std::string_view(displayName(Start)), "Start");
  EXPECT_EQ(std::string_view(displayName(Select)), "Select");
  EXPECT_EQ(std::string_view(displayName(DpadUp)), "D-Pad Up");
  EXPECT_EQ(std::string_view(displayName(DpadRight)), "D-Pad Right");
  EXPECT_EQ(std::string_view(displayName(LeftStickUp)), "Left Stick Up");
  EXPECT_EQ(std::string_view(displayName(RightStickRight)), "Right Stick Right");
}

// Mouse inputs get button-oriented labels (the device class disambiguates the
// generic "Left Button"/"Right Button" wording from a joypad's).
TEST(GamepadInputDisplayNameTest, MouseLabels) {
  EXPECT_EQ(std::string_view(displayName(MouseX)), "Mouse X");
  EXPECT_EQ(std::string_view(displayName(MouseY)), "Mouse Y");
  EXPECT_EQ(std::string_view(displayName(MouseLeft)), "Left Button");
  EXPECT_EQ(std::string_view(displayName(MouseRight)), "Right Button");
  EXPECT_EQ(std::string_view(displayName(MouseMiddle)), "Middle Button");
  EXPECT_EQ(std::string_view(displayName(MouseButton4)), "Button 4");
}

TEST(GamepadInputDisplayNameTest, LightgunLabels) {
  EXPECT_EQ(std::string_view(displayName(LightgunTrigger)), "Trigger");
  EXPECT_EQ(std::string_view(displayName(LightgunReload)), "Reload");
  EXPECT_EQ(std::string_view(displayName(LightgunAuxA)), "Aux A");
  EXPECT_EQ(std::string_view(displayName(LightgunStart)), "Start");
  EXPECT_EQ(std::string_view(displayName(LightgunDpadLeft)), "D-Pad Left");
}

// An input value outside the labeled set falls back to "Unknown" (the switch's
// default), so the UI never renders an empty label.
TEST(GamepadInputDisplayNameTest, UnknownValueFallsBack) {
  EXPECT_EQ(std::string_view(displayName(static_cast<GamepadInput>(99999))),
            "Unknown");
}

// displayName agrees with classOf on device family for representative inputs
// (the label set and the class mask are derived independently).
TEST(GamepadInputDisplayNameTest, LabelsAlignWithDeviceClass) {
  EXPECT_EQ(classOf(SouthFace), GamepadInputClass::Joypad);
  EXPECT_EQ(classOf(MouseLeft), GamepadInputClass::Mouse);
  EXPECT_EQ(classOf(LightgunTrigger), GamepadInputClass::Lightgun);
}

} // namespace firelight::input
