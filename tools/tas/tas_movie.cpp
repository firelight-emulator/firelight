// tools/tas/tas_movie.cpp
//
// Headless TAS movie engine (Phase 1) for Firelight, driving a libretro core
// directly (mGBA for Game Boy -- gate 1a showed stock gambatte v0.5.0 savestates
// are unreliable; mGBA passes fully). No Qt dependency. Subcommands:
//
//   gen    <core> <rom> <out.fltm> [frames=4000] [seed=1]
//            Generate a scripted-input movie (for exercising the pipeline) and
//            embed per-checkpoint video-framebuffer hashes.
//   play   <core> <rom> <movie.fltm>
//            Replay the movie deterministically; print progress + final hash and
//            report checkpoint matches if the movie carries them.
//   verify <core> <rom> <movie.fltm>
//            Replay twice (determinism) AND check every embedded checkpoint
//            (sync). Exit 0 iff both pass.
//
// Determinism is measured on the VIDEO framebuffer (see determinism_test.cpp for
// why the raw savestate blob is a poor metric). Build via tools/tas/build.sh.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "fltm.hpp"
#include "libretro.h"

#ifdef _WIN32
#include <windows.h>
using dl_t = HMODULE;
static dl_t dl_open(const char *p) { return LoadLibraryA(p); }
static void *dl_sym(dl_t h, const char *s) {
  return reinterpret_cast<void *>(GetProcAddress(h, s));
}
static void dl_close(dl_t h) { FreeLibrary(h); }
#else
#include <dlfcn.h>
using dl_t = void *;
static dl_t dl_open(const char *p) { return dlopen(p, RTLD_NOW | RTLD_LOCAL); }
static void *dl_sym(dl_t h, const char *s) { return dlsym(h, s); }
static void dl_close(dl_t h) { dlclose(h); }
#endif

#ifndef RETRO_DEVICE_ID_JOYPAD_MASK
#define RETRO_DEVICE_ID_JOYPAD_MASK 256
#endif

using tas::InputFrame;
using tas::Movie;

