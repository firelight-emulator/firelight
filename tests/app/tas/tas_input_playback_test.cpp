// Proves the Phase 1 movie input seam integrates with the REAL app input path:
// a firelight::tas::MovieInputProvider installed on the actual CoreInputRouter is
// polled and read exactly the way the libretro core polls/reads it each frame, and
// reproduces the movie's input bit-for-bit — buttons, mask, and analog axes. This
// is the "does a movie drive the live pipeline deterministically" gate; it runs
// headlessly (no core DLL, no video, no savestate).
#include <libretro/core_input_router.hpp>

#include <firelight/input/input_frame.hpp>
#include <firelight/tas/movie_input_provider.hpp>

#include "libretro/libretro.h"
#include <gtest/gtest.h>

#include <vector>

namespace firelight::tas {
namespace {

using input::InputFrame;

// A movie where each frame presses a distinct button plus a walking direction, so
// consecutive frames are never accidentally equal.
std::vector<InputFrame> makeMovie(std::size_t n) {
  std::vector<InputFrame> m;
  m.reserve(n);
  for (std::size_t k = 0; k < n; ++k) {
    InputFrame f;
    f.setButton(static_cast<unsigned>(k % 12), true);
    f.setButton(k % 2 ? RETRO_DEVICE_ID_JOYPAD_RIGHT : RETRO_DEVICE_ID_JOYPAD_LEFT,
                true);
    m.push_back(f);
  }
  return m;
}

// Reads one port's joypad state out of the router exactly as the core would.
InputFrame readPortJoypad(const libretro::CoreInputRouter &router, unsigned port) {
  InputFrame f;
  for (unsigned id = 0; id < 16; ++id) {
    if (router.readInputState(port, RETRO_DEVICE_JOYPAD, 0, id)) {
      f.setButton(id, true);
    }
  }
  return f;
}

} // namespace

// The per-frame drive loop the TAS session runs: pollInput() snapshots the movie's
// current frame, the core reads it via readInputState(), then advance() steps the
// cursor. The sequence the router yields must equal the movie.
TEST(TasInputPlaybackTest, MovieDrivesRealRouterInOrder) {
  const auto movie = makeMovie(24);
  MovieInputProvider provider(movie);

  libretro::CoreInputRouter router;
  router.setPlatformId(7);
  router.setRetropadProvider(&provider);

  std::vector<InputFrame> seen;
  for (std::size_t i = 0; i < movie.size(); ++i) {
    router.pollInput();                 // once per emulated frame
    seen.push_back(readPortJoypad(router, 0));
    // The button mask the core reads must match too.
    EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_JOYPAD, 0,
                                    RETRO_DEVICE_ID_JOYPAD_MASK),
              movie[i].buttonMask())
        << "frame " << i;
    provider.advance();
  }
  EXPECT_EQ(seen, movie);
  EXPECT_TRUE(provider.atEnd());
}

// Polling several times within one frame (the router polls all 8 ports each frame)
// must not advance the movie — the whole frame sees one consistent input.
TEST(TasInputPlaybackTest, RepeatedPollWithinFrameIsStable) {
  const auto movie = makeMovie(3);
  MovieInputProvider provider(movie);
  libretro::CoreInputRouter router;
  router.setPlatformId(7);
  router.setRetropadProvider(&provider);

  for (int poll = 0; poll < 5; ++poll) {
    router.pollInput();
    EXPECT_EQ(readPortJoypad(router, 0), movie[0]);
  }
  EXPECT_EQ(provider.cursor(), 0u);
}

// Analog axes propagate through the router's snapshot as well.
TEST(TasInputPlaybackTest, AnalogAxesReachTheCore) {
  InputFrame f;
  f.leftStickX = 1234;
  f.leftStickY = -2345;
  f.rightStickX = 3456;
  f.rightStickY = -4567;
  MovieInputProvider provider(std::vector<InputFrame>{f});

  libretro::CoreInputRouter router;
  router.setPlatformId(7);
  router.setRetropadProvider(&provider);
  router.pollInput();

  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_ANALOG,
                                  RETRO_DEVICE_INDEX_ANALOG_LEFT,
                                  RETRO_DEVICE_ID_ANALOG_X),
            1234);
  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_ANALOG,
                                  RETRO_DEVICE_INDEX_ANALOG_LEFT,
                                  RETRO_DEVICE_ID_ANALOG_Y),
            -2345);
  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_ANALOG,
                                  RETRO_DEVICE_INDEX_ANALOG_RIGHT,
                                  RETRO_DEVICE_ID_ANALOG_X),
            3456);
  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_ANALOG,
                                  RETRO_DEVICE_INDEX_ANALOG_RIGHT,
                                  RETRO_DEVICE_ID_ANALOG_Y),
            -4567);
}

// Past the movie's end the provider presents a neutral frame, so an over-run reads
// as all-released rather than replaying stale input into the core.
TEST(TasInputPlaybackTest, OverrunReadsNeutralThroughRouter) {
  MovieInputProvider provider(makeMovie(2));
  libretro::CoreInputRouter router;
  router.setPlatformId(7);
  router.setRetropadProvider(&provider);

  provider.seekTo(9); // past the end
  router.pollInput();
  EXPECT_EQ(readPortJoypad(router, 0), InputFrame{});
  EXPECT_EQ(router.readInputState(0, RETRO_DEVICE_JOYPAD, 0,
                                  RETRO_DEVICE_ID_JOYPAD_MASK),
            0);
}

} // namespace firelight::tas
