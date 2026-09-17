// TODO: NEEDS REVIEW
#include "../app/emulation/emulation_service_fixture.hpp"

#include <algorithm>
#include <cmath>
#include <emulation/emulation_loop.hpp>
#include <emulation/emulator_controller.hpp>
#include <emulation/emulator_instance.hpp>
#include <gtest/gtest.h>
#include <limits>
#include <string>
#include <vector>

namespace firelight::emulation {

namespace {
constexpr int64_t SECOND_NS = 1000000000;

// TODO
/** Seconds of a twenty second stretch the display's tally covers, the first two being settling */
constexpr double COUNTED_SECONDS = 18.0;
constexpr double SNES_FPS = 60.0988;

/** How late the loop wakes past its deadline at the least, which the real waiter spins down to */
constexpr int64_t WAKE_LATE_NS = 100000;

/** Time that moves only when the loop waits or the core runs */
class E2eClock : public ILoopClock {
public:
  int64_t nowNs() override { return m_nowNs; }

  void waitUntil(const int64_t untilNs) override { m_nowNs = std::max(m_nowNs, untilNs + m_wakeLateNs); }

  int64_t m_nowNs = SECOND_NS;
  int64_t m_wakeLateNs = 0;
};

/** Takes the core's frames and drops them; the core under test never hands any over */
class E2eReceiver : public libretro::IVideoDataReceiver {
public:
  void receive(const void *, unsigned, unsigned, size_t) override {}

  retro_hw_context_type getPreferredHwRender() override { return RETRO_HW_CONTEXT_NONE; }

  void setHwRenderInterface(retro_hw_render_callback *) override {}

  void setSystemAVInfo(retro_system_av_info *) override {}

  void setPixelFormat(retro_pixel_format *) override {}

  void setScreenRotation(unsigned) override {}

  void setHwRenderContextNegotiationInterface(retro_hw_render_context_negotiation_interface *) override {}

  void getHwRenderInterface(retro_hw_render_interface **) override {}
};

class E2eController : public IEmulatorController {
public:
  float playbackMultiplier() const override { return m_multiplier; }

  void setPlaybackMultiplier(const float multiplier) override { m_multiplier = multiplier; }

  bool paused() const override { return m_paused; }

  void setPaused(const bool paused) override { m_paused = paused; }

  void advanceOneFrame() override {}

  void writeSuspendPoint(int) override {}

  void loadSuspendPoint(int) override {}

  void captureScreenshot() override {}

  void captureVideoClip() override {}

  float m_multiplier = 1.0F;
  bool m_paused = false;
};

struct Noise {
  uint64_t state = 0x9E3779B97F4A7C15ULL;

