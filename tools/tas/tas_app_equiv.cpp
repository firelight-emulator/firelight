// app-vs-CLI framebuffer equivalence gate (roadmap Phase 1, critical path #2).
//
// Replays a movie through the REAL app emulation path — libretro::Core, the exact
// wrapper the GUI drives — and asserts each frame's framebuffer hash equals the CLI
// oracle's stored checkpoints (tools/tas `gen` records g_frameHash per checkpoint).
// If they match, the `verify` gate over the CLI genuinely protects the app pipeline.
//
// Standalone (its OWN process, linking firelight_emulation_lib — no Qt Quick/Vulkan)
// so a real-core crash can never take down fl_test. The Core is intentionally leaked:
// its destructor unloads the DLL (retro_deinit), which exits the process for a real
// core; a one-shot check never needs teardown. mGBA is a software core
// (RETRO_HW_CONTEXT_NONE), so a hashing video receiver suffices — no GPU context.
//
// Usage: tas_app_equiv <core> <rom> <movie.fltm>
// Exit: 0 = PASS or SKIP (inputs missing); 1 = MISMATCH; 2 = load/setup error.

#include "fltm.hpp"

#include "libretro/core.hpp"
#include "libretro/game.hpp"

#include <firelight/libretro/audio_output.hpp>
#include <firelight/libretro/configuration_provider.hpp>
#include <firelight/libretro/core_run_config.hpp>
#include <firelight/libretro/video_data_receiver.hpp>
#include <firelight/tas/movie_input_provider.hpp>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace {

// Minimal core-options provider: the core registers options during construction but
// this gate drives none, so every query is a no-op / default.
class StubConfigProvider : public firelight::libretro::IConfigurationProvider {
public:
  void registerOption(Option) override {}
  [[nodiscard]] std::vector<Option> getOptions() const override { return {}; }
  bool anyOptionValueHasChanged() override { return false; }
  void setDefaultValue(const std::string &, const std::string &) override {}
  std::optional<std::string> getOptionValue(const std::string &) override {
    return std::nullopt;
  }
  void setOptionVisibility(const std::string &, bool) override {}
};

// Inert audio sink. Required: Core::loadGame() calls audioReceiver->initialize()
// and the audio batch callback calls audioReceiver->receive() — a null receiver
// segfaults. Audio is irrelevant to framebuffer equivalence, so this discards it.
class NullAudioOutput : public IAudioOutput {
public:
  size_t receive(const int16_t *, size_t numFrames) override { return numFrames; }
  void initialize(double) override {}
  void setMuted(bool) override {}
  [[nodiscard]] bool isMuted() const override { return true; }
  void setPaused(bool) override {}
  [[nodiscard]] float getBufferLevel() const override { return 0.0f; }
  void setPlaybackRateRatio(double) override {}
  void setDynamicRateControlEnabled(bool) override {}
};

// Hashes each frame with the identical algorithm tools/tas uses (continuing FNV-1a
// over width*bpp bytes per row, honoring pitch), so a hash equals the CLI's for the
// same framebuffer bytes. bpp follows the core's declared pixel format.
class HashingVideoReceiver : public firelight::libretro::IVideoDataReceiver {
public:
  std::vector<uint64_t> hashes;

  void receive(const void *data, unsigned width, unsigned height,
               size_t pitch) override {
    if (!data) {
      hashes.push_back(0);
      return;
    }
    const auto *p = static_cast<const uint8_t *>(data);
    const size_t rowBytes = static_cast<size_t>(width) * m_bpp;
    uint64_t h = 1469598103934665603ull;
    for (unsigned y = 0; y < height; ++y) {
      const uint8_t *row = p + static_cast<size_t>(y) * pitch;
      for (size_t i = 0; i < rowBytes; ++i) {
        h ^= row[i];
        h *= 1099511628211ull;
      }
    }
    hashes.push_back(h);
  }

