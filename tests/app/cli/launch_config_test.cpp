#include "cli/launch_config.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <map>
#include <string>

namespace firelight::cli {
namespace {

std::string writeTempFile(const std::string &name, const std::string &contents) {
  const auto path =
      (std::filesystem::temp_directory_path() / name).string();
  std::ofstream out(path, std::ios::binary);
  out << contents;
  out.close();
  return path;
}

std::map<std::string, std::string> asMap(const std::vector<OverridePair> &pairs) {
  std::map<std::string, std::string> result;
  for (const auto &[key, value] : pairs) {
    result[key] = value;
  }
  return result;
}

} // namespace

TEST(LaunchConfigTest, ParsesPropertiesFile) {
  const auto path = writeTempFile("fl_over.properties", "# a comment\n"
                                                        "rewind-enabled=false\n"
                                                        "; another comment\n"
                                                        "\n"
                                                        "target-framerate = 120\n"
                                                        "sync-method=fixed\n");
  const auto pairs = loadOverrideFile(path);
  const auto map = asMap(pairs);
  EXPECT_EQ(map.at("rewind-enabled"), "false");
  EXPECT_EQ(map.at("target-framerate"), "120");
  EXPECT_EQ(map.at("sync-method"), "fixed");
  EXPECT_EQ(map.size(), 3u);
  std::filesystem::remove(path);
}

TEST(LaunchConfigTest, ParsesJsonFileWithMixedScalarTypes) {
  const auto path = writeTempFile(
      "fl_over.json", R"({"rewind-enabled": false, "target-framerate": 120,
                          "sync-method": "fixed"})");
  const auto map = asMap(loadOverrideFile(path));
  EXPECT_EQ(map.at("rewind-enabled"), "false");
  EXPECT_EQ(map.at("target-framerate"), "120");
  EXPECT_EQ(map.at("sync-method"), "fixed");
  std::filesystem::remove(path);
}

TEST(LaunchConfigTest, PropertiesAndJsonAgree) {
  const auto propsPath = writeTempFile(
      "fl_a.properties", "rewind-enabled=false\nsync-method=fixed\n");
  const auto jsonPath = writeTempFile(
      "fl_a.json", R"({"rewind-enabled": "false", "sync-method": "fixed"})");
  EXPECT_EQ(asMap(loadOverrideFile(propsPath)),
            asMap(loadOverrideFile(jsonPath)));
  std::filesystem::remove(propsPath);
  std::filesystem::remove(jsonPath);
}

TEST(LaunchConfigTest, MissingFileThrows) {
  EXPECT_THROW(loadOverrideFile("/no/such/file.properties"), std::runtime_error);
}

TEST(LaunchConfigTest, MalformedPropertiesLineThrows) {
  const auto path = writeTempFile("fl_bad.properties", "this-has-no-equals\n");
  EXPECT_THROW(loadOverrideFile(path), std::runtime_error);
  std::filesystem::remove(path);
}

TEST(LaunchConfigTest, MalformedJsonThrows) {
  const auto path = writeTempFile("fl_bad.json", "{ not valid json ");
  EXPECT_THROW(loadOverrideFile(path), std::runtime_error);
  std::filesystem::remove(path);
}

TEST(LaunchConfigTest, JsonArrayIsRejected) {
  // A JSON document that isn't an object of key/value pairs is an error
  const auto path = writeTempFile("fl_arr.json", R"(["a", "b"])");
  EXPECT_THROW(loadOverrideFile(path), std::runtime_error);
  std::filesystem::remove(path);
}

TEST(LaunchConfigTest, CommentAndBlankOnlyPropertiesYieldsNothing) {
  const auto path =
      writeTempFile("fl_comments.properties", "# just a comment\n\n; and another\n   \n");
  EXPECT_TRUE(loadOverrideFile(path).empty());
  std::filesystem::remove(path);
}

TEST(LaunchConfigTest, PropertiesValueKeepsInnerSpaces) {
  const auto path = writeTempFile("fl_spaces.properties", "label = hello world\n");
  const auto map = asMap(loadOverrideFile(path));
  EXPECT_EQ(map.at("label"), "hello world");
  std::filesystem::remove(path);
}

TEST(LaunchConfigTest, ParseControllerTypeKnownNames) {
  EXPECT_EQ(parseControllerType("xbox360"), MICROSOFT_XBOX_360);
  EXPECT_EQ(parseControllerType("dualshock4"), SONY_DUALSHOCK_4);
  EXPECT_EQ(parseControllerType("switch-pro"), NINTENDO_SWITCH_PRO);
  // Case-insensitive
  EXPECT_EQ(parseControllerType("DualSense"), SONY_DUALSENSE);
}

TEST(LaunchConfigTest, ParseControllerTypeUnknownReturnsNullopt) {
  EXPECT_FALSE(parseControllerType("not-a-controller").has_value());
}

} // namespace firelight::cli
