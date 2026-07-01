#include <firelight/input/analog_settings.hpp>
#include <firelight/input/binding.hpp>
#include <firelight/input/input_mapping.hpp>
#include <firelight/input/input_source.hpp>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

namespace firelight::input {

TEST(InputModelTest, InputSourceJsonRoundTrip) {
  InputSource source;
  source.type = SourceType::Button;
  source.code = GamepadInput::EastFace;
  source.modifiers = {GamepadInput::LeftTrigger, GamepadInput::RightTrigger};

  const nlohmann::json j = source;
  const auto restored = j.get<InputSource>();

  EXPECT_EQ(restored, source);
}

TEST(InputModelTest, InputSourceOmitsEmptyModifiers) {
  InputSource source;
  source.type = SourceType::AxisNegative;
  source.code = GamepadInput::LeftStickUp;

  const nlohmann::json j = source;
  EXPECT_FALSE(j.contains("modifiers"));
  EXPECT_EQ(j.get<InputSource>(), source);
}

TEST(InputModelTest, BindingDefaultsRoundTrip) {
  Binding binding;
  binding.source.type = SourceType::Button;
  binding.source.code = GamepadInput::SouthFace;

  const nlohmann::json j = binding;
  // Defaults should not be serialized.
  EXPECT_FALSE(j.contains("toggle"));
  EXPECT_FALSE(j.contains("turbo"));
  EXPECT_FALSE(j.contains("threshold"));

  EXPECT_EQ(j.get<Binding>(), binding);
}

TEST(InputModelTest, BindingWithOptionsRoundTrip) {
  Binding binding;
  binding.source.type = SourceType::Button;
  binding.source.code = GamepadInput::WestFace;
  binding.toggle = true;
  binding.turbo.enabled = true;
  binding.turbo.rateHz = 15.0f;
  binding.threshold = 0.8f;

  const nlohmann::json j = binding;
  EXPECT_TRUE(j.contains("toggle"));
  EXPECT_TRUE(j.contains("turbo"));

  EXPECT_EQ(j.get<Binding>(), binding);
}

TEST(InputModelTest, AnalogSettingsRoundTrip) {
  AnalogSettings settings;
  settings.leftStick.innerDeadzone = 0.1f;
  settings.leftStick.curve = ResponseCurve::Exponential;
  settings.leftStick.curveExponent = 1.5f;
  settings.rightStick.sensitivity = 1.3f;
  settings.leftTrigger.threshold = 0.3f;

  const nlohmann::json j = settings;
  EXPECT_EQ(j.get<AnalogSettings>(), settings);
}

TEST(InputModelTest, ApplyAxisSettingsZerosInsideDeadzone) {
  AxisSettings settings; // innerDeadzone == 0.25
  // 0.25 * 32767 ~= 8191; matches the previous hardcoded +-8192 behavior.
  EXPECT_EQ(applyAxisSettings(8000, settings), 0);
  EXPECT_EQ(applyAxisSettings(-8000, settings), 0);
}

TEST(InputModelTest, ApplyAxisSettingsPassesFullDeflection) {
  AxisSettings settings;
  EXPECT_EQ(applyAxisSettings(32767, settings), 32767);
  // Negative full deflection stays near the negative extreme.
  EXPECT_LT(applyAxisSettings(-32767, settings), -32000);
}

TEST(InputModelTest, ApplyAxisSettingsRescalesAfterDeadzone) {
  AxisSettings settings;
  settings.innerDeadzone = 0.5f;
  // Just past the deadzone should be near zero, not a sudden jump to half.
  const auto justPast = applyAxisSettings(static_cast<int>(0.51f * 32767), settings);
  EXPECT_GT(justPast, 0);
  EXPECT_LT(justPast, 2000);
}

TEST(InputModelTest, ApplyAxisSettingsNoDeadzoneIsLinear) {
  AxisSettings settings;
  settings.innerDeadzone = 0.0f;
  const auto half = applyAxisSettings(16383, settings);
  EXPECT_NEAR(half, 16383, 2);
}

TEST(InputModelTest, InputMappingCompatShim) {
  InputMapping mapping(1, 1, 1, 1);
  mapping.addMapping(GamepadInput::SouthFace, GamepadInput::EastFace);

  const auto mapped = mapping.getMappedInput(GamepadInput::SouthFace);
  ASSERT_TRUE(mapped.has_value());
  EXPECT_EQ(mapped.value(), GamepadInput::EastFace);

  const auto &bindings = mapping.getBindings(GamepadInput::SouthFace);
  ASSERT_EQ(bindings.size(), 1u);
  EXPECT_EQ(bindings.front().source.type, SourceType::Button);
  EXPECT_EQ(bindings.front().source.code, GamepadInput::EastFace);

  EXPECT_FALSE(mapping.getMappedInput(GamepadInput::NorthFace).has_value());
}

TEST(InputModelTest, InputMappingMultipleBindings) {
  InputMapping mapping(1, 1, 1, 1);

  Binding dpad;
  dpad.source.type = SourceType::Button;
  dpad.source.code = GamepadInput::DpadUp;
  Binding stick;
  stick.source.type = SourceType::AxisNegative;
  stick.source.code = GamepadInput::LeftStickUp;

  mapping.addBinding(GamepadInput::DpadUp, dpad);
  mapping.addBinding(GamepadInput::DpadUp, stick);
  EXPECT_EQ(mapping.getBindings(GamepadInput::DpadUp).size(), 2u);

  mapping.removeBinding(GamepadInput::DpadUp, 0);
  ASSERT_EQ(mapping.getBindings(GamepadInput::DpadUp).size(), 1u);
  EXPECT_EQ(mapping.getBindings(GamepadInput::DpadUp).front().source.type,
            SourceType::AxisNegative);
}

TEST(InputModelTest, InputMappingJsonRoundTrip) {
  InputMapping mapping(1, 1, 1, 1);

  Binding turbo;
  turbo.source.type = SourceType::Button;
  turbo.source.code = GamepadInput::EastFace;
  turbo.turbo.enabled = true;
  turbo.turbo.rateHz = 12.0f;
  mapping.setBindings(GamepadInput::SouthFace, {turbo});

  AnalogSettings override;
  override.leftStick.innerDeadzone = 0.05f;
  mapping.setAnalogOverride(override);

  const auto serialized = mapping.serialize();

  InputMapping restored(1, 1, 1, 1);
  restored.deserialize(serialized);

  EXPECT_EQ(restored.getBindings(GamepadInput::SouthFace),
            mapping.getBindings(GamepadInput::SouthFace));
  ASSERT_TRUE(restored.getAnalogOverride().has_value());
  EXPECT_EQ(restored.getAnalogOverride().value(), override);
}

TEST(InputModelTest, InputMappingToleratesLegacyData) {
  InputMapping mapping(1, 1, 1, 1);
  // Old CSV-style blob must not throw; it simply resets to no bindings.
  EXPECT_NO_THROW(mapping.deserialize("0:8,1:9,4:4,"));
  EXPECT_TRUE(mapping.getAllBindings().empty());
}

} // namespace firelight::input
