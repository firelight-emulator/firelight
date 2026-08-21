#include <firelight/library/disc_set_playlist.hpp>

#include <gtest/gtest.h>

// The playlist is what gives a set one identity and one memory card, and its first line is
// what that identity is read from. These pin the ordering and the derived path
namespace firelight::library {

namespace {
ContentFile disc(const int number, const std::string &path, const std::string &contentHash = "") {
  ContentFile file;
  file.m_type = ContentType::Disc;
  file.m_discNumber = number;
  file.m_filePath = path;
  file.m_contentHash = contentHash;
  return file;
}

std::vector<std::string> linesOf(const std::string &contents) {
  std::vector<std::string> lines;
  size_t start = 0;

  while (start < contents.size()) {
    const auto end = contents.find('\n', start);

    if (end == std::string::npos) {
      lines.push_back(contents.substr(start));
      break;
    }

    lines.push_back(contents.substr(start, end - start));
    start = end + 1;
  }

  return lines;
}
} // namespace

// The file is ours and lives where we put it, so the lines say exactly where each disc is
TEST(DiscSetPlaylistTest, WritesAbsoluteLinesUnderAppData) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/FF7 (Disc 1).chd", "a"),
                                       disc(2, "C:/roms/FF7 (Disc 2).chd", "b")};

  const auto plan = planPlaylist(discs, "hash1", "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(plan->path, "C:/appdata/playlists/hash1.m3u");
  EXPECT_EQ(linesOf(plan->contents),
            (std::vector<std::string>{"C:/roms/FF7 (Disc 1).chd", "C:/roms/FF7 (Disc 2).chd"}));
}

// A core that manages its own memory card names it after the file it was handed, so the name
// has to survive a retitle and a rebuilt library
TEST(DiscSetPlaylistTest, ThePathIsAPureFunctionOfTheIdentity) {
  EXPECT_EQ(playlistPathFor("abc123", "C:/appdata"), "C:/appdata/playlists/abc123.m3u");

  const std::vector<ContentFile> discs{disc(1, "C:/roms/a.chd", "a"), disc(2, "C:/roms/b.chd", "b")};

  EXPECT_EQ(planPlaylist(discs, "abc123", "C:/appdata")->path, playlistPathFor("abc123", "C:/appdata"));
}

// The identity is read from the first line, so an order that followed the scan would move it
TEST(DiscSetPlaylistTest, OrderIsByDiscNumberNotByTheOrderGiven) {
  const std::vector<ContentFile> discs{disc(3, "C:/roms/c.chd", "c"), disc(1, "C:/roms/a.chd", "a"),
                                       disc(2, "C:/roms/b.chd", "b")};

  const auto plan = planPlaylist(discs, "hash1", "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(linesOf(plan->contents), (std::vector<std::string>{"C:/roms/a.chd", "C:/roms/b.chd", "C:/roms/c.chd"}));
}

// One disc is not a set, and neither is a set nothing can identify
TEST(DiscSetPlaylistTest, NothingToWriteForOneDiscOrNoIdentity) {
  const std::vector<ContentFile> one{disc(1, "C:/roms/a.chd", "a")};
  EXPECT_FALSE(planPlaylist(one, "hash1", "C:/appdata").has_value());

  const std::vector<ContentFile> two{disc(1, "C:/roms/a.chd", "a"), disc(2, "C:/roms/b.chd", "b")};
  EXPECT_FALSE(planPlaylist(two, "", "C:/appdata").has_value());
}

// A path inside an archive is not a path the core can open
TEST(DiscSetPlaylistTest, DiscsInsideAnArchiveHaveNoPlaylist) {
  auto archived = disc(2, "b.chd", "b");
  archived.m_inArchive = true;
  archived.m_archivePathName = "C:/roms/game.zip";

  const std::vector<ContentFile> discs{disc(1, "C:/roms/a.chd", "a"), archived};

  EXPECT_FALSE(planPlaylist(discs, "hash1", "C:/appdata").has_value());
}

// One disc kept as both a cue and a chd is two files of identical bytes, so listing both would
// hand the core the same disc under two indices
TEST(DiscSetPlaylistTest, ADiscDumpedTwiceIsOneLine) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/a.cue", "hashA"), disc(1, "C:/roms/a.chd", "hashA"),
                                       disc(2, "C:/roms/b.cue", "hashB")};

  const auto plan = planPlaylist(discs, "hashA", "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(linesOf(plan->contents), (std::vector<std::string>{"C:/roms/a.chd", "C:/roms/b.cue"}));
}

// Two containers of one disc are still one disc, and a set of one is not a set
TEST(DiscSetPlaylistTest, OneDiscInTwoContainersIsNotAPlaylist) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/a.cue", "hashA"), disc(1, "C:/roms/a.chd", "hashA")};

  EXPECT_FALSE(planPlaylist(discs, "hashA", "C:/appdata").has_value());
}

// A path inside an archive is not one the core can open, so the loose copy is the one to name
TEST(DiscSetPlaylistTest, TheOpenableCopyOfADiscWins) {
  auto archived = disc(1, "a.cue", "hashA");
  archived.m_inArchive = true;
  archived.m_archivePathName = "C:/roms/a.zip";

  const std::vector<ContentFile> discs{archived, disc(1, "C:/roms/a.cue", "hashA"), disc(2, "C:/roms/b.cue", "hashB")};

  const auto plan = planPlaylist(discs, "hashA", "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(linesOf(plan->contents), (std::vector<std::string>{"C:/roms/a.cue", "C:/roms/b.cue"}));
}

// A file nothing has hashed is not the same dump as every other unhashed file
TEST(DiscSetPlaylistTest, UnhashedDiscsAreNotCollapsedTogether) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/a.chd"), disc(2, "C:/roms/b.chd"), disc(3, "C:/roms/c.chd")};

  const auto plan = planPlaylist(discs, "hash1", "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(linesOf(plan->contents), (std::vector<std::string>{"C:/roms/a.chd", "C:/roms/b.chd", "C:/roms/c.chd"}));
}

// Whether a core skips comment lines is not something we know, and the directory already says
// the file is ours
TEST(DiscSetPlaylistTest, GeneratedPlaylistsCarryNoCommentLine) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/a.chd", "a"), disc(2, "C:/roms/b.chd", "b")};

  const auto plan = planPlaylist(discs, "hash1", "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_FALSE(plan->contents.starts_with("#"));
}

// Playlists written by older versions carry the marker, which is what tells one of ours from
// one somebody made themselves
TEST(DiscSetPlaylistTest, AnOlderGeneratedPlaylistIsStillRecognisable) {
  EXPECT_TRUE(isGeneratedPlaylist(std::string(PLAYLIST_MARKER) + "\nGame (Disc 1).cue\n"));
  EXPECT_FALSE(isGeneratedPlaylist("Game (Disc 1).cue\nGame (Disc 2).cue\n"));
  EXPECT_FALSE(isGeneratedPlaylist(""));
}

} // namespace firelight::library
