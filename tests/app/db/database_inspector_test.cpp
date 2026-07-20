#include "db/database_inspector.hpp"

#include <SQLiteCpp/SQLiteCpp.h>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

namespace firelight::db {
namespace fs = std::filesystem;

namespace {
std::atomic_int g_counter{0};
} // namespace

class DatabaseInspectorTest : public testing::Test {
protected:
  fs::path m_tempDir;

  void SetUp() override {
    const auto unique =
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "_" + std::to_string(g_counter++);
    m_tempDir = fs::temp_directory_path() / ("fl_dbinspect_test_" + unique);
    fs::create_directories(m_tempDir);
  }

  void TearDown() override {
    std::error_code ec;
    fs::remove_all(m_tempDir, ec);
  }

  [[nodiscard]] std::string pathFor(const std::string &name) const { return (m_tempDir / name).string(); }
};

TEST_F(DatabaseInspectorTest, MissingFileReportsNotExists) {
  const auto info = inspect(pathFor("nope.db"));
  EXPECT_FALSE(info.exists);
  EXPECT_FALSE(info.opens);
}

TEST_F(DatabaseInspectorTest, HealthyDatabaseReportsTablesAndVersion) {
  const auto path = pathFor("healthy.db");
  {
    SQLite::Database db(path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    db.exec("CREATE TABLE a (id INTEGER PRIMARY KEY)");
    db.exec("CREATE TABLE b (id INTEGER PRIMARY KEY)");
    db.exec("PRAGMA user_version = 4");
  }

  const auto info = inspect(path);
  EXPECT_TRUE(info.exists);
  EXPECT_TRUE(info.opens);
  EXPECT_TRUE(info.integrityOk);
  EXPECT_EQ(info.userVersion, 4);
  EXPECT_EQ(info.tableCount, 2);
  EXPECT_TRUE(info.error.empty());
}

TEST_F(DatabaseInspectorTest, UnversionedDatabaseReportsVersionZero) {
  const auto path = pathFor("unversioned.db");
  {
    SQLite::Database db(path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    db.exec("CREATE TABLE only (id INTEGER PRIMARY KEY)");
  }

  const auto info = inspect(path);
  EXPECT_TRUE(info.opens);
  EXPECT_EQ(info.userVersion, 0);
}

TEST_F(DatabaseInspectorTest, NonDatabaseFileFailsToOpen) {
  const auto path = pathFor("garbage.db");
  {
    std::ofstream out(path, std::ios::binary);
    out << "this is definitely not a sqlite database";
  }

  const auto info = inspect(path);
  EXPECT_TRUE(info.exists);
  EXPECT_FALSE(info.integrityOk);
  EXPECT_FALSE(info.error.empty());
}

TEST_F(DatabaseInspectorTest, InspectingDoesNotCreateAFile) {
  const auto path = pathFor("absent.db");
  inspect(path);
  EXPECT_FALSE(fs::exists(path));
}

} // namespace firelight::db
