#include <firelight/library/content_extensions.hpp>

#include <gtest/gtest.h>

namespace firelight::library {

// The scanner gates disc detection on isDiscExtension, so the full set of
// container/sheet extensions must be recognized (a dropped entry silently stops
// that format from ever being scanned as a disc)
TEST(ContentExtensionsTest, RecognizesAllDiscExtensions) {
  for (const auto *ext : {"iso", "bin", "cue", "chd", "pbp", "cso", "m3u",
                          "gdi", "ccd", "img", "mdf", "nrg"}) {
    EXPECT_TRUE(isDiscExtension(ext)) << ext;
  }
}

TEST(ContentExtensionsTest, RejectsNonDiscExtensions) {
  for (const auto *ext : {"gba", "nes", "sfc", "n64", "zip", "txt", "", "iso2"}) {
    EXPECT_FALSE(isDiscExtension(ext)) << ext;
  }
}

// Raw track/data images are pulled in via their sheet; the classifier marks the
// self-contained formats (iso/chd/pbp/cso) as NOT tracks so they aren't skipped
TEST(ContentExtensionsTest, TrackExtensionsAreRawImagesOnly) {
  for (const auto *ext : {"bin", "img", "mdf", "nrg"}) {
    EXPECT_TRUE(isDiscTrackExtension(ext)) << ext;
  }
  for (const auto *ext : {"iso", "chd", "pbp", "cso", "cue", "gdi", "ccd",
                          "m3u"}) {
    EXPECT_FALSE(isDiscTrackExtension(ext)) << ext;
  }
}

TEST(ContentExtensionsTest, SheetExtensionsReferenceTracks) {
  for (const auto *ext : {"cue", "gdi", "ccd", "m3u"}) {
    EXPECT_TRUE(isDiscSheetExtension(ext)) << ext;
  }
  for (const auto *ext : {"iso", "bin", "img", "chd", "pbp"}) {
    EXPECT_FALSE(isDiscSheetExtension(ext)) << ext;
  }
}

// Invariant the scanner relies on: every track and every sheet extension is
// itself a disc extension, so isDiscExtension never lets one slip past the gate
TEST(ContentExtensionsTest, TracksAndSheetsAreAlsoDiscExtensions) {
  for (const auto *ext : {"bin", "img", "mdf", "nrg", "cue", "gdi", "ccd",
                          "m3u"}) {
    EXPECT_TRUE(isDiscExtension(ext)) << ext;
    EXPECT_TRUE(isDiscTrackExtension(ext) || isDiscSheetExtension(ext)) << ext;
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