  /** Uniform in [-1, 1) */
  double next() {
    state = state * 6364136223846793005ULL + 1442695040888963407ULL;
    return static_cast<double>(state >> 11) / static_cast<double>(1ULL << 53) * 2.0 - 1.0;
  }
};

/** A refresh the picture did not advance by exactly one frame at */
struct Hitch {
  int64_t atNs = 0;
  int refreshIndex = 0;
};

/** What the modelled display saw over a stretch of the run */
struct Seen {
  int framesRun = 0;
  int framesShown = 0;
  /** Frames the core ran that a newer one replaced before a refresh showed them */
  int neverSeen = 0;
  /** Refreshes with nothing new to show, after the first two seconds of the stretch */
  int repeats = 0;
  // TODO
  /** Every hitch in the order they happened, and when each frame ran */
  std::vector<Hitch> hitches;
  std::vector<int64_t> frameAtNs;
};

// TODO
/** The furthest any hitch strayed from a beat arriving beatHz times a second, in events */
double beatDeviation(const std::vector<Hitch> &hitches, const double beatHz) {
  auto deviation = 0.0;

  for (size_t index = 1; index < hitches.size(); ++index) {
    const auto elapsed = static_cast<double>(hitches[index].atNs - hitches[0].atNs) / SECOND_NS;
    deviation = std::max(deviation, std::abs(static_cast<double>(index) - beatHz * elapsed));
  }

  return deviation;
}

// TODO
/** The shortest gap between consecutive hitches, in refreshes */
int shortestHitchGap(const std::vector<Hitch> &hitches) {
  auto shortest = std::numeric_limits<int>::max();

  for (size_t index = 1; index < hitches.size(); ++index) {
    shortest = std::min(shortest, hitches[index].refreshIndex - hitches[index - 1].refreshIndex);
  }

  return hitches.size() < 2 ? 0 : shortest;
}

// TODO
/**
 * How far the game's clock strayed from the content's own rate over a stretch, in frames: the span
 * of the time elapsed against the frames that ran in it
 */
double deficitSpan(const std::vector<int64_t> &frameAtNs, const double contentFps) {
  auto lowest = 0.0;
  auto highest = 0.0;

  for (size_t index = 0; index < frameAtNs.size(); ++index) {
    const auto elapsed = static_cast<double>(frameAtNs[index] - frameAtNs[0]) / SECOND_NS;
    const auto deficit = elapsed * contentFps - static_cast<double>(index);
    lowest = std::min(lowest, deficit);
    highest = std::max(highest, deficit);
  }

  return frameAtNs.size() < 2 ? 0.0 : highest - lowest;
}

// TODO
// The real loop, service, instance and pacer over a fake core whose frames take a scripted amount of
// clock time, against a modelled display: every refresh shows the newest picture ready by then and
// tells the pacer a frame was submitted, the way the window does. Each test is a scenario; the
// display, the core's frame time and the loop's wake lateness are all set per stretch, so a run can
// change them part-way through
class EmulationLoopE2e : public EmulationServiceFixture {
protected:
  /** The display's real refresh rate, which the pacer never sees directly */
  double m_realHz = 59.959;

  /** Present jitter as a fraction of a refresh */
  double m_presentJitter = 0.0;

  /** How late the loop wakes past its deadline, at most, on top of WAKE_LATE_NS */
  int64_t m_wakeJitterNs = 600000;

  /** How long a frame takes the core, and how far every other one swings around that, peak to peak */
  int64_t m_frameTimeNs = 1500000;
  int64_t m_frameTimeSwingNs = 0;

  /** Every so many frames the core takes hitchNs instead; 0 for never */
  int m_hitchEveryFrames = 0;
  int64_t m_hitchNs = 0;

  /** Whether the window is presenting at all; while it is not, refreshes tell the pacer nothing */
  bool m_presenting = true;

  E2eClock m_clock;
  E2eController m_controller;
  E2eReceiver m_receiver;
  std::unique_ptr<EmulationLoop> m_loop;
  Noise m_noise;

  int64_t m_nextRefreshNs = SECOND_NS;
  int m_published = 0;
  int m_shown = 0;
  int64_t m_readyAtNs = 0;
  int m_refreshIndex = 0;
  std::vector<int64_t> m_frameAtNs;
  bool m_passRequested = true;
  bool m_slowFrame = false;
  int m_framesSinceHitch = 0;

  void SetUp() override {
    EmulationServiceFixture::SetUp();
    m_loop = std::make_unique<EmulationLoop>(m_clock, *m_emulationService, m_controller,
                                             LoopHooks{.receiver = [this] { return &m_receiver; },
                                                       .prepareForFrames = [] { return true; },
                                                       .requestPass = [this] { m_passRequested = true; }});
    m_loop->getPacer().notePassDuration(500000);
  }

  void TearDown() override {
    m_loop.reset();
    EmulationServiceFixture::TearDown();
  }

  /**
   * Loads the game and scripts the core's frame time onto the clock
   */
  void start() {
    ASSERT_NE(loadGame(), nullptr);
    ASSERT_NE(m_fakeCore, nullptr);
    m_fakeCore->m_onRun = [this] {
      m_frameAtNs.push_back(m_clock.m_nowNs);
      auto frameTimeNs = m_frameTimeNs + (m_slowFrame ? m_frameTimeSwingNs / 2 : -m_frameTimeSwingNs / 2);
      m_slowFrame = !m_slowFrame;

      if (m_hitchEveryFrames > 0 && ++m_framesSinceHitch >= m_hitchEveryFrames) {
        m_framesSinceHitch = 0;
        frameTimeNs = m_hitchNs;
      }

      m_clock.m_nowNs += frameTimeNs;
      m_published++;
      m_readyAtNs = m_clock.m_nowNs;
    };
  }