namespace {

struct Core {
  void (*set_environment)(retro_environment_t);
  void (*set_video_refresh)(retro_video_refresh_t);
  void (*set_audio_sample)(retro_audio_sample_t);
  void (*set_audio_sample_batch)(retro_audio_sample_batch_t);
  void (*set_input_poll)(retro_input_poll_t);
  void (*set_input_state)(retro_input_state_t);
  void (*init)();
  void (*deinit)();
  void (*run)();
  bool (*load_game)(const retro_game_info *);
  void (*unload_game)();
  void (*get_system_info)(retro_system_info *);
  void (*get_system_av_info)(retro_system_av_info *);
};

Core g;
dl_t g_lib = nullptr;
uint32_t g_frame = 0;
uint64_t g_frameHash = 0;
const Movie *g_movie = nullptr;
const std::vector<uint8_t> *g_rom = nullptr;
const char *g_romPath = nullptr;

uint64_t fnv1a(const void *p, size_t n, uint64_t h = 1469598103934665603ull) {
  const uint8_t *b = static_cast<const uint8_t *>(p);
  for (size_t i = 0; i < n; ++i) {
    h ^= b[i];
    h *= 1099511628211ull;
  }
  return h;
}

template <class T> void bind(T &fn, const char *name) {
  fn = reinterpret_cast<T>(dl_sym(g_lib, name));
  if (!fn) {
    std::fprintf(stderr, "FATAL: core missing symbol %s\n", name);
    std::exit(2);
  }
}

bool env_cb(unsigned cmd, void *data) {
  switch (cmd) {
  case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
    return true;
  case RETRO_ENVIRONMENT_GET_CAN_DUPE:
    *static_cast<bool *>(data) = true;
    return true;
  case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
  case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
    *static_cast<const char **>(data) = ".";
    return true;
  default:
    return false;
  }
}
void video_cb(const void *data, unsigned width, unsigned height, size_t pitch) {
  if (!data)
    return;
  const uint8_t *p = static_cast<const uint8_t *>(data);
  const size_t rowBytes = static_cast<size_t>(width) * 2; // 16bpp
  uint64_t h = 1469598103934665603ull;
  for (unsigned y = 0; y < height; ++y)
    h = fnv1a(p + static_cast<size_t>(y) * pitch, rowBytes, h);
  g_frameHash = h;
}
void audio_cb(int16_t, int16_t) {}
size_t audio_batch_cb(const int16_t *, size_t frames) { return frames; }
void input_poll_cb() {}
int16_t input_state_cb(unsigned port, unsigned device, unsigned, unsigned id) {
  if (port != 0 || device != RETRO_DEVICE_JOYPAD || !g_movie ||
      g_frame >= g_movie->input.size())
    return 0;
  const uint16_t b = g_movie->input[g_frame].buttons;
  if (id == RETRO_DEVICE_ID_JOYPAD_MASK)
    return static_cast<int16_t>(b);
  return (b >> (id & 15)) & 1;
}

void bindAll() {
  bind(g.set_environment, "retro_set_environment");
  bind(g.set_video_refresh, "retro_set_video_refresh");
  bind(g.set_audio_sample, "retro_set_audio_sample");
  bind(g.set_audio_sample_batch, "retro_set_audio_sample_batch");
  bind(g.set_input_poll, "retro_set_input_poll");
  bind(g.set_input_state, "retro_set_input_state");
  bind(g.init, "retro_init");
  bind(g.deinit, "retro_deinit");
  bind(g.run, "retro_run");
  bind(g.load_game, "retro_load_game");
  bind(g.unload_game, "retro_unload_game");
  bind(g.get_system_info, "retro_get_system_info");
  bind(g.get_system_av_info, "retro_get_system_av_info");
}

void openCore(const char *corePath) {
  g_lib = dl_open(corePath);
  if (!g_lib) {
    std::fprintf(stderr, "FATAL: cannot load core '%s'\n", corePath);
    std::exit(2);
  }
  bindAll();
  g.set_environment(env_cb);
  g.set_video_refresh(video_cb);
  g.set_audio_sample(audio_cb);
  g.set_audio_sample_batch(audio_batch_cb);
  g.set_input_poll(input_poll_cb);
  g.set_input_state(input_state_cb);
  g.init();
  retro_game_info gi{};
  gi.path = g_romPath;
  gi.data = g_rom->data();
  gi.size = g_rom->size();
  gi.meta = nullptr;
  if (!g.load_game(&gi)) {
    std::fprintf(stderr, "FATAL: load_game failed\n");
    std::exit(3);
  }
  retro_system_av_info av{};
  g.get_system_av_info(&av);
}

void closeCore() {
  g.unload_game();
  g.deinit();
  dl_close(g_lib);
  g_lib = nullptr;
}

// Replay the current g_movie on a fresh core, sampling the framebuffer hash
// every `every` frames (and on the last frame). Fills `out`.
void replay(const char *corePath,
            std::vector<std::pair<uint32_t, uint64_t>> &out,
            uint32_t every = 100) {
  openCore(corePath);
  const uint32_t n = static_cast<uint32_t>(g_movie->input.size());
  for (g_frame = 0; g_frame < n; ++g_frame) {
    g.run();
    if ((g_frame % every) == 0 || g_frame == n - 1)
      out.push_back({g_frame, g_frameHash});
  }
  closeCore();
}

std::vector<uint8_t> readFile(const char *path) {
  std::ifstream f(path, std::ios::binary);
  if (!f)
    return {};
  return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)),
                              std::istreambuf_iterator<char>());
}

std::string baseName(const std::string &p) {
  const auto s = p.find_last_of("/\\");
  return s == std::string::npos ? p : p.substr(s + 1);
}

// Scripted, purely-frame-and-seed-derived input (GB joypad ids only, mask
// 0x1FD). Matches determinism_test so movies are reproducible.
uint16_t scriptedButtons(uint32_t frame, uint32_t seed) {
  uint32_t x = frame * 2654435761u + seed * 40503u + 12345u;
  x ^= x >> 13;
  x *= 1274126177u;
  x ^= x >> 16;
  return static_cast<uint16_t>(x & 0x1FDu);
}

int usage(const char *argv0) {
  std::fprintf(stderr,
               "usage:\n"
               "  %s gen    <core> <rom> <out.fltm> [frames=4000] [seed=1]\n"
               "  %s play   <core> <rom> <movie.fltm>\n"
               "  %s verify <core> <rom> <movie.fltm>\n",
               argv0, argv0, argv0);
  return 1;
}

bool cmpCheckpoints(const std::vector<std::pair<uint32_t, uint64_t>> &got,
                    const std::vector<std::pair<uint32_t, uint64_t>> &want,
                    uint32_t &firstBadFrame) {
  if (got.size() != want.size())
    return false;
  for (size_t i = 0; i < got.size(); ++i)
    if (got[i] != want[i]) {
      firstBadFrame = want[i].first;
      return false;
    }
  return true;
}

} // namespace