  retro_hw_context_type getPreferredHwRender() override {
    return RETRO_HW_CONTEXT_NONE;
  }
  void setHwRenderInterface(retro_hw_render_callback *) override {}
  void setSystemAVInfo(retro_system_av_info *) override {}
  void setPixelFormat(retro_pixel_format *format) override {
    m_bpp = (format && *format == RETRO_PIXEL_FORMAT_XRGB8888) ? 4 : 2;
  }
  void setScreenRotation(unsigned) override {}
  void setHwRenderContextNegotiationInterface(
      retro_hw_render_context_negotiation_interface *) override {}
  void getHwRenderInterface(retro_hw_render_interface **) override {}

private:
  int m_bpp = 2;
};

} // namespace

int main(int argc, char **argv) {
  if (argc < 4) {
    std::fprintf(stderr, "usage: %s <core> <rom> <movie.fltm>\n", argv[0]);
    return 2;
  }
  namespace fs = std::filesystem;
  const std::string core = argv[1], rom = argv[2], moviePath = argv[3];
  for (const auto *p : {&core, &rom, &moviePath}) {
    if (!fs::exists(*p)) {
      std::printf("tas_app_equiv: SKIP (missing input: %s)\n", p->c_str());
      return 0;
    }
  }

  tas::Movie movie;
  if (!movie.load(moviePath)) {
    std::fprintf(stderr, "FATAL: cannot load movie '%s'\n", moviePath.c_str());
    return 2;
  }
  if (movie.input.empty() || movie.checkpoints.empty()) {
    std::fprintf(stderr, "FATAL: movie needs input + checkpoints (gen with a "
                         "checkpoint interval)\n");
    return 2;
  }

  std::ifstream romFile(rom, std::ios::binary);
  const std::vector<unsigned char> romBytes(
      (std::istreambuf_iterator<char>(romFile)),
      std::istreambuf_iterator<char>());

  firelight::libretro::CoreRunConfig cfg;
  cfg.platformId = 0;
  cfg.corePath = core;
  cfg.configProvider = std::make_shared<StubConfigProvider>();
  cfg.systemDirectory = ".";
  cfg.saveDirectory = ".";

  // Intentionally leaked (see header): never destruct a real core in-process.
  auto *appCore = new libretro::Core(std::move(cfg));

  HashingVideoReceiver video;
  NullAudioOutput audio;
  firelight::tas::MovieInputProvider provider(movie.input);
  // Attach before loadGame: the core declares its pixel format during load, and
  // loadGame calls audioReceiver->initialize() (null-derefs without a receiver).
  appCore->setVideoReceiver(&video);
  appCore->setAudioReceiver(std::shared_ptr<IAudioOutput>(&audio, [](IAudioOutput *) {}));
  appCore->setRetropadProvider(&provider);

  appCore->init();
  auto *game = new libretro::Game(rom, romBytes);
  if (!appCore->loadGame(game)) {
    std::fprintf(stderr, "FATAL: app core failed to load ROM\n");
    return 2;
  }

  const auto frames = static_cast<uint32_t>(movie.input.size());
  for (uint32_t f = 0; f < frames; ++f) {
    appCore->run(0.0); // one retro_run -> one framebuffer -> one hash
    provider.advance();
  }
  if (video.hashes.size() != frames) {
    std::fprintf(stderr, "FATAL: got %zu frames, expected %u\n",
                 video.hashes.size(), frames);
    return 2;
  }

  // Equivalence: at every frame the CLI checkpointed, the app framebuffer hash
  // must match — the app path reproduces the CLI oracle bit-for-bit.
  uint32_t mismatches = 0;
  for (const auto &[frame, cliHash] : movie.checkpoints) {
    if (frame >= video.hashes.size()) {
      continue;
    }
    if (video.hashes[frame] != cliHash) {
      ++mismatches;
      std::printf("  MISMATCH @frame %u: app=%016llx cli=%016llx\n", frame,
                  static_cast<unsigned long long>(video.hashes[frame]),
                  static_cast<unsigned long long>(cliHash));
    }
  }

  std::printf("tas_app_equiv: %s (%zu checkpoints over %u frames, core=%s %s)\n",
              mismatches == 0 ? "PASS" : "FAIL", movie.checkpoints.size(), frames,
              movie.coreName.c_str(), movie.coreVersion.c_str());
  return mismatches == 0 ? 0 : 1;
}