  void configure(const SyncMode mode, const double contentFps, const double reportedHz, const bool presentationLocked) {
    m_loop->getPacer().configure(
        {.mode = mode, .contentFps = contentFps, .displayHz = reportedHz, .presentationLocked = presentationLocked});
  }

  /**
   * Runs the loop for `seconds` of clock time against the display, returning what the display saw
   */
  Seen runFor(const double seconds) {
    Seen seen;
    m_frameAtNs.clear();
    const auto startNs = m_clock.m_nowNs;
    const auto endNs = startNs + static_cast<int64_t>(seconds * SECOND_NS);
    const auto framesBefore = m_fakeCore->frameCount();
    const auto refreshNs = static_cast<double>(SECOND_NS) / m_realHz;

    while (m_clock.m_nowNs < endNs) {
      const auto deadlineNs = m_loop->getPacer().getNextDeadlineNs();
      const auto wakeLateNs =
          WAKE_LATE_NS + static_cast<int64_t>((m_noise.next() + 1.0) * 0.5 * static_cast<double>(m_wakeJitterNs));
      const auto tickAtNs =
          deadlineNs > 0 ? std::max(deadlineNs + wakeLateNs, m_clock.m_nowNs) : m_clock.m_nowNs + SECOND_NS / 1000;

      while (m_nextRefreshNs <= tickAtNs) {
        const auto refreshAt = m_nextRefreshNs;
        const auto newest = m_readyAtNs <= refreshAt ? m_published : m_published - 1;

        // TODO
        // A window that is not presenting keeps the pass it was asked for, and presents it the moment
        // it can again
        if (!m_presenting) {
          m_nextRefreshNs = static_cast<int64_t>(static_cast<double>(m_nextRefreshNs) + refreshNs);
          continue;
        }

        if (newest > m_shown || m_passRequested || m_controller.m_paused) {
          m_loop->getPacer().noteSubmit(refreshAt + static_cast<int64_t>(m_noise.next() * m_presentJitter * refreshNs));
        }

        const auto isCounting = refreshAt > startNs + 2 * SECOND_NS && !m_controller.m_paused;

        if (newest > m_shown) {
          seen.framesShown++;

          if (newest - m_shown - 1 > 0 && isCounting) {
            seen.neverSeen += newest - m_shown - 1;
            seen.hitches.push_back({.atNs = refreshAt, .refreshIndex = m_refreshIndex});
          }

          m_shown = newest;
        } else if (isCounting) {
          seen.repeats++;
          seen.hitches.push_back({.atNs = refreshAt, .refreshIndex = m_refreshIndex});
        }

        m_refreshIndex++;
        m_passRequested = false;
        m_nextRefreshNs = static_cast<int64_t>(static_cast<double>(m_nextRefreshNs) + refreshNs);
      }

      m_clock.m_wakeLateNs = deadlineNs > 0 ? wakeLateNs : 0;
      m_loop->runOnce();
    }

    seen.framesRun = m_fakeCore->frameCount() - framesBefore;
    seen.frameAtNs = m_frameAtNs;
    return seen;
  }
};
} // namespace

//****************
// The contract
//****************

// TODO
/** How close a rate has to come to a whole division of the display to count as the same rate */
constexpr double CONTRACT_MATCH_TOLERANCE = 0.01;

// TODO
/** What a pair of rates alone says: the division of the display nearest one frame, and its cost */
struct Promise {
  int refreshes = 1;
  double dividedFps = 0.0;
  double error = 0.0;
  bool isCommensurate = false;
};

// TODO
/** The division of displayHz nearest one frame of contentFps, and whether it is close enough to count */
Promise promiseFor(const double displayHz, const double contentFps) {
  Promise promise;
  promise.refreshes = std::max(1, static_cast<int>(std::lround(displayHz / contentFps)));
  promise.dividedFps = displayHz / promise.refreshes;
  promise.error = std::abs(promise.dividedFps - contentFps) / contentFps;
  promise.isCommensurate = promise.error <= CONTRACT_MATCH_TOLERANCE;
  return promise;
}

// TODO
/**
 * Hitches the rates do not force. Two rates that differ cost one repeat or one frame never shown per
 * beat, whichever way round they are; one of each in the same stretch is a pair nothing asked for
 */
int extraHitches(const Seen &seen) { return 2 * std::min(seen.repeats, seen.neverSeen); }

// TODO
/** Names a case in a sweep, for a failure to point at */
std::string describe(const double panelHz, const double contentFps, const SyncMode mode) {
  const auto named = mode == SyncMode::Display ? "monitor" : mode == SyncMode::Fixed ? "native" : "auto";
  return std::string(named) + " at " + std::to_string(contentFps) + " fps on a " + std::to_string(panelHz) +
         " Hz panel";
}

// TODO
/**
 * One promise from docs/emulation-loop-design.md, "The contract": what a mode must run the game at
 * against a panel of a given real rate, and whether the clock is held to its grid
 */
struct ContractRow {
  const char *name;
  SyncMode mode;
  double contentFps;
  /** What the display says it does, and what it really does */
  double reportedHz;
  double realHz;
  bool followsTheDisplay;
  /** Refreshes a held clock steps by, or 0 where the clock must not be held */
  int heldRefreshes;
  /** Refreshes the display spends on one frame where the mode follows the display */
  int refreshesPerFrame;
};

const ContractRow CONTRACT[] = {
    {"native SNES", SyncMode::Fixed, SNES_FPS, 60.0, 59.959, false, 1, 0},
    {"native NTSC", SyncMode::Fixed, 60.0, 60.0, 59.959, false, 1, 0},
    {"native at half the refresh", SyncMode::Fixed, 30.0, 60.0, 59.959, false, 2, 0},
    {"native on a 240 Hz panel", SyncMode::Fixed, 60.0, 240.0, 239.76, false, 4, 0},
    {"native PAL on a 60 Hz panel", SyncMode::Fixed, 50.0, 60.0, 59.959, false, 0, 0},
    {"native on a 144 Hz panel", SyncMode::Fixed, 60.0, 144.0, 143.9, false, 0, 0},
    {"monitor SNES", SyncMode::Display, SNES_FPS, 60.0, 59.959, true, 0, 1},
    {"monitor PAL", SyncMode::Display, 50.0, 60.0, 59.959, true, 0, 1},
    {"monitor on a 144 Hz panel", SyncMode::Display, 60.0, 144.0, 143.9, true, 0, 2},
    {"auto takes the display for SNES", SyncMode::Auto, SNES_FPS, 60.0, 59.959, true, 0, 1},
    {"auto takes the display at half the refresh", SyncMode::Auto, 30.0, 60.0, 59.959, true, 0, 2},
    {"auto takes the display on a 240 Hz panel", SyncMode::Auto, 60.0, 240.0, 239.76, true, 0, 4},
    {"auto leaves PAL on its own rate", SyncMode::Auto, 50.0, 60.0, 59.959, false, 0, 0},
    {"auto leaves 60 fps alone on a 144 Hz panel", SyncMode::Auto, 60.0, 144.0, 143.9, false, 0, 0},
};

// TODO
// Native runs at the core's rate whatever the panel does; monitor runs at the panel's real rate or a
// whole division of it; auto picks between them by how far apart they are. Every row is checked
// against the frames the loop really ran and what the display really saw, not only against what the
// pacer reports
TEST_F(EmulationLoopE2e, EveryContractRowHolds) {
  start();

  for (const auto &row : CONTRACT) {
    m_realHz = row.realHz;
    configure(row.mode, row.contentFps, row.reportedHz, true);
    runFor(5.0);
    const auto seen = runFor(20.0);
    auto &pacer = m_loop->getPacer();
    const auto promisedFps = row.followsTheDisplay ? row.realHz / std::max(1, row.refreshesPerFrame) : row.contentFps;

    EXPECT_EQ(pacer.isFollowingTheDisplay(), row.followsTheDisplay) << row.name;
    EXPECT_EQ(pacer.getHeldRefreshes(), row.heldRefreshes) << row.name;
    EXPECT_EQ(pacer.getRefreshesPerFrame(), row.refreshesPerFrame) << row.name;

    // TODO
    // Tight enough that taking the rate the panel claims rather than the one it keeps fails here
    EXPECT_NEAR(pacer.getEffectiveFps(), promisedFps, promisedFps * 0.0004) << row.name;
    EXPECT_NEAR(pacer.getAudioRatio(), promisedFps / row.contentFps, 0.0004) << row.name;
    EXPECT_NEAR(seen.framesRun / 20.0, promisedFps, promisedFps * 0.004) << row.name;

    // TODO
    // A clock that is not held owes nothing at any point; a held one owes the beat it has not paid
    EXPECT_LE(deficitSpan(seen.frameAtNs, promisedFps), row.heldRefreshes > 0 ? 1.5 : 0.2) << row.name;

    // TODO
    // The rates decide how many refreshes have nothing new on them, and the loop adds none of its own.
    // Each end of the counted stretch is a refresh and a frame wide
    EXPECT_NEAR(seen.repeats - seen.neverSeen, (row.realHz - promisedFps) * COUNTED_SECONDS, 4.0) << row.name;
    EXPECT_LE(extraHitches(seen), 2) << row.name;
  }
}

// TODO
// Vsync decides tearing and latency. It does not decide which mode runs, nor the rate it runs at, so
// every row keeps its rate with presentation not waiting for the display. What the display makes of
// those frames is a separate promise the loop does not keep yet, so it is not checked here
TEST_F(EmulationLoopE2e, TheContractSurvivesVsyncBeingOff) {
  start();

  for (const auto &row : CONTRACT) {
    m_realHz = row.realHz;
    configure(row.mode, row.contentFps, row.reportedHz, false);
    runFor(5.0);
    const auto seen = runFor(20.0);
    auto &pacer = m_loop->getPacer();

    // TODO
    // Without presents standing for refreshes there is nothing to measure, so a mode that follows the
    // display follows the rate it was told
    const auto promisedFps =
        row.followsTheDisplay ? row.reportedHz / std::max(1, row.refreshesPerFrame) : row.contentFps;

    EXPECT_EQ(pacer.isFollowingTheDisplay(), row.followsTheDisplay) << row.name;
    EXPECT_EQ(pacer.getRefreshesPerFrame(), row.refreshesPerFrame) << row.name;
    EXPECT_EQ(pacer.getHeldRefreshes(), 0) << row.name;
    EXPECT_NEAR(seen.framesRun / 20.0, promisedFps, promisedFps * 0.004) << row.name;
    EXPECT_LE(deficitSpan(seen.frameAtNs, promisedFps), 0.2) << row.name;
  }
}

// TODO
// The rule the table is 14 examples of, checked against rates it does not name. For any panel and any
// core rate: monitor takes the nearest whole division of the panel, native keeps the core's rate and
// holds only where that rate is a division of the panel's, and auto is monitor where the division is
// within the threshold and native where it is not
TEST_F(EmulationLoopE2e, TheRuleHoldsForRatesTheTableDoesNotName) {
  const double PANELS[] = {49.92, 59.94, 59.959, 74.97, 100.0, 119.88, 143.9, 239.76};
  const double RATES[] = {25.0, 30.0, 50.0, 59.7275, 60.0, 60.0988, 60.5, 60.75};
  start();

  for (const auto panelHz : PANELS) {
    for (const auto contentFps : RATES) {
      const auto promise = promiseFor(panelHz, contentFps);
      m_realHz = panelHz;

      for (const auto mode : {SyncMode::Auto, SyncMode::Display, SyncMode::Fixed}) {
        const auto followsTheDisplay = mode == SyncMode::Display || (mode == SyncMode::Auto && promise.isCommensurate);
        const auto promisedFps = followsTheDisplay ? promise.dividedFps : contentFps;
        const auto heldRefreshes = mode == SyncMode::Fixed && promise.isCommensurate ? promise.refreshes : 0;
        const auto where = describe(panelHz, contentFps, mode);

        configure(mode, contentFps, panelHz, true);
        runFor(3.0);
        const auto seen = runFor(8.0);
        auto &pacer = m_loop->getPacer();

        EXPECT_EQ(pacer.isFollowingTheDisplay(), followsTheDisplay) << where;
        EXPECT_EQ(pacer.getHeldRefreshes(), heldRefreshes) << where;
        EXPECT_EQ(pacer.getRefreshesPerFrame(), followsTheDisplay ? promise.refreshes : 0) << where;
        EXPECT_NEAR(pacer.getEffectiveFps(), promisedFps, promisedFps * 0.0004) << where;
        EXPECT_NEAR(pacer.getAudioRatio(), promisedFps / contentFps, 0.0004) << where;
        EXPECT_NEAR(seen.framesRun / 8.0, promisedFps, promisedFps * 0.01) << where;
        EXPECT_LE(deficitSpan(seen.frameAtNs, promisedFps), heldRefreshes > 0 ? 1.5 : 0.2) << where;
      }
    }
  }
}

// TODO
// A panel that changes rate under the loop, which is what moving a window between two displays does.
// Monitor mode follows the new one; native keeps the core's rate through it either way
TEST_F(EmulationLoopE2e, EveryModeAnswersARefreshRateChangeTheWayItPromised) {
  start();

  for (const auto mode : {SyncMode::Display, SyncMode::Fixed}) {
    const auto followsTheDisplay = mode == SyncMode::Display;
    const auto where = describe(119.88, 60.0, mode);

    m_realHz = 59.959;
    configure(mode, 60.0, 60.0, true);
    runFor(5.0);
    EXPECT_NEAR(m_loop->getPacer().getEffectiveFps(), followsTheDisplay ? 59.959 : 60.0, 0.03) << where;

    // TODO
    // The panel changes and says nothing about it, so only the presents can tell the loop
    m_realHz = 119.88;
    runFor(10.0);
    const auto seen = runFor(20.0);

    EXPECT_NEAR(seen.framesRun / 20.0, followsTheDisplay ? 59.94 : 60.0, 0.25) << where;
    EXPECT_EQ(m_loop->getPacer().isFollowingTheDisplay(), followsTheDisplay) << where;
  }
}

//****************
// Steady scenarios
//****************

TEST_F(EmulationLoopE2e, MonitorModeOnAMisreportedPanelLosesNothing) {
  start();
  configure(SyncMode::Display, 60.0, 60.0, true);

  const auto seen = runFor(300.0);

  EXPECT_EQ(seen.neverSeen, 0);
  EXPECT_EQ(seen.repeats, 0);
  EXPECT_NEAR(seen.framesRun, 300.0 * 59.959, 3.0);
}

TEST_F(EmulationLoopE2e, NativeSnesPaysTheBeatAndNothingElse) {
  start();
  configure(SyncMode::Fixed, SNES_FPS, 60.0, true);

  const auto seen = runFor(300.0);
  const auto beatHz = SNES_FPS - 59.959;

  EXPECT_LE(beatDeviation(seen.hitches, beatHz), 1.0);
  EXPECT_GE(shortestHitchGap(seen.hitches), static_cast<int>(59.959 / beatHz * 0.9));
  EXPECT_LE(deficitSpan(seen.frameAtNs, SNES_FPS), 1.05);
  EXPECT_NEAR(seen.framesRun, 300.0 * SNES_FPS, 3.0);
}

TEST_F(EmulationLoopE2e, NativeWithAlternatingFrameTimesStaysAtTheBeat) {
  m_frameTimeNs = 10000000;
  m_frameTimeSwingNs = 6000000;
  start();
  configure(SyncMode::Fixed, 60.0, 60.0, true);

  const auto seen = runFor(300.0);
  const auto beatHz = 60.0 - 59.959;

  EXPECT_LE(beatDeviation(seen.hitches, beatHz), 1.0);
  EXPECT_GE(shortestHitchGap(seen.hitches), static_cast<int>(59.959 / beatHz * 0.9));
  EXPECT_LE(deficitSpan(seen.frameAtNs, 60.0), 1.5);
  EXPECT_NEAR(seen.framesRun, 300.0 * 60.0, 3.0);
}

// TODO
// Without vsync the presents say nothing about the refresh, so native is a free clock: the rate is
// exact, and where the picture lands against the refresh is wherever the drift put it
TEST_F(EmulationLoopE2e, NativeWithoutVsyncKeepsTheRateAndDithersAtTheBeat) {
  start();
  configure(SyncMode::Fixed, SNES_FPS, 60.0, false);

  const auto seen = runFor(300.0);

  EXPECT_LE(deficitSpan(seen.frameAtNs, SNES_FPS), 0.1);
  EXPECT_NEAR(seen.framesRun, 300.0 * SNES_FPS, 3.0);
}

TEST_F(EmulationLoopE2e, MonitorModeHoldsAFrameForTwoRefreshesAt120) {
  m_realHz = 119.961;
  start();
  configure(SyncMode::Display, 60.0, 120.0, true);

  const auto seen = runFor(300.0);

  EXPECT_EQ(seen.neverSeen, 0);
  EXPECT_NEAR(seen.framesRun, 300.0 * 119.961 / 2.0, 3.0);
}

TEST_F(EmulationLoopE2e, NativeOn144HzNeverDoublesAFrame) {
  m_realHz = 143.9;
  start();
  configure(SyncMode::Fixed, 60.0, 144.0, true);

  const auto seen = runFor(300.0);

  EXPECT_EQ(seen.neverSeen, 0);
  EXPECT_NEAR(seen.framesRun, 300.0 * 60.0, 3.0);
}

// TODO
// Fifty frames into sixty refreshes is too far from a whole division to hold, so the clock runs on
// its own period: every sixth refresh or so shows the same picture again, and nothing is ever owed
TEST_F(EmulationLoopE2e, PalOnASixtyHertzPanelPullsDownWithoutHolding) {
  start();
  configure(SyncMode::Fixed, 50.0, 60.0, true);

  const auto seen = runFor(300.0);

  EXPECT_EQ(seen.neverSeen, 0);
  EXPECT_GE(shortestHitchGap(seen.hitches), 5);
  EXPECT_LE(deficitSpan(seen.frameAtNs, 50.0), 0.1);
  EXPECT_NEAR(seen.framesRun, 300.0 * 50.0, 3.0);
}

// TODO
// A 30 ms frame every five seconds starts a couple of milliseconds before a blank and spans the next
// one too, so two refreshes show nothing new and the two frames that follow complete inside one
// refresh: two repeats and two doubles per hitch, on top of the beat, and the rate held to a frame
// or two per minute. A hitch that lands on a slip re-anchors and drops what was owed, which is the
// frame or two
TEST_F(EmulationLoopE2e, AThirtyMillisecondHitchCostsTwoRepeatsAndTwoDoubles) {
  m_hitchEveryFrames = 300;
  m_hitchNs = 30000000;
  start();
  configure(SyncMode::Fixed, SNES_FPS, 60.0, true);

  for (auto piece = 0; piece < 6; ++piece) {
    const auto seen = runFor(50.0);
    EXPECT_NEAR(seen.framesRun, 50.0 * SNES_FPS, 2.5) << "piece " << piece;
    EXPECT_LE(seen.repeats, 2 * 10 + 3) << "piece " << piece;
    EXPECT_LE(seen.neverSeen, 8 + 2 * 10 + 3) << "piece " << piece;
  }
}

//****************
// Things that change part-way
//****************

TEST_F(EmulationLoopE2e, ARefreshRateChangeRelocksWithinAFewFrames) {
  start();
  configure(SyncMode::Display, 60.0, 60.0, true);

  const auto before = runFor(100.0);

  m_realHz = 119.961;
  configure(SyncMode::Display, 60.0, 120.0, true);

  const auto after = runFor(200.0);

  EXPECT_EQ(before.neverSeen, 0);
  EXPECT_LE(after.neverSeen, 3);
  EXPECT_NEAR(before.framesRun, 100.0 * 59.959, 3.0);
  EXPECT_NEAR(after.framesRun, 200.0 * 119.961 / 2.0, 5.0);
}

// TODO
// Getting onto the display's grid moves the schedule, and where it moves to is not where the content
// had got to. Twenty trips on and off the grid, and the game has still been run for as many frames as
// the time that passed, to within the refresh the ledger settles to
TEST_F(EmulationLoopE2e, GettingOntoTheGridCostsTheGameNoTime) {
  start();
  configure(SyncMode::Fixed, SNES_FPS, 60.0, true);

  const auto beganNs = m_clock.m_nowNs;
  auto framesRun = 0;

  for (auto trip = 0; trip < 20; ++trip) {
    configure(SyncMode::Fixed, SNES_FPS, 60.0, false);
    framesRun += runFor(2.5).framesRun;
    configure(SyncMode::Fixed, SNES_FPS, 60.0, true);
    framesRun += runFor(2.5).framesRun;
  }

  const auto elapsed = static_cast<double>(m_clock.m_nowNs - beganNs) / SECOND_NS;

  EXPECT_NEAR(framesRun, elapsed * SNES_FPS, 2.0);
}

TEST_F(EmulationLoopE2e, VsyncTogglingMidRunKeepsTheRate) {
  start();
  configure(SyncMode::Fixed, SNES_FPS, 60.0, true);

  const auto on = runFor(100.0);
  configure(SyncMode::Fixed, SNES_FPS, 60.0, false);
  const auto off = runFor(100.0);
  configure(SyncMode::Fixed, SNES_FPS, 60.0, true);
  const auto onAgain = runFor(100.0);

  EXPECT_NEAR(on.framesRun + off.framesRun + onAgain.framesRun, 300.0 * SNES_FPS, 5.0);
  EXPECT_GE(on.neverSeen, 12);
  EXPECT_LE(on.neverSeen, 18);
  EXPECT_EQ(on.repeats, 0);
  EXPECT_GE(onAgain.neverSeen, 12);
  EXPECT_LE(onAgain.neverSeen, 20);
  EXPECT_LE(onAgain.repeats, 2);
}

TEST_F(EmulationLoopE2e, APauseOwesNothingAndResumesOnTheGrid) {
  start();
  configure(SyncMode::Fixed, SNES_FPS, 60.0, true);

  const auto before = runFor(100.0);
  m_controller.m_paused = true;
  const auto paused = runFor(5.0);
  m_controller.m_paused = false;
  const auto after = runFor(195.0);

  EXPECT_EQ(paused.framesRun, 0);
  EXPECT_NEAR(before.framesRun + after.framesRun, 295.0 * SNES_FPS, 5.0);
  EXPECT_LE(before.neverSeen + after.neverSeen, 48 + 2);
  EXPECT_LE(after.repeats, 2);
}

// TODO
// The window stops presenting for 400 ms every twenty seconds: the pacer notices within 250 ms,
// runs nothing until a present returns, and picks the cadence back up
TEST_F(EmulationLoopE2e, APresentStallStopsTheGameAndItResumes) {
  start();
  configure(SyncMode::Fixed, SNES_FPS, 60.0, true);

  auto total = 0;

  for (auto round = 0; round < 3; ++round) {
    total += runFor(19.6).framesRun;
    m_presenting = false;
    total += runFor(0.4).framesRun;
    m_presenting = true;
  }

  for (auto piece = 0; piece < 5; ++piece) {
    const auto last = runFor(2.0);
    EXPECT_NEAR(last.framesRun, 2.0 * SNES_FPS, 1.0)
        << "piece " << piece << " doubles " << last.neverSeen << " repeats " << last.repeats;
  }

  EXPECT_GE(total, static_cast<int>(58.0 * SNES_FPS));
  EXPECT_LE(total, static_cast<int>(60.0 * SNES_FPS) + 3);
}

} // namespace firelight::emulation