int main(int argc, char **argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  if (argc < 5)
    return usage(argv[0]);

  const std::string cmd = argv[1];
  const char *corePath = argv[2];
  g_romPath = argv[3];
  const std::string movPath = argv[4];
  const uint32_t every = 100;

  const std::vector<uint8_t> rom = readFile(g_romPath);
  if (rom.empty()) {
    std::fprintf(stderr, "FATAL: cannot read rom '%s'\n", g_romPath);
    return 2;
  }
  g_rom = &rom;
  const uint64_t romHash = fnv1a(rom.data(), rom.size());

  if (cmd == "gen") {
    const uint32_t frames = argc > 5 ? std::atoi(argv[5]) : 4000;
    const uint32_t seed = argc > 6 ? std::atoi(argv[6]) : 1;
    Movie m;
    m.romName = baseName(g_romPath);
    m.romHash = romHash;
    m.romSize = static_cast<uint32_t>(rom.size());
    m.input.resize(frames);
    for (uint32_t i = 0; i < frames; ++i)
      m.input[i].buttons = scriptedButtons(i, seed);

    g_movie = &m;
    openCore(corePath);
    retro_system_info si{};
    g.get_system_info(&si);
    m.coreName = si.library_name ? si.library_name : "?";
    m.coreVersion = si.library_version ? si.library_version : "?";
    for (g_frame = 0; g_frame < frames; ++g_frame) {
      g.run();
      if ((g_frame % every) == 0 || g_frame == frames - 1)
        m.checkpoints.push_back({g_frame, g_frameHash});
    }
    closeCore();

    if (!m.save(movPath)) {
      std::fprintf(stderr, "FATAL: cannot write '%s'\n", movPath.c_str());
      return 3;
    }
    std::printf("wrote %s\n  core=%s %s\n  frames=%u  checkpoints=%zu  seed=%u\n",
                movPath.c_str(), m.coreName.c_str(), m.coreVersion.c_str(),
                frames, m.checkpoints.size(), seed);
    return 0;
  }

  Movie m;
  if (!m.load(movPath)) {
    std::fprintf(stderr, "FATAL: cannot load movie '%s'\n", movPath.c_str());
    return 2;
  }
  g_movie = &m;
  std::printf("[movie] %s\n  core=%s %s  frames=%zu  checkpoints=%zu  rom=%s\n",
              movPath.c_str(), m.coreName.c_str(), m.coreVersion.c_str(),
              m.input.size(), m.checkpoints.size(), m.romName.c_str());
  if (m.romHash != romHash)
    std::printf("  WARNING: ROM hash mismatch -- movie was made for a different "
                "ROM (0x%016llx vs 0x%016llx)\n",
                static_cast<unsigned long long>(m.romHash),
                static_cast<unsigned long long>(romHash));

  if (cmd == "play") {
    std::vector<std::pair<uint32_t, uint64_t>> got;
    replay(corePath, got);
    std::printf("replayed %zu frames; final framebuffer hash = 0x%016llx\n",
                m.input.size(),
                static_cast<unsigned long long>(got.empty() ? 0
                                                            : got.back().second));
    if (!m.checkpoints.empty()) {
      uint32_t bad = 0;
      const bool ok = cmpCheckpoints(got, m.checkpoints, bad);
      if (ok)
        std::printf("checkpoints: %zu/%zu match\n", m.checkpoints.size(),
                    m.checkpoints.size());
      else
        std::printf("checkpoints: MISMATCH at/near frame %u\n", bad);
    }
    return 0;
  }

  if (cmd == "verify") {
    std::vector<std::pair<uint32_t, uint64_t>> a, b;
    replay(corePath, a);
    replay(corePath, b);
    const bool det = (a == b);
    uint32_t bad = 0;
    const bool cp =
        m.checkpoints.empty() ? true : cmpCheckpoints(a, m.checkpoints, bad);
    std::printf("\ndeterminism (replay x2) : %s\n", det ? "PASS" : "FAIL");
    if (m.checkpoints.empty())
      std::printf("checkpoint match        : (movie has no checkpoints)\n");
    else
      std::printf("checkpoint match        : %s%s\n", cp ? "PASS" : "FAIL",
                  cp ? "" : "");
    const bool ok = det && cp;
    std::printf("\n%s\n", ok ? "MOVIE VERIFIED" : "MOVIE FAILED VERIFICATION");
    return ok ? 0 : 4;
  }

  return usage(argv[0]);
}
