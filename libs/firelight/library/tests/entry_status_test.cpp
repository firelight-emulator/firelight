#include <firelight/library/entry_status.hpp>

#include <gtest/gtest.h>

// The verdict a library row carries. Pure over gathered facts, so every case here is one
// struct in and one list out
namespace firelight::library {

namespace {
bool has(const EntryStatus &status, const EntryProblem problem) {
  return std::ranges::find(status.problems, problem) != status.problems.end();
}
} // namespace

TEST(EntryStatusTest, NothingWrongIsPlayableAndSaysNothing) {
  const auto status = evaluateEntryStatus({});

  EXPECT_TRUE(status.problems.empty());
  EXPECT_TRUE(status.isPlayable());
}

TEST(EntryStatusTest, AMissingFileStopsItPlaying) {
  const auto status = evaluateEntryStatus({.hasWayIn = false});

  EXPECT_TRUE(has(status, EntryProblem::FilesMissing));
  EXPECT_FALSE(status.isPlayable());
}

TEST(EntryStatusTest, APlatformNothingRunsIsReportedInsteadOfAMissingCore) {
  const auto status = evaluateEntryStatus({.isCoreRegistered = false, .isCoreInstalled = false});

  EXPECT_TRUE(has(status, EntryProblem::PlatformNotSupported));
  // Saying the core is not installed as well would be telling somebody to install a core that
  // does not exist
  EXPECT_FALSE(has(status, EntryProblem::CoreNotInstalled));
  EXPECT_FALSE(status.isPlayable());
}

TEST(EntryStatusTest, ARegisteredCoreThatIsNotOnDiskIsItsOwnProblem) {
  const auto status = evaluateEntryStatus({.isCoreInstalled = false});

  EXPECT_TRUE(has(status, EntryProblem::CoreNotInstalled));
  EXPECT_FALSE(status.isPlayable());
}

TEST(EntryStatusTest, AnInstalledCoreWithNoBiosStopsItPlaying) {
  const auto status = evaluateEntryStatus({.hasRequiredBios = false});

  EXPECT_TRUE(has(status, EntryProblem::BiosMissing));
  EXPECT_FALSE(status.isPlayable());
}

// Naming the file a core boots from before that core exists sends somebody hunting for
// something they cannot use yet
TEST(EntryStatusTest, AMissingBiosWaitsForTheCoreToArrive) {
  const auto withoutCore = evaluateEntryStatus({.isCoreInstalled = false, .hasRequiredBios = false});

  EXPECT_TRUE(has(withoutCore, EntryProblem::CoreNotInstalled));
  EXPECT_FALSE(has(withoutCore, EntryProblem::BiosMissing));

  const auto withoutPlatform = evaluateEntryStatus({.isCoreRegistered = false, .hasRequiredBios = false});

  EXPECT_TRUE(has(withoutPlatform, EntryProblem::PlatformNotSupported));
  EXPECT_FALSE(has(withoutPlatform, EntryProblem::BiosMissing));
}

// A disc inside an archive hands the core a path that only exists within the archive
TEST(EntryStatusTest, ADiscInsideAnArchiveStopsItPlaying) {
  const auto status = evaluateEntryStatus({.isDiscInArchive = true});

  EXPECT_TRUE(has(status, EntryProblem::ContentInArchive));
  EXPECT_FALSE(status.isPlayable());
}

// A library scanned before any core is installed would otherwise report the same thing on
// every row and hide every content problem behind it
TEST(EntryStatusTest, EveryProblemIsReported) {
  const auto status = evaluateEntryStatus({
      .hasWayIn = false,
      .isCoreInstalled = false,
      .isDiscInArchive = true,
      .presentDiscCount = 1,
      .expectedDiscCount = 3,
  });

  EXPECT_EQ(status.problems.size(), 4u);
  EXPECT_TRUE(has(status, EntryProblem::CoreNotInstalled));
  EXPECT_TRUE(has(status, EntryProblem::FilesMissing));
  EXPECT_TRUE(has(status, EntryProblem::ContentInArchive));
  EXPECT_TRUE(has(status, EntryProblem::DiscsMissing));
}

// What somebody can do about it, not how broken it is: a game on a platform nothing runs must
// not be reported as a missing file, which would send them off to re-copy it
TEST(EntryStatusTest, ProblemsComeInTheOrderTheyCanBeActedOn) {
  const auto status = evaluateEntryStatus({
      .hasWayIn = false,
      .isCoreRegistered = false,
      .isDiscInArchive = true,
  });

  ASSERT_EQ(status.problems.size(), 3u);
  EXPECT_EQ(status.problems[0], EntryProblem::PlatformNotSupported);
  EXPECT_EQ(status.problems[1], EntryProblem::FilesMissing);
  EXPECT_EQ(status.problems[2], EntryProblem::ContentInArchive);
}

// Finding a BIOS is one action, the same class of thing as installing a core, so it belongs
// above everything the filesystem has to fix
TEST(EntryStatusTest, AMissingBiosOutranksTheContentProblems) {
  const auto status = evaluateEntryStatus({
      .hasWayIn = false,
      .hasRequiredBios = false,
      .isDiscInArchive = true,
  });

  ASSERT_EQ(status.problems.size(), 3u);
  EXPECT_EQ(status.problems[0], EntryProblem::BiosMissing);
  EXPECT_EQ(status.problems[1], EntryProblem::FilesMissing);
  EXPECT_EQ(status.problems[2], EntryProblem::ContentInArchive);
}

// A folder that cannot be read says nothing about whether the files under it are gone. Reporting
// both would tell somebody their games are lost when the drive is merely unplugged
TEST(EntryStatusTest, AnUnreachableFolderIsReportedInsteadOfMissingFiles) {
  const auto status = evaluateEntryStatus({.hasWayIn = false, .isContentReachable = false});

  EXPECT_TRUE(has(status, EntryProblem::ContentUnavailable));
  EXPECT_FALSE(has(status, EntryProblem::FilesMissing));
  EXPECT_FALSE(status.isPlayable());
}

// Reconnecting a drive is one action, so it ranks with installing a core rather than with what
// the filesystem has to fix
TEST(EntryStatusTest, AnUnreachableFolderOutranksTheContentProblems) {
  const auto status = evaluateEntryStatus({
      .isContentReachable = false,
      .isDiscInArchive = true,
  });

  ASSERT_EQ(status.problems.size(), 2u);
  EXPECT_EQ(status.problems[0], EntryProblem::ContentUnavailable);
  EXPECT_EQ(status.problems[1], EntryProblem::ContentInArchive);
}

// The folder being unreadable is not the core's fault, so both are still worth saying
TEST(EntryStatusTest, AMissingBiosStillOutranksAnUnreachableFolder) {
  const auto status = evaluateEntryStatus({.hasRequiredBios = false, .isContentReachable = false});

  ASSERT_EQ(status.problems.size(), 2u);
  EXPECT_EQ(status.problems[0], EntryProblem::BiosMissing);
  EXPECT_EQ(status.problems[1], EntryProblem::ContentUnavailable);
}

// A set with a disc missing still plays, starting from the lowest disc present
TEST(EntryStatusTest, AMissingDiscIsWorthSayingButDoesNotStopItPlaying) {
  const auto status = evaluateEntryStatus({.presentDiscCount = 2, .expectedDiscCount = 3});

  EXPECT_TRUE(has(status, EntryProblem::DiscsMissing));
  EXPECT_TRUE(status.isPlayable());
}

// Nothing knows how many discs most games came on, and guessing from what somebody owns is
// how you tell a two-disc game it is missing a third
TEST(EntryStatusTest, AnUnknownDiscCountSaysNothing) {
  EXPECT_TRUE(evaluateEntryStatus({.presentDiscCount = 1, .expectedDiscCount = 0}).problems.empty());
  EXPECT_TRUE(evaluateEntryStatus({.presentDiscCount = 3, .expectedDiscCount = 3}).problems.empty());
}

// Owning disc 2 and not disc 1 is a complete two-disc set as far as this is concerned. The set
// launches from the lowest disc present, so only the count decides
TEST(EntryStatusTest, AbsentFirstDiscIsNotAProblemByItself) {
  const auto status = evaluateEntryStatus({.presentDiscCount = 2, .expectedDiscCount = 2});

  EXPECT_TRUE(status.problems.empty());
  EXPECT_TRUE(status.isPlayable());
}

TEST(EntryStatusTest, OnlyTheDiscCountLeavesAGamePlayable) {
  EXPECT_FALSE(blocksLaunch(EntryProblem::DiscsMissing));
  EXPECT_TRUE(blocksLaunch(EntryProblem::PlatformNotSupported));
  EXPECT_TRUE(blocksLaunch(EntryProblem::CoreNotInstalled));
  EXPECT_TRUE(blocksLaunch(EntryProblem::BiosMissing));
  EXPECT_TRUE(blocksLaunch(EntryProblem::ContentUnavailable));
  EXPECT_TRUE(blocksLaunch(EntryProblem::FilesMissing));
  EXPECT_TRUE(blocksLaunch(EntryProblem::ContentInArchive));
}

} // namespace firelight::library
