#include "fake_ram_core.hpp"

#include <firelight/cheats/cheat_engine.hpp>

#include <gtest/gtest.h>

namespace firelight::cheats {

TEST(CheatEngineTest, WritesPokesRespectingSizeEndiannessAndBounds) {
  FakeRamCore core;
  core.setSystemRamSize(256);

  CheatEngine engine;
  engine.setActivePokes({
      {0x10u, 0x2Au, 1, false},       // single byte
      {0x20u, 0x1234u, 2, false},     // 2 bytes, little-endian
      {0x30u, 0x1234u, 2, true},      // 2 bytes, big-endian
      {0xFEu, 0xDEADBEEFu, 4, false}, // 0xFE+4 > 256 -> out of range, skipped
  });
  engine.apply(core);

  const auto &ram = core.systemRam();
  EXPECT_EQ(ram[0x10], 0x2A);
  EXPECT_EQ(ram[0x20], 0x34); // LE: low byte first
  EXPECT_EQ(ram[0x21], 0x12);
  EXPECT_EQ(ram[0x30], 0x12); // BE: high byte first
  EXPECT_EQ(ram[0x31], 0x34);
  EXPECT_EQ(ram[0xFE], 0x00); // out-of-range poke left the byte untouched
  EXPECT_EQ(ram[0xFF], 0x00);
}

TEST(CheatEngineTest, NoWritableRamIsSafe) {
  FakeRamCore core; // no system RAM configured
  CheatEngine engine;
  engine.setActivePokes({{0u, 1u, 1, false}});
  engine.apply(core); // must not dereference a null pointer
  SUCCEED();
}

TEST(CheatEngineTest, EmptyEngineIsANoOp) {
  FakeRamCore core;
  core.setSystemRamSize(16);
  CheatEngine engine;
  EXPECT_TRUE(engine.empty());
  engine.apply(core);
  for (const auto byte : core.systemRam()) {
    EXPECT_EQ(byte, 0);
  }
}

} // namespace firelight::cheats
