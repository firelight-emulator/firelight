#include <firelight/library/disc_set_playlist.hpp>

#include <gtest/gtest.h>

// The playlist is what gives a set one identity and one memory card, and its first line is
// what that identity is read from. These pin the ordering and the path style
namespace firelight::library {

namespace {
ContentFile disc(const int number, const std::string &path) {
  ContentFile file;
  file.m_type = ContentType::Disc;
  file.m_discNumber = number;
  file.m_filePath = path;
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

TEST(DiscSetPlaylistTest, WritesRelativeLinesBesideTheDiscs) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/FF7 (Disc 1).chd"), disc(2, "C:/roms/FF7 (Disc 2).chd")};

  const auto plan = planPlaylist(discs, 1, "Final Fantasy VII", PlaylistLocation::BesideDiscs, "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(plan->path, "C:/roms/Final Fantasy VII.m3u");
  EXPECT_TRUE(plan->isPortable);
  EXPECT_EQ(linesOf(plan->contents),
            (std::vector<std::string>{std::string(PLAYLIST_MARKER), "FF7 (Disc 1).chd", "FF7 (Disc 2).chd"}));
}

// The identity is read from the first line, so an order that followed the scan would move it
TEST(DiscSetPlaylistTest, OrderIsByDiscNumberNotByTheOrderGiven) {
  const std::vector<ContentFile> discs{disc(3, "C:/roms/c.chd"), disc(1, "C:/roms/a.chd"), disc(2, "C:/roms/b.chd")};

  const auto plan = planPlaylist(discs, 1, "Game", PlaylistLocation::BesideDiscs, "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(linesOf(plan->contents),
            (std::vector<std::string>{std::string(PLAYLIST_MARKER), "a.chd", "b.chd", "c.chd"}));
}

// Discs split across folders have no shared parent, so relative lines cannot address them all
TEST(DiscSetPlaylistTest, SplitDiscsFallBackToAbsoluteAndAreNotPortable) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/a/FF7 (Disc 1).chd"), disc(2, "D:/elsewhere/FF7 (Disc 2).chd")};

  const auto plan = planPlaylist(discs, 1, "Final Fantasy VII", PlaylistLocation::BesideDiscs, "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  // Beside disc 1, which is the only folder that is any better than the others
  EXPECT_EQ(plan->path, "C:/roms/a/Final Fantasy VII.m3u");
  EXPECT_FALSE(plan->isPortable);
  EXPECT_EQ(linesOf(plan->contents),
            (std::vector<std::string>{std::string(PLAYLIST_MARKER), "C:/roms/a/FF7 (Disc 1).chd",
                                      "D:/elsewhere/FF7 (Disc 2).chd"}));
}

TEST(DiscSetPlaylistTest, AppDataPlaylistsNeverTouchTheGamesFolder) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/FF7 (Disc 1).chd"), disc(2, "C:/roms/FF7 (Disc 2).chd")};

  const auto plan = planPlaylist(discs, 7, "Final Fantasy VII", PlaylistLocation::AppData, "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(plan->path, "C:/appdata/playlists/7 - Final Fantasy VII.m3u");
  // Nothing beside the discs can be addressed relatively from there
  EXPECT_FALSE(plan->isPortable);
  EXPECT_EQ(linesOf(plan->contents), (std::vector<std::string>{std::string(PLAYLIST_MARKER), "C:/roms/FF7 (Disc 1).chd",
                                                               "C:/roms/FF7 (Disc 2).chd"}));
}

TEST(DiscSetPlaylistTest, OneDiscGetsNoPlaylist) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/FF7 (Disc 1).chd")};

  EXPECT_FALSE(planPlaylist(discs, 1, "Final Fantasy VII", PlaylistLocation::BesideDiscs, "C:/appdata").has_value());
  EXPECT_FALSE(planPlaylist({}, 1, "Final Fantasy VII", PlaylistLocation::BesideDiscs, "C:/appdata").has_value());
}

// A path inside an archive is not a path a core can open, so there is no playlist to write
TEST(DiscSetPlaylistTest, ArchivedDiscsGetNoPlaylist) {
  auto archived = disc(2, "FF7 (Disc 2).chd");
  archived.m_inArchive = true;
  archived.m_archivePathName = "C:/roms/FF7.zip";

  const std::vector<ContentFile> discs{disc(1, "C:/roms/FF7 (Disc 1).chd"), archived};

  EXPECT_FALSE(planPlaylist(discs, 1, "Final Fantasy VII", PlaylistLocation::BesideDiscs, "C:/appdata").has_value());
}

// A title becomes a filename, and titles carry characters filenames cannot
TEST(DiscSetPlaylistTest, TitlesAreMadeSafeToUseAsAFilename) {
  EXPECT_EQ(sanitizeFileName("Ratchet & Clank: Up Your Arsenal"), "Ratchet & Clank Up Your Arsenal");
  EXPECT_EQ(sanitizeFileName("Where/Are\\We?"), "WhereAreWe");
  EXPECT_EQ(sanitizeFileName("Trailing dots..."), "Trailing dots");
  EXPECT_EQ(sanitizeFileName("   "), "");

  const std::vector<ContentFile> discs{disc(1, "C:/roms/a.chd"), disc(2, "C:/roms/b.chd")};
  const auto plan = planPlaylist(discs, 1, "A:B?C", PlaylistLocation::BesideDiscs, "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(plan->path, "C:/roms/ABC.m3u");
}

// A set with no usable title still needs a playlist, so it borrows disc 1's filename
TEST(DiscSetPlaylistTest, AnUntitledSetIsNamedAfterItsFirstDisc) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/FF7 (Disc 1).chd"), disc(2, "C:/roms/FF7 (Disc 2).chd")};

  const auto plan = planPlaylist(discs, 1, "", PlaylistLocation::BesideDiscs, "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(plan->path, "C:/roms/FF7 (Disc 1).m3u");
}

// One folder per disc is a normal way to keep a multi-disc game, and burying the playlist
// inside the first disc's folder is not where anybody would look for it
TEST(DiscSetPlaylistTest, PerDiscFoldersPutThePlaylistBesideThoseFolders) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/FF7 (Disc 1)/FF7 (Disc 1).cue"),
                                       disc(2, "C:/roms/FF7 (Disc 2)/FF7 (Disc 2).cue"),
                                       disc(3, "C:/roms/FF7 (Disc 3)/FF7 (Disc 3).cue")};

