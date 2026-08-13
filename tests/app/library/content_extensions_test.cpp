#include <firelight/library/content_extensions.hpp>

#include <gtest/gtest.h>

namespace firelight::library {

// Iterating the arrays rather than re-typing them: a hand-written second copy is what drifted
// from the real list and let a format quietly stop being scanned
TEST(ContentExtensionsTest, RecognizesEveryDiscExtension) {
  ASSERT_FALSE(DISC_EXTENSIONS.empty());

  for (const auto *ext : DISC_EXTENSIONS) {
    EXPECT_TRUE(isDiscExtension(ext)) << ext;
  }
}

TEST(ContentExtensionsTest, RejectsNonDiscExtensions) {
  for (const auto *ext : {"gba", "nes", "sfc", "n64", "zip", "txt", "", "iso2"}) {
    EXPECT_FALSE(isDiscExtension(ext)) << ext;
  }
}

// Removed because nothing can read them: a .ccd is a text descriptor we never parse, and a .nrg
// carries two layouts plus a footer. Accepting an extension nothing identifies is how a file
// gets taken in and then dropped
TEST(ContentExtensionsTest, FormatsNothingCanReadAreNotAccepted) {
  EXPECT_FALSE(isDiscExtension("ccd"));
  EXPECT_FALSE(isDiscExtension("nrg"));
  EXPECT_FALSE(isDiscSheetExtension("ccd"));
  EXPECT_FALSE(isDiscTrackExtension("nrg"));
}

// Raw track images are reached through the sheet naming them; the self-contained formats are
// not tracks and must never be skipped for one
TEST(ContentExtensionsTest, TrackExtensionsAreRawImagesOnly) {
  ASSERT_FALSE(DISC_TRACK_EXTENSIONS.empty());

  for (const auto *ext : DISC_TRACK_EXTENSIONS) {
    EXPECT_TRUE(isDiscTrackExtension(ext)) << ext;
  }
  for (const auto *ext : {"iso", "chd", "pbp", "cso", "cue", "gdi", "m3u"}) {
    EXPECT_FALSE(isDiscTrackExtension(ext)) << ext;
  }
}

TEST(ContentExtensionsTest, SheetExtensionsReferenceTracks) {
  ASSERT_FALSE(DISC_SHEET_EXTENSIONS.empty());

  for (const auto *ext : DISC_SHEET_EXTENSIONS) {
    EXPECT_TRUE(isDiscSheetExtension(ext)) << ext;
  }
  for (const auto *ext : {"iso", "bin", "img", "chd", "pbp"}) {
    EXPECT_FALSE(isDiscSheetExtension(ext)) << ext;
  }
}

// Invariant the scanner relies on: every track and every sheet extension is itself a disc
// extension, so isDiscExtension never lets one slip past the gate
TEST(ContentExtensionsTest, TracksAndSheetsAreAlsoDiscExtensions) {
  for (const auto *ext : DISC_TRACK_EXTENSIONS) {
    EXPECT_TRUE(isDiscExtension(ext)) << ext;
  }
  for (const auto *ext : DISC_SHEET_EXTENSIONS) {
    EXPECT_TRUE(isDiscExtension(ext)) << ext;
  }
}

// Matching is exact/case-sensitive by contract: callers lowercase the extension
// before classifying (the scanner and ContentIdentifier both do)
TEST(ContentExtensionsTest, MatchingIsCaseSensitive) {
  EXPECT_FALSE(isDiscExtension("ISO"));
  EXPECT_FALSE(isDiscExtension("Cue"));
  EXPECT_FALSE(isDiscTrackExtension("BIN"));
  EXPECT_FALSE(isDiscSheetExtension("M3U"));
}

} // namespace firelight::library
