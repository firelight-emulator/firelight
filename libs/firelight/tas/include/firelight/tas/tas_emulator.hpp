#pragma once

#include <firelight/libretro/retropad_provider.hpp>

#include <cstdint>
#include <vector>

namespace firelight::tas {

// The minimal emulator surface a TasSession drives, kept abstract so the session
// stays Qt/core-agnostic and unit-testable. The app implements this by delegating
// to EmulatorInstance (runFrame / serializeState / deserializeState) and the loaded
// core's setRetropadProvider(); tests implement a deterministic fake.
//
// All calls are single-threaded from the session's owner (the render thread in-app,
// the test thread in gtest), matching EmulatorInstance's render-thread confinement.
class ITasEmulator {
public:
  virtual ~ITasEmulator() = default;

  // Install the authoritative per-frame input source (the movie provider).
  virtual void setRetropadProvider(libretro::IRetropadProvider *provider) = 0;

  // Emulate exactly one frame, consuming the provider's current input.
  virtual void runFrame() = 0;

  // Whole-state snapshot / restore for the greenzone (an mGBA savestate blob).
  [[nodiscard]] virtual std::vector<uint8_t> serializeState() = 0;
  virtual bool deserializeState(const std::vector<uint8_t> &state) = 0;
};

} // namespace firelight::tas
