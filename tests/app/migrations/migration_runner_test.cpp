#include <firelight/migrations/migration_runner.hpp>

#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace firelight::migrations {
namespace {

TEST(MigrationRunnerTest, FreshDatabaseRunsAllInOrder) {
  std::vector<std::string> log;
  const std::vector<Migration> migrations = {
      {1, [&] { log.emplace_back("v1"); }},
      {2, [&] { log.emplace_back("v2"); }},
      {3, [&] { log.emplace_back("v3"); }},
  };
  int stamped = -1;
  const int result = applyMigrations(0, migrations, [&](int v) { stamped = v; });

  EXPECT_EQ(result, 3);
  EXPECT_EQ(stamped, 3);
  EXPECT_EQ(log, (std::vector<std::string>{"v1", "v2", "v3"}));
}

TEST(MigrationRunnerTest, ReopenAtLatestRunsNothing) {
  std::vector<std::string> log;
  const std::vector<Migration> migrations = {
      {1, [&] { log.emplace_back("v1"); }},
      {2, [&] { log.emplace_back("v2"); }},
  };
  int stampCount = 0;
  const int result = applyMigrations(2, migrations, [&](int) { stampCount++; });

  EXPECT_EQ(result, 2);
  EXPECT_TRUE(log.empty());
  EXPECT_EQ(stampCount, 0);
}

TEST(MigrationRunnerTest, PartialUpgradeRunsOnlyNewer) {
  std::vector<std::string> log;
  const std::vector<Migration> migrations = {
      {1, [&] { log.emplace_back("v1"); }},
      {2, [&] { log.emplace_back("v2"); }},
      {3, [&] { log.emplace_back("v3"); }},
  };
  int stamped = -1;
  const int result = applyMigrations(1, migrations, [&](int v) { stamped = v; });

  EXPECT_EQ(result, 3);
  EXPECT_EQ(stamped, 3);
  EXPECT_EQ(log, (std::vector<std::string>{"v2", "v3"}));
}

} // namespace
} // namespace firelight::migrations
