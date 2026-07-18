#include "cli/cli_app.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace firelight::cli {
namespace {

// Parses a synthetic command line. Owns the arg strings for the call (CLI11's
// int/argv overload does not modify them, but writable buffers are safest)
CliOptions parseArgs(std::vector<std::string> args) {
  std::vector<char *> argv;
  argv.reserve(args.size() + 1);
  for (auto &arg : args) {
    argv.push_back(arg.data());
  }
  argv.push_back(nullptr);
  return parseCli(static_cast<int>(args.size()), argv.data());
}

} // namespace

TEST(CliAppTest, NoArgsRunsGui) {
  const auto opts = parseArgs({"firelight"});
  EXPECT_EQ(opts.action, CliAction::RunGui);
  EXPECT_FALSE(opts.fullscreen);
  EXPECT_TRUE(opts.romPath.empty());
}

TEST(CliAppTest, FullscreenAndVerboseFlags) {
  const auto opts = parseArgs({"firelight", "--fullscreen", "--verbose"});
  EXPECT_EQ(opts.action, CliAction::RunGui);
  EXPECT_TRUE(opts.fullscreen);
  EXPECT_TRUE(opts.verbose);
}

TEST(CliAppTest, ConfigDirOption) {
  const auto opts = parseArgs({"firelight", "--config-dir", "C:/tmp/fl"});
  EXPECT_EQ(opts.configDir, "C:/tmp/fl");
  EXPECT_EQ(opts.action, CliAction::RunGui);
}

TEST(CliAppTest, PositionalRomPath) {
  const auto opts = parseArgs({"firelight", "/roms/game.gba"});
  EXPECT_EQ(opts.romPath, "/roms/game.gba");
  EXPECT_EQ(opts.action, CliAction::RunGui);
}

TEST(CliAppTest, ScanSubcommand) {
  const auto opts = parseArgs({"firelight", "scan"});
  EXPECT_EQ(opts.action, CliAction::RunScan);
}

TEST(CliAppTest, UnknownOptionExitsNonZero) {
  const auto opts = parseArgs({"firelight", "--does-not-exist"});
  EXPECT_EQ(opts.action, CliAction::Exit);
  EXPECT_NE(opts.exitCode, 0);
}

TEST(CliAppTest, VersionExitsZero) {
  const auto opts = parseArgs({"firelight", "--version"});
  EXPECT_EQ(opts.action, CliAction::Exit);
  EXPECT_EQ(opts.exitCode, 0);
}

TEST(CliAppTest, SetOptionsSplitIntoPairs) {
  const auto opts = parseArgs(
      {"firelight", "--set", "rewind-enabled=false", "--set", "sync-method=fixed"});
  EXPECT_EQ(opts.action, CliAction::RunGui);
  ASSERT_EQ(opts.sets.size(), 2u);
  EXPECT_EQ(opts.sets[0].first, "rewind-enabled");
  EXPECT_EQ(opts.sets[0].second, "false");
  EXPECT_EQ(opts.sets[1].first, "sync-method");
  EXPECT_EQ(opts.sets[1].second, "fixed");
}

TEST(CliAppTest, SetValueMayContainEquals) {
  const auto opts = parseArgs({"firelight", "--set", "path=a=b"});
  ASSERT_EQ(opts.sets.size(), 1u);
  EXPECT_EQ(opts.sets[0].first, "path");
  EXPECT_EQ(opts.sets[0].second, "a=b");
}

TEST(CliAppTest, MalformedSetExitsNonZero) {
  const auto opts = parseArgs({"firelight", "--set", "novalue"});
  EXPECT_EQ(opts.action, CliAction::Exit);
  EXPECT_NE(opts.exitCode, 0);
}

TEST(CliAppTest, MutedWindowedPausedFlags) {
  const auto opts =
      parseArgs({"firelight", "--muted", "--windowed", "--paused"});
  EXPECT_TRUE(opts.muted);
  EXPECT_TRUE(opts.windowed);
  EXPECT_TRUE(opts.paused);
  EXPECT_FALSE(opts.fullscreen);
}

TEST(CliAppTest, ExitOnCloseAndSingleInstanceFlags) {
  const auto opts =
      parseArgs({"firelight", "--exit-on-close", "--single-instance", "game.gba"});
  EXPECT_TRUE(opts.exitOnClose);
  EXPECT_TRUE(opts.singleInstance);
  EXPECT_EQ(opts.romPath, "game.gba");
  EXPECT_EQ(opts.action, CliAction::RunGui);
}

TEST(CliAppTest, FullscreenAndWindowedConflict) {
  const auto opts = parseArgs({"firelight", "--fullscreen", "--windowed"});
  EXPECT_EQ(opts.action, CliAction::Exit);
  EXPECT_NE(opts.exitCode, 0);
}

TEST(CliAppTest, SaveSlotControllerCoreOptions) {
  const auto opts = parseArgs({"firelight", "--save-slot", "3", "--controller",
                               "xbox360", "--core", "mgba_libretro"});
  EXPECT_EQ(opts.saveSlot, 3);
  EXPECT_EQ(opts.controller, "xbox360");
  EXPECT_EQ(opts.core, "mgba_libretro");
}

TEST(CliAppTest, SettingsFileOption) {
  const auto opts = parseArgs({"firelight", "--settings-file", "over.json"});
  EXPECT_EQ(opts.settingsFile, "over.json");
}

TEST(CliAppTest, LoginSubcommand) {
  const auto opts = parseArgs(
      {"firelight", "login", "--username", "player", "--token", "abc123"});
  EXPECT_EQ(opts.action, CliAction::Login);
  EXPECT_EQ(opts.raUsername, "player");
  EXPECT_EQ(opts.raToken, "abc123");
}

TEST(CliAppTest, RaStartupFlags) {
  const auto opts = parseArgs(
      {"firelight", "--ra-username", "player", "--ra-password", "pw", "game.gba"});
  EXPECT_EQ(opts.action, CliAction::RunGui);
  EXPECT_EQ(opts.raUsername, "player");
  EXPECT_EQ(opts.raPassword, "pw");
  EXPECT_EQ(opts.romPath, "game.gba");
}

TEST(CliAppTest, ListSettingsAndCoresActions) {
  EXPECT_EQ(parseArgs({"firelight", "--list-settings"}).action,
            CliAction::ListSettings);
  EXPECT_EQ(parseArgs({"firelight", "--list-cores"}).action,
            CliAction::ListCores);
}

} // namespace firelight::cli
