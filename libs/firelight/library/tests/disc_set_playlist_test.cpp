// TODO: NEEDS REVIEW
#include <firelight/library/disc_set_playlist.hpp>

#include <algorithm>
#include <gtest/gtest.h>
#include <string>
#include <vector>

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

// TODO
// The disc lines, without the line saying the file is ours
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

  std::erase_if(lines, [](const std::string &line) { return line.starts_with("#"); });
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

// TODO
// A set of one launches through its playlist like any other, so a single disc renders
TEST(DiscSetPlaylistTest, OneDiscStillGetsAPlaylist) {
  const std::vector<ContentFile> one{disc(1, "C:/roms/a.chd", "a")};

  const auto plan = planPlaylist(one, "hash1", "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(linesOf(plan->contents), (std::vector<std::string>{"C:/roms/a.chd"}));
}

// TODO
// What is left to refuse: nothing to name it by, and nothing to name
TEST(DiscSetPlaylistTest, NothingToWriteWithoutAnIdentityOrAnyDiscs) {
  const std::vector<ContentFile> two{disc(1, "C:/roms/a.chd", "a"), disc(2, "C:/roms/b.chd", "b")};
  EXPECT_FALSE(planPlaylist(two, "", "C:/appdata").has_value());

  EXPECT_FALSE(planPlaylist({}, "hash1", "C:/appdata").has_value());
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

// TODO
// Two containers of one disc are still one disc, so the set renders as the one line it is
TEST(DiscSetPlaylistTest, OneDiscInTwoContainersIsOneLine) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/a.cue", "hashA"), disc(1, "C:/roms/a.chd", "hashA")};

  const auto plan = planPlaylist(discs, "hashA", "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(linesOf(plan->contents), (std::vector<std::string>{"C:/roms/a.chd"}));
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

// TODO
// A playlist of ours says so on its first line, which is what makes overwriting or deleting one
// ours to do. rc_hash skips comment lines when it resolves a playlist to its first disc, so the
// marker cannot move the set's identity
TEST(DiscSetPlaylistTest, AGeneratedPlaylistSaysItIsOurs) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/a.chd", "a"), disc(2, "C:/roms/b.chd", "b")};

  const auto plan = planPlaylist(discs, "hash1", "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_TRUE(isGeneratedPlaylist(plan->contents));
  EXPECT_TRUE(plan->contents.starts_with(PLAYLIST_MARKER));

  // The discs still follow, in order, with the marker taking only its own line
  EXPECT_NE(plan->contents.find("C:/roms/a.chd"), std::string::npos);
  EXPECT_LT(plan->contents.find("C:/roms/a.chd"), plan->contents.find("C:/roms/b.chd"));
}

// TODO
// Somebody else's playlist is not ours to touch, whatever it is called or wherever it sits
TEST(DiscSetPlaylistTest, APlaylistSomebodyElseWroteIsNotOurs) {
  EXPECT_FALSE(isGeneratedPlaylist("Game (Disc 1).cue\nGame (Disc 2).cue\n"));
  EXPECT_FALSE(isGeneratedPlaylist(""));
  EXPECT_FALSE(isGeneratedPlaylist("# Generated by something else\nGame (Disc 1).cue\n"));
}

} // namespace firelight::library
