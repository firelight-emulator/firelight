#include "cli/cli_app.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace firelight::cli {
namespace {

// Parses a synthetic command line. Owns the arg strings for the call (CLI11's
// int/argv overload does not modify them, but writable buffers are safest).
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

} // namespace firelight::cli
