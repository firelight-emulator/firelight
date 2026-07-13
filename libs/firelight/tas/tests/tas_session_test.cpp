#include <firelight/tas/tas_session.hpp>

#include <firelight/input/input_frame.hpp>
#include <firelight/libretro/retropad.hpp>

#include <gtest/gtest.h>

#include <cstring>
#include <vector>

namespace firelight::tas {
namespace {

using input::InputFrame;

// A deterministic ITasEmulator whose serialized state is (frameCounter, inputHash),
// where inputHash folds in the buttons of every frame consumed by runFrame(). That
// makes state a function of the exact input sequence emulated, so a greenzone seek
// (restore keyframe + replay) is only correct if it re-feeds the identical frames —
// exactly what these tests assert.
class FakeTasEmulator final : public ITasEmulator {
public:
  libretro::IRetropadProvider *provider = nullptr;
  uint64_t counter = 0;
  uint64_t inputHash = 1469598103934665603ull; // FNV-1a offset basis

  void setRetropadProvider(libretro::IRetropadProvider *p) override {
    provider = p;
  }
  void runFrame() override {
    InputFrame f{};
    if (provider) {
      if (const auto pad = provider->getRetropadForPlayerIndex(0)) {
        f = input::captureJoypadFrame(*pad, /*platformId=*/0, /*ctrlType=*/1);
      }
    }
    inputHash ^= f.buttons;
    inputHash *= 1099511628211ull; // FNV-1a prime
    ++counter;
  }
  [[nodiscard]] std::vector<uint8_t> serializeState() override {
    std::vector<uint8_t> s(16);
    std::memcpy(s.data(), &counter, 8);
    std::memcpy(s.data() + 8, &inputHash, 8);
    return s;
  }
  bool deserializeState(const std::vector<uint8_t> &s) override {
    if (s.size() != 16) {
      return false;
    }
    std::memcpy(&counter, s.data(), 8);
    std::memcpy(&inputHash, s.data() + 8, 8);
    return true;
  }
};

std::vector<InputFrame> makeMovie(std::size_t n) {
  std::vector<InputFrame> m;
  m.reserve(n);
  for (std::size_t k = 0; k < n; ++k) {
    InputFrame f;
    f.setButton(static_cast<unsigned>(k % 12), true);
    f.setButton(k % 2 ? libretro::IRetroPad::DpadRight
                      : libretro::IRetroPad::DpadLeft,
                true);
    m.push_back(f);
  }
  return m;
}

// Plays a fresh session straight to `frame` and returns its state, for use as an
// independent reference against seek/edit results.
std::vector<uint8_t> referenceStateAt(const std::vector<InputFrame> &movie,
                                      uint64_t frame) {
  FakeTasEmulator emu;
  TasSession s(emu, {/*stride=*/4, /*maxKeyframes=*/0});
  s.loadMovie(movie);
  for (uint64_t i = 0; i < frame; ++i) {
    s.stepForward();
  }
  return emu.serializeState();
}

} // namespace

TEST(TasSessionTest, PlaysForwardFrameByFrame) {
  FakeTasEmulator emu;
  TasSession s(emu, {/*stride=*/4, /*maxKeyframes=*/0});
  const auto movie = makeMovie(20);
  s.loadMovie(movie);
  EXPECT_EQ(s.currentFrame(), 0u);

  for (int i = 0; i < 20; ++i) {
    s.stepForward();
  }
  EXPECT_EQ(s.currentFrame(), 20u);
  EXPECT_EQ(emu.counter, 20u);
}

TEST(TasSessionTest, CapturesGreenzoneOnStride) {
  FakeTasEmulator emu;
  TasSession s(emu, {/*stride=*/4, /*maxKeyframes=*/0});
  s.loadMovie(makeMovie(20));
  for (int i = 0; i < 20; ++i) {
    s.stepForward();
  }
  // Anchor + every 4th frame: 0,4,8,12,16,20.
  const auto idx = s.greenzone().keyframeIndices();
  const std::vector<uint64_t> expected{0, 4, 8, 12, 16, 20};
  EXPECT_EQ(idx, expected);
}

TEST(TasSessionTest, SeekBackwardThenForwardReproducesState) {
  FakeTasEmulator emu;
  TasSession s(emu, {/*stride=*/4, /*maxKeyframes=*/0});
  const auto movie = makeMovie(20);
  s.loadMovie(movie);
  for (int i = 0; i < 20; ++i) {
    s.stepForward();
  }
  const auto state20 = emu.serializeState();

  s.seekTo(5);
  EXPECT_EQ(s.currentFrame(), 5u);
  EXPECT_EQ(emu.serializeState(), referenceStateAt(movie, 5));

  s.seekTo(20);
  EXPECT_EQ(s.currentFrame(), 20u);
  EXPECT_EQ(emu.serializeState(), state20); // deterministic round-trip
}

TEST(TasSessionTest, SeekToArbitraryFrameMatchesStraightPlay) {
  FakeTasEmulator emu;
  TasSession s(emu, {/*stride=*/4, /*maxKeyframes=*/0});
  const auto movie = makeMovie(30);
  s.loadMovie(movie);
  for (int i = 0; i < 30; ++i) {
    s.stepForward();
  }
  // A backward seek that lands between keyframes (restore 12, replay 1).
  s.seekTo(13);
  EXPECT_EQ(s.currentFrame(), 13u);
  EXPECT_EQ(emu.serializeState(), referenceStateAt(movie, 13));

  // A forward seek that can continue from the current position (no restore).
  s.seekTo(19);
  EXPECT_EQ(s.currentFrame(), 19u);
  EXPECT_EQ(emu.serializeState(), referenceStateAt(movie, 19));
}

TEST(TasSessionTest, EditInvalidatesGreenzoneAndRepositions) {
  FakeTasEmulator emu;
  TasSession s(emu, {/*stride=*/4, /*maxKeyframes=*/0});
  const auto movie = makeMovie(20);
  s.loadMovie(movie);
  for (int i = 0; i < 20; ++i) {
    s.stepForward();
  }
  ASSERT_EQ(s.currentFrame(), 20u);

  InputFrame edited; // distinct from movie[6]
  edited.setButton(libretro::IRetroPad::Start, true);

  const auto rr = s.rerecordCount();
  s.editFrame(6, edited);

  EXPECT_EQ(s.rerecordCount(), rr + 1);
  EXPECT_EQ(s.currentFrame(), 6u); // repositioned to the edited frame
  EXPECT_EQ(s.movie()[6], edited);

  // The greenzone tail after the edit is gone.
  for (const auto k : s.greenzone().keyframeIndices()) {
    EXPECT_LE(k, 6u);
  }

  // State at frame 6 is unchanged: editing frame 6's input doesn't alter the state
  // reached BEFORE that input is consumed.
  EXPECT_EQ(emu.serializeState(), referenceStateAt(movie, 6));

  // Stepping forward now consumes the edited input, diverging from the original.
  s.stepForward();
  EXPECT_NE(emu.serializeState(), referenceStateAt(movie, 7));
}

TEST(TasSessionTest, EditBeforeCurrentPositionDoesNotReposition) {
  FakeTasEmulator emu;
  TasSession s(emu, {/*stride=*/4, /*maxKeyframes=*/0});
  const auto movie = makeMovie(20);
  s.loadMovie(movie);
  for (int i = 0; i < 5; ++i) {
    s.stepForward();
  }
  ASSERT_EQ(s.currentFrame(), 5u);
  InputFrame edited;
  edited.setButton(libretro::IRetroPad::Start, true);
  // Editing a FUTURE frame (>= current) leaves the position untouched.
  s.editFrame(10, edited);
  EXPECT_EQ(s.currentFrame(), 5u);
  EXPECT_EQ(s.rerecordCount(), 1u);
}

} // namespace firelight::tas
