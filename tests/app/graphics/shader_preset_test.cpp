#include "graphics/shader_preset.hpp"

#include <gtest/gtest.h>

using namespace firelight::graphics;

namespace {
constexpr const char *kBase = "/tmp/shaders/my-shader";

TEST(ShaderPresetTest, ParsesPassesAndParameters) {
  const QString json = R"({
    "name": "CRT",
    "parameters": [
      { "key": "strength", "label": "Strength", "min": 0.0, "max": 1.0, "step": 0.05, "default": 0.4 }
    ],
    "passes": [
      { "fragment": "a.frag", "scale": 1.0, "filter": "linear", "wrap": "clamp" },
      { "fragment": "b.frag", "scale": 0.5, "filter": "nearest" }
    ]
  })";

  QString error;
  const auto preset =
      shader_preset::parse(json, kBase, "my-shader", kBase, &error);
  ASSERT_TRUE(preset.has_value()) << error.toStdString();
  EXPECT_EQ(preset->id.toStdString(), "my-shader");
  EXPECT_EQ(preset->name.toStdString(), "CRT");
  ASSERT_EQ(preset->passes.size(), 2);
  EXPECT_EQ(preset->passes[0].fragmentGlsl.toStdString(),
            std::string(kBase) + "/a.frag");
  EXPECT_FLOAT_EQ(preset->passes[1].scale, 0.5f);
  EXPECT_EQ(preset->passes[1].filter.toStdString(), "nearest");
  ASSERT_EQ(preset->parameters.size(), 1);
  EXPECT_EQ(preset->parameters[0].key.toStdString(), "strength");
  EXPECT_FLOAT_EQ(preset->parameters[0].defaultValue, 0.4f);
  EXPECT_FLOAT_EQ(preset->parameters[0].maxValue, 1.0f);
}

TEST(ShaderPresetTest, AbsolutePassPathsArePreserved) {
  const QString json = R"({ "passes": [ { "fragment": "/abs/x.frag" } ] })";
  const auto preset = shader_preset::parse(json, kBase, "x", kBase);
  ASSERT_TRUE(preset.has_value());
  EXPECT_EQ(preset->passes[0].fragmentGlsl.toStdString(), "/abs/x.frag");
}

TEST(ShaderPresetTest, RejectsInvalidJson) {
  QString error;
  const auto preset = shader_preset::parse("{ not json", kBase, "x", kBase,
                                           &error);
  EXPECT_FALSE(preset.has_value());
  EXPECT_FALSE(error.isEmpty());
}

TEST(ShaderPresetTest, RejectsMissingPasses) {
  QString error;
  const auto preset =
      shader_preset::parse(R"({ "name": "x" })", kBase, "x", kBase, &error);
  EXPECT_FALSE(preset.has_value());
}

TEST(ShaderPresetTest, RejectsEmptyPasses) {
  const auto preset =
      shader_preset::parse(R"({ "passes": [] })", kBase, "x", kBase);
  EXPECT_FALSE(preset.has_value());
}

TEST(ShaderPresetTest, DefaultsFilterAndScale) {
  const auto preset =
      shader_preset::parse(R"({ "passes": [ { "fragment": "a.frag" } ] })",
                           kBase, "x", kBase);
  ASSERT_TRUE(preset.has_value());
  EXPECT_FLOAT_EQ(preset->passes[0].scale, 1.0f);
  EXPECT_EQ(preset->passes[0].filter.toStdString(), "linear");
  EXPECT_EQ(preset->passes[0].wrap.toStdString(), "clamp");
}
} // namespace
