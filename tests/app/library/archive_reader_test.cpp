#include <firelight/library/archive_reader.hpp>

#include <algorithm>
#include <archive.h>
#include <archive_entry.h>
#include <filesystem>
#include <gtest/gtest.h>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace firelight::library {
class ArchiveReaderTest : public testing::Test {
protected:
  std::filesystem::path tempDir;

  void SetUp() override {
    std::random_device rd;
    tempDir = std::filesystem::temp_directory_path() / ("fl_artest_" + std::to_string(rd()));
    std::error_code ec;
    std::filesystem::create_directories(tempDir, ec);
  }

  void TearDown() override {
    std::error_code ec;
    std::filesystem::remove_all(tempDir, ec);
  }

  // Writes a .zip with the given (name, content) entries; returns its path
  std::string writeZip(const std::vector<std::pair<std::string, std::string>> &entries) {
    const std::string zipPath = (tempDir / "test.zip").string();

    archive *a = archive_write_new();
    archive_write_set_format_zip(a);
    EXPECT_EQ(archive_write_open_filename(a, zipPath.c_str()), ARCHIVE_OK);

    for (const auto &[name, content] : entries) {
      archive_entry *entry = archive_entry_new();
      archive_entry_set_pathname(entry, name.c_str());
      archive_entry_set_size(entry, static_cast<la_int64_t>(content.size()));
      archive_entry_set_filetype(entry, AE_IFREG);
      archive_entry_set_perm(entry, 0644);
      archive_write_header(a, entry);
      archive_write_data(a, content.data(), content.size());
      archive_entry_free(entry);
    }

    archive_write_close(a);
    archive_write_free(a);
    return zipPath;
  }
};

TEST_F(ArchiveReaderTest, ListsEntries) {
  const ArchiveReader reader(writeZip({{"a.txt", "hello"}, {"b.bin", "world!"}}));

  const auto entries = reader.listEntries();
  ASSERT_EQ(entries.size(), 2u);

  const auto byName = [&](const std::string &name) {
    return std::find_if(entries.begin(), entries.end(), [&](const auto &e) { return e.pathName == name; });
  };
  ASSERT_NE(byName("a.txt"), entries.end());
  ASSERT_EQ(byName("a.txt")->size, 5);
  ASSERT_NE(byName("b.bin"), entries.end());
  ASSERT_EQ(byName("b.bin")->size, 6);
}

TEST_F(ArchiveReaderTest, ReadsEntryByPathAndBaseName) {
  const ArchiveReader reader(writeZip({{"dir/a.txt", "hello"}}));

  const auto byPath = reader.readEntryByPath("dir/a.txt");
  ASSERT_EQ(std::string(byPath.begin(), byPath.end()), "hello");

  const auto byBase = reader.readEntryByBaseName("a.txt");
  ASSERT_EQ(std::string(byBase.begin(), byBase.end()), "hello");

  ASSERT_TRUE(reader.readEntryByPath("missing.txt").empty());
}

TEST_F(ArchiveReaderTest, ExtractsOnlyWantedEntries) {
  const ArchiveReader reader(writeZip({{"keep.bin", "data"}, {"skip.bin", "no"}}));

  ASSERT_TRUE(reader.extractEntries({"keep.bin"}, tempDir));
  ASSERT_TRUE(std::filesystem::exists(tempDir / "keep.bin"));
  ASSERT_FALSE(std::filesystem::exists(tempDir / "skip.bin"));
}

} // namespace firelight::library
