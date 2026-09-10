// TODO
// What each sync method resolves to on a given panel, and what it costs. native always runs the
// core's rate; display always runs a whole division of the monitor's, with no tolerance gate; auto
// picks display when the resulting speed error is inside the threshold and native otherwise
#include <cmath>
#include <cstdio>
#include <initializer_list>

namespace {

// TODO
/** How far display mode's speed may be from the content's before auto prefers native */
constexpr double AUTO_THRESHOLD = 0.01;

struct Content {
  const char *name;
  double fps;
};

/**
 * @return How many whole refreshes land nearest one content frame
 */
int refreshesPerFrame(const double displayHz, const double contentFps) {
  return std::max(1, static_cast<int>(std::lround(displayHz / contentFps)));
}

/**
 * @return How far a speed ratio is from unity in cents, which is what the pitch bend is heard as
 */
double cents(const double ratio) { return 1200.0 * std::log2(ratio); }

} // namespace

int main() {
  for (const auto &content : {Content{"SNES 60.0988", 60.0988}, Content{"NTSC 59.94", 59.94},
                              Content{"GBA  59.7275", 59.7275}, Content{"PAL  50.007", 50.007}}) {
    std::printf("=== content %s fps, auto threshold %.0f%% ===\n", content.name, AUTO_THRESHOLD * 100);
    std::printf("  %-9s %3s %10s %9s %8s   %s\n", "display", "k", "display=", "speed", "pitch", "auto picks");

    for (const auto displayHz : {59.959, 60.0, 75.0, 100.0, 119.961, 144.0, 165.0, 174.962, 240.0}) {
      const auto k = refreshesPerFrame(displayHz, content.fps);
      const auto locked = displayHz / k;
      const auto speed = locked / content.fps;

      std::printf("  %8.3f  %3d %9.3f  %+7.2f%% %+7.0fc   %s\n", displayHz, k, locked, (speed - 1.0) * 100,
                  cents(speed), std::fabs(1.0 - speed) <= AUTO_THRESHOLD ? "display" : "native");
    }

    std::printf("\n");
  }

  // TODO
  // Display mode's cadence is exact on every row above; what it costs is speed. Native's cost is the
  // other way round, and only reads as a periodic beat where the two rates are close
  std::printf("=== native mode: how often the beat repeats, where the rates are close ===\n");
  std::printf("  %-14s %-10s %12s %14s\n", "content", "display", "beat", "events/min");

  for (const auto &content :
       {Content{"SNES 60.0988", 60.0988}, Content{"NTSC 59.94", 59.94}, Content{"GBA  59.7275", 59.7275}}) {
    for (const auto displayHz : {59.959, 119.961}) {
      const auto k = refreshesPerFrame(displayHz, content.fps);
      const auto difference = std::fabs(content.fps - displayHz / k);

      std::printf("  %-14s %-10.3f %11.2fs %14.1f\n", content.name, displayHz, 1.0 / difference, 60.0 * difference);
    }
  }

  std::printf("\nAt 144 and 165 Hz there is no near match, so native holds each frame for 2 or 3 refreshes\n");
  std::printf("in an alternating pattern. That is pulldown rather than a beat, and has no period to report.\n");

  return 0;
}
