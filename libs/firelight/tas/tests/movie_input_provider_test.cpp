#include <firelight/tas/movie_input_provider.hpp>

#include <firelight/input/input_frame.hpp>

#include <gtest/gtest.h>

#include <vector>

namespace firelight::tas {

using input::captureJoypadFrame;
using input::InputFrame;

namespace {
// A movie where frame k presses button id (k % 12), so each frame is distinct.
std::vector<InputFrame> makeMovie(std::size_t n) {
  std::vector<InputFrame> m;
  m.reserve(n);
  for (std::size_t k = 0; k < n; ++k) {
    InputFrame f;
    f.setButton(static_cast<unsigned>(k % 12), true);
    m.push_back(f);
  }
  return m;
}
} // namespace

TEST(MovieInputProviderTest, CursorStartsAtZeroAndPresentsFrameZero) {
  auto movie = makeMovie(5);
  MovieInputProvider p(movie);
  EXPECT_EQ(p.frameCount(), 5u);
  EXPECT_EQ(p.cursor(), 0u);
  EXPECT_EQ(p.currentFrame(), movie[0]);
}

// The provider must present the current frame identically no matter how many
// times a port is polled within one frame (the CoreInputRouter polls all ports
// each frame) — getRetropadForPlayerIndex() must NOT self-advance.
TEST(MovieInputProviderTest, PollingDoesNotAdvanceCursor) {
  auto movie = makeMovie(3);
  MovieInputProvider p(movie);
  for (int i = 0; i < 8; ++i) {
    auto pad = p.getRetropadForPlayerIndex(0);
    ASSERT_NE(pad, nullptr);
    EXPECT_EQ(captureJoypadFrame(*pad, 7, 1), movie[0]);
  }
  EXPECT_EQ(p.cursor(), 0u);
}

TEST(MovieInputProviderTest, OnlyDrivesItsPort) {
  MovieInputProvider p(makeMovie(2));
  EXPECT_NE(p.getRetropadForPlayerIndex(0), nullptr);
  EXPECT_EQ(p.getRetropadForPlayerIndex(1), nullptr);
  p.setPort(2);
  EXPECT_EQ(p.getRetropadForPlayerIndex(0), nullptr);
  EXPECT_NE(p.getRetropadForPlayerIndex(2), nullptr);
}

// Simulate the per-frame drive loop: sample the port (as the router would), then
// advance exactly once. The sampled sequence must equal the movie.
TEST(MovieInputProviderTest, DriveLoopReplaysMovieInOrder) {
  auto movie = makeMovie(10);
  MovieInputProvider p(movie);
  std::vector<InputFrame> seen;
  for (std::size_t i = 0; i < movie.size(); ++i) {
    auto pad = p.getRetropadForPlayerIndex(0);
    seen.push_back(captureJoypadFrame(*pad, 7, 1));
    p.advance();
  }
  EXPECT_EQ(seen, movie);
  EXPECT_TRUE(p.atEnd());
}

// Past-the-end reads a neutral frame — never stale input.
TEST(MovieInputProviderTest, OverrunPresentsNeutralFrame) {
  MovieInputProvider p(makeMovie(2));
  p.seekTo(5);
  EXPECT_TRUE(p.atEnd());
  auto pad = p.getRetropadForPlayerIndex(0);
  EXPECT_EQ(captureJoypadFrame(*pad, 7, 1), InputFrame{});
}

TEST(MovieInputProviderTest, SeekToJumpsCursor) {
  auto movie = makeMovie(20);
  MovieInputProvider p(movie);
  p.seekTo(13);
  EXPECT_EQ(p.cursor(), 13u);
  auto pad = p.getRetropadForPlayerIndex(0);
  EXPECT_EQ(captureJoypadFrame(*pad, 7, 1), movie[13]);
}

// The record decorator forwards to the live provider AND logs one frame per
// captureFrame(); the log must replay identically to what was played.
TEST(MovieInputProviderTest, RecordingCapturesPlayedInput) {
  auto movie = makeMovie(6);
  MovieInputProvider live(movie); // stands in for the real InputService here
  RecordingInputProvider rec(&live, /*platformId=*/7, /*controllerTypeId=*/1);

  for (std::size_t i = 0; i < movie.size(); ++i) {
    // The core would poll rec (forwarded to live); the session then snapshots.
    auto pad = rec.getRetropadForPlayerIndex(0);
    ASSERT_NE(pad, nullptr);
    rec.captureFrame();
    live.advance();
  }
  EXPECT_EQ(rec.log(), movie);
}

} // namespace firelight::tas
