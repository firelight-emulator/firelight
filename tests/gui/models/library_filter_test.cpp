// TODO: NEEDS REVIEW
#include "gui/models/library_filter.hpp"

#include <gtest/gtest.h>

namespace firelight::gui {
namespace {

// TODO
// A criterion the editing UI has no control for still has to survive being loaded and saved. This
// is the defect that lost playedAfterMillis every time somebody opened a folder for edit
TEST(LibraryFilterTest, SetJsonThenGetJsonPreservesUnsurfacedCriteria) {
  LibraryFilter filter;
  filter.setJson(R"({"platformIds":[3],"playedAfterMillis":1700000000000})");

  filter.setDeveloper("Nintendo");

  const auto restored = library::SmartFolderCriteria::parse(filter.getJson().toStdString());

  ASSERT_TRUE(restored.playedAfterMillis.has_value()) << "a criterion with no control was dropped";
  EXPECT_EQ(*restored.playedAfterMillis, 1700000000000);
  EXPECT_EQ(restored.platformIds, (std::vector<int>{3}));
  EXPECT_EQ(restored.developer, "Nintendo");
}

// TODO
// Collapsing the list to its first element is what lost every genre but one on an edit
TEST(LibraryFilterTest, GenresSurviveAMultiValueRoundTrip) {
  LibraryFilter filter;
  filter.setJson(R"({"genres":["Action","Puzzle","Racing"]})");

  EXPECT_EQ(filter.getGenres(), (QStringList{"Action", "Puzzle", "Racing"}));

  const auto restored = library::SmartFolderCriteria::parse(filter.getJson().toStdString());
  EXPECT_EQ(restored.genres, (std::vector<std::string>{"Action", "Puzzle", "Racing"}));
}

// TODO
// Three states: wanted, not wanted, and not asked about. A two-state control can only ever say two
TEST(LibraryFilterTest, FavoriteFalseIsExpressible) {
  LibraryFilter filter;
  EXPECT_EQ(filter.getFavorite(), LibraryFilter::Unset);
  EXPECT_EQ(filter.getJson(), QStringLiteral("{}")) << "an untouched filter pinned something";

  filter.setFavorite(LibraryFilter::No);

  const auto restored = library::SmartFolderCriteria::parse(filter.getJson().toStdString());
  ASSERT_TRUE(restored.favorite.has_value());
  EXPECT_FALSE(*restored.favorite);

  filter.setFavorite(LibraryFilter::Unset);
  EXPECT_EQ(filter.getJson(), QStringLiteral("{}"));
}

// TODO
// The unit conversion lives here rather than in a QML expression, so a second consumer of the same
// JSON cannot read minutes as seconds
TEST(LibraryFilterTest, MinMinutesConvertsToSeconds) {
  LibraryFilter filter;
  EXPECT_EQ(filter.getMinMinutesPlayed(), LibraryFilter::NO_MINUTES);

  filter.setMinMinutesPlayed(90);

  const auto restored = library::SmartFolderCriteria::parse(filter.getJson().toStdString());
  ASSERT_TRUE(restored.minSecondsPlayed.has_value());
  EXPECT_EQ(*restored.minSecondsPlayed, 5400);
  EXPECT_EQ(filter.getMinMinutesPlayed(), 90);
}

// TODO
// A corrupt folder shows the whole library rather than nothing, which is the recoverable failure
TEST(LibraryFilterTest, MalformedJsonLeavesAMatchAllFilter) {
  LibraryFilter filter;
  filter.setPlatformIds({3});
  ASSERT_FALSE(filter.isEmpty());

  filter.setJson(QStringLiteral("not json{{{"));

  EXPECT_TRUE(filter.isEmpty());
  EXPECT_EQ(filter.getJson(), QStringLiteral("{}"));
}

// TODO
// Editing anything has to reach a listener that only cares that something moved
TEST(LibraryFilterTest, EveryEditEmitsTheAggregateSignal) {
  LibraryFilter filter;
  auto changes = 0;
  QObject::connect(&filter, &LibraryFilter::changed, [&changes] { ++changes; });

  filter.setNameContains("zelda");
  filter.setYearMin(1990);
  filter.setPlayable(LibraryFilter::Yes);
  EXPECT_EQ(changes, 3);

  filter.setNameContains("zelda");
  EXPECT_EQ(changes, 3) << "setting a criterion to what it already was counted as a change";
}

// TODO
// Saving the current view as a folder is a copy of the object the view is already holding
TEST(LibraryFilterTest, CopyFromTakesTheOtherFiltersCriteria) {
  LibraryFilter view;
  view.setNameContains("metroid");
  view.setPlatformIds({3, 7});

  LibraryFilter folder;
  folder.copyFrom(&view);

  EXPECT_EQ(folder.getJson(), view.getJson());
  EXPECT_NO_FATAL_FAILURE(folder.copyFrom(nullptr));
}

} // namespace
} // namespace firelight::gui
