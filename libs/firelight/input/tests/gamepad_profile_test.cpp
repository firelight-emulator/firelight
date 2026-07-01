#include <firelight/input/gamepad_profile.hpp>
#include <firelight/input/gamepad_type.hpp>
#include <firelight/input/input_mapping.hpp>

#include <gtest/gtest.h>

namespace firelight::input {

TEST(GamepadProfileTest, GetMappingHitAndMiss) {
  GamepadProfile profile(1);
  const auto mapping = std::make_shared<InputMapping>(1, 1, 5, 1);
  profile.addMapping(mapping);

  EXPECT_EQ(profile.getMappingForPlatformAndController(5, 1), mapping);
  EXPECT_EQ(profile.getMappingForPlatformAndController(99, 1), nullptr);
  EXPECT_EQ(profile.getMappingForPlatformAndController(5, 2), nullptr);
}

TEST(GamepadProfileTest, AddMappingDeDupsByPlatformAndType) {
  GamepadProfile profile(1);
  const auto first = std::make_shared<InputMapping>(1, 1, 5, 1);
  const auto second = std::make_shared<InputMapping>(2, 1, 5, 1); // same key
  profile.addMapping(first);
  profile.addMapping(second);

  // The first mapping wins; the duplicate is ignored.
  EXPECT_EQ(profile.getMappingForPlatformAndController(5, 1), first);
}

TEST(GamepadProfileTest, AnalogSettingsFallBackToDefault) {
  GamepadProfile profile(1);
  const auto mapping = std::make_shared<InputMapping>(1, 1, 5, 1);
  profile.addMapping(mapping);

  AnalogSettings profileDefault;
  profileDefault.leftStick.innerDeadzone = 0.3f;
  profile.setDefaultAnalogSettings(profileDefault);

  // No per-platform override -> profile default.
  EXPECT_EQ(profile.getAnalogSettings(5, 1), profileDefault);

  // With an override on the platform mapping -> the override wins.
  AnalogSettings override;
  override.leftStick.innerDeadzone = 0.05f;
  mapping->setAnalogOverride(override);
  EXPECT_EQ(profile.getAnalogSettings(5, 1), override);
}

TEST(GamepadProfileTest, Metadata) {
  GamepadProfile profile(7);
  EXPECT_EQ(profile.getId(), 7);

  profile.setName("My Profile");
  EXPECT_EQ(profile.getName(), "My Profile");

  profile.setIsKeyboardProfile(true);
  EXPECT_TRUE(profile.isKeyboardProfile());

  profile.setBuiltin(true);
  EXPECT_TRUE(profile.isBuiltin());

  profile.setBasedOnType(NINTENDO_NSO_N64);
  EXPECT_EQ(profile.getBasedOnType(), static_cast<int>(NINTENDO_NSO_N64));

  profile.setIcon("gamepad.png");
  EXPECT_EQ(profile.getIcon(), "gamepad.png");
}

} // namespace firelight::input
