// TODO
// Drives the real FramePacer against present trains whose timestamps carry increasing noise, to
// show how the current design loses frames. Links production code; everything else here models it
#include "frame_pacer.hpp"

#include <cstdio>
#include <random>

namespace {

using namespace firelight::emulation;

// TODO
/** Frames a single render pass may carry, matching render() in emulator_item_renderer.cpp */
constexpr int FRAMES_PER_PASS = 2;

// TODO
/** How much of a second one run covers */
constexpr int PRESENTS_PER_RUN = 3600;

/**
 * What one run of a present train produced
 */
struct Outcome {
  int framesRun = 0;
  int framesShown = 0;
};

/**
 * Replays a present train through the pacer, timestamping each present with noise of the given
 * fraction of a refresh. Only the last frame of a pass reaches the display, so anything a pass
 * carries beyond the first is a frame nobody sees
 */
Outcome replay(const double displayHz, const double contentFps, const double noiseFraction, const unsigned seed) {
  FramePacer pacer;
  const PacingContext context{SyncMode::Display, contentFps, displayHz, true};
  pacer.configure(context);
  pacer.setReady(true);
  pacer.setPaused(false);

  const auto periodNs = 1e9 / displayHz;
  std::mt19937 rng(seed);
  std::uniform_real_distribution noise(-noiseFraction, noiseFraction);

  auto vsyncNs = 1e9;
  auto owed = 0;
  Outcome outcome;

  for (auto present = 0; present < PRESENTS_PER_RUN; ++present) {
    vsyncNs += periodNs;
    const auto stampNs = static_cast<int64_t>(vsyncNs + noise(rng) * periodNs);

    pacer.noteSubmit(stampNs);
    owed += pacer.tick(stampNs).framesToRun;

    if (owed <= 0) {
      continue;
    }

    const auto thisPass = owed < FRAMES_PER_PASS ? owed : FRAMES_PER_PASS;
    owed -= thisPass;
    outcome.framesRun += thisPass;
    outcome.framesShown++;
  }

  return outcome;
}

} // namespace

int main() {
  // TODO
  // The panel and core from the frame-pacing investigation, where refreshesPerFrame lands on 1
  constexpr auto DISPLAY_HZ = 59.959;
  constexpr auto CONTENT_FPS = 60.0988;

  std::printf("Display mode, %.3f Hz panel, %.4f fps core, %d presents per run.\n", DISPLAY_HZ, CONTENT_FPS,
              PRESENTS_PER_RUN);
  std::printf("Frames run are conserved; what varies is how many reach a refresh.\n\n");
  std::printf("  %-14s %10s %10s %12s %10s\n", "noise", "run", "shown", "never seen", "share");

  for (const auto noiseFraction : {0.0, 0.01, 0.02, 0.05, 0.10, 0.20, 0.35, 0.50}) {
    const auto outcome = replay(DISPLAY_HZ, CONTENT_FPS, noiseFraction, 12345);
    const auto neverSeen = outcome.framesRun - outcome.framesShown;

    std::printf("  +/-%-10.1f%% %10d %10d %12d %9.1f%%\n", noiseFraction * 100, outcome.framesRun, outcome.framesShown,
                neverSeen, 100.0 * neverSeen / outcome.framesRun);
  }

  return 0;
}