  const auto plan = planPlaylist(discs, 1, "Final Fantasy VII", PlaylistLocation::BesideDiscs, "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(plan->path, "C:/roms/Final Fantasy VII.m3u");
  // Relative lines reach into the per-disc folders, so the set survives being moved
  EXPECT_TRUE(plan->isPortable);
  EXPECT_EQ(linesOf(plan->contents),
            (std::vector<std::string>{std::string(PLAYLIST_MARKER), "FF7 (Disc 1)/FF7 (Disc 1).cue",
                                      "FF7 (Disc 2)/FF7 (Disc 2).cue", "FF7 (Disc 3)/FF7 (Disc 3).cue"}));
}

// The marker is how a playlist of ours is recognised after the library database is thrown
// away, which would otherwise leave us adopting our own orphan as somebody else's work
TEST(DiscSetPlaylistTest, AGeneratedPlaylistIsRecognisableFromItsFirstLine) {
  const std::vector<ContentFile> discs{disc(1, "C:/roms/a.chd"), disc(2, "C:/roms/b.chd")};
  const auto plan = planPlaylist(discs, 1, "Game", PlaylistLocation::BesideDiscs, "C:/appdata");

  ASSERT_TRUE(plan.has_value());
  EXPECT_TRUE(isGeneratedPlaylist(plan->contents));
  EXPECT_FALSE(isGeneratedPlaylist("Game (Disc 1).cue\nGame (Disc 2).cue\n"));
  EXPECT_FALSE(isGeneratedPlaylist(""));
}

} // namespace firelight::library
