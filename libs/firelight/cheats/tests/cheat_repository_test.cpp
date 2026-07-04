#include <firelight/cheats/sqlite_cheat_repository.hpp>

#include <gtest/gtest.h>

namespace firelight::cheats {

TEST(SqliteCheatRepositoryTest, RoundTripsTypedCheatsWithPokes) {
  SqliteCheatRepository repo(":memory:");
  const std::string hash = "abc123";

  EXPECT_TRUE(repo.getCheats(hash).empty());

  // A RAM cheat (GameShark-style) carries resolved pokes.
  Cheat a{.contentHash = hash,
          .name = "Infinite Lives",
          .type = CheatType::GameShark,
          .rawCode = "01C1-0003",
          .pokes = {{0x00C1u, 3u, 1u, false}},
          .enabled = true,
          .affectsHardcore = true};
  ASSERT_TRUE(repo.addCheat(a));
  EXPECT_GT(a.id, 0);
  EXPECT_EQ(a.ordinal, 0);

  // A core-applied cheat (Game Genie) has a raw code and no pokes.
  Cheat b{.contentHash = hash,
          .name = "Moon Jump",
          .type = CheatType::GameGenie,
          .rawCode = "SXIOPO",
          .enabled = false,
          .affectsHardcore = true};
  ASSERT_TRUE(repo.addCheat(b));
  EXPECT_EQ(b.ordinal, 1);

  auto cheats = repo.getCheats(hash);
  ASSERT_EQ(cheats.size(), 2u);

  EXPECT_EQ(cheats[0].name, "Infinite Lives");
  EXPECT_EQ(cheats[0].type, CheatType::GameShark);
  EXPECT_EQ(cheats[0].rawCode, "01C1-0003");
  ASSERT_EQ(cheats[0].pokes.size(), 1u);
  EXPECT_EQ(cheats[0].pokes[0].address, 0x00C1u);
  EXPECT_EQ(cheats[0].pokes[0].value, 3u);
  EXPECT_EQ(cheats[0].pokes[0].size, 1u);
  EXPECT_FALSE(cheats[0].isCoreApplied());

  EXPECT_EQ(cheats[1].type, CheatType::GameGenie);
  EXPECT_TRUE(cheats[1].pokes.empty());
  EXPECT_TRUE(cheats[1].isCoreApplied()); // no pokes + a code -> core-applied

  // Toggle + full edit persist.
  EXPECT_TRUE(repo.setEnabled(b.id, true));
  EXPECT_TRUE(repo.getCheats(hash)[1].enabled);

  b.enabled = true;
  b.name = "Moon Jump!";
  b.affectsHardcore = false;
  EXPECT_TRUE(repo.updateCheat(b));
  cheats = repo.getCheats(hash);
  EXPECT_EQ(cheats[1].name, "Moon Jump!");
  EXPECT_FALSE(cheats[1].affectsHardcore);

  // Scoped per game.
  EXPECT_TRUE(repo.getCheats("some-other-hash").empty());

  // Remove.
  EXPECT_TRUE(repo.removeCheat(a.id));
  ASSERT_EQ(repo.getCheats(hash).size(), 1u);
  EXPECT_FALSE(repo.removeCheat(999999));
}

} // namespace firelight::cheats
