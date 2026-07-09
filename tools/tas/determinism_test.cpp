// tools/tas/determinism_test.cpp
//
// Gate 1a: internal-determinism check for a libretro core (the stock gambatte
// core Firelight bundles). Answers three questions a TAS tool depends on, and
// SWEEPS several input seeds because savestate reliability is seed-dependent
// (a single-seed run is misleading):
//
//   power-on            : two fresh runs with identical inputs -> identical video
//                         every frame. (Full-run determinism.)
//   savestate (same)    : within ONE live core, serialize at frame M, keep
//                         running to N (reference); unserialize back to M and
//                         replay to N. Must match. (Re-recording / RNG-search
//                         seek-back within a live editing session.)
//   savestate (fresh)   : serialize at M in one process, restore into a FRESH
//                         core, run to N. Must match. (Reloading a movie's
//                         embedded savestate in a new session -- portability.)
//
// Determinism is measured on the VIDEO FRAMEBUFFER (a pure function of the
// deterministic machine state), NOT the raw savestate blob -- gambatte's blob
// carries emulator-uninitialized padding that differs per process and would
// produce false mismatches.
//
// Standalone: dlopen/LoadLibrary the core; no Firelight/Qt deps. Build with
// tools/tas/build.sh. Usage:
//   determinism_test <core.(dll|so)> <rom> [frames=4000] [seeds=8]
//
// Exit 0 = power-on deterministic AND savestate(same) reliable across all seeds
// (authoring sound on the stock core). Non-zero otherwise.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>

#include "libretro.h" // from firelight/include/libretro (add -Iinclude/libretro)

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
  size_t (*serialize_size)();
  bool (*serialize)(void *, size_t);
  bool (*unserialize)(const void *, size_t);
  void (*get_system_info)(retro_system_info *);
  void (*get_system_av_info)(retro_system_av_info *);
};

Core g;
dl_t g_lib = nullptr;

const char *g_corePath = nullptr;
const char *g_romPath = nullptr;
std::vector<uint8_t> g_rom;
uint32_t g_frames = 4000;
uint32_t g_every = 100;
uint32_t g_saveAt = 2000;
uint32_t g_seed = 1;
uint32_t g_frame = 0;     // current frame, read by input_state_cb
uint64_t g_frameHash = 0; // hash of the last non-duplicate video frame

template <class T> void bind(T &fn, const char *name) {
  fn = reinterpret_cast<T>(dl_sym(g_lib, name));
  if (!fn) {
    std::fprintf(stderr, "FATAL: core missing symbol %s\n", name);
    std::exit(2);
  }
}

// Scripted, purely-frame-and-seed-derived input (reproducible across passes).
// Only Game Boy joypad ids: B(0) SELECT(2) START(3) UP(4) DOWN(5) LEFT(6)
// RIGHT(7) A(8) -> bitmask 0x1FD.
uint16_t frame_buttons(uint32_t frame) {
  uint32_t x = frame * 2654435761u + g_seed * 40503u + 12345u;
  x ^= x >> 13;
  x *= 1274126177u;
  x ^= x >> 16;
  return static_cast<uint16_t>(x & 0x1FDu);
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
  case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS: {
    uint64_t *q = static_cast<uint64_t *>(data);
    static bool printed = false;
    if (!printed) {
      std::printf("[env] core serialization quirks = 0x%llx%s\n",
                  static_cast<unsigned long long>(*q),
                  (*q & RETRO_SERIALIZATION_QUIRK_INCOMPLETE)
                      ? "  (INCOMPLETE flag set)"
                      : "");
      printed = true;
    }
    return true;
  }
  default:
    return false;
  }
}
void video_cb(const void *data, unsigned width, unsigned height, size_t pitch) {
  if (!data)
    return; // duplicate frame (GET_CAN_DUPE): keep the previous hash
  const uint8_t *p = static_cast<const uint8_t *>(data);
  const size_t rowBytes = static_cast<size_t>(width) * 2; // gambatte: 16bpp
  uint64_t h = 1469598103934665603ull;
  for (unsigned y = 0; y < height; ++y) {
    const uint8_t *row = p + static_cast<size_t>(y) * pitch;
    for (size_t i = 0; i < rowBytes; ++i) {
      h ^= row[i];
      h *= 1099511628211ull;
    }
  }
  g_frameHash = h;
}
void audio_cb(int16_t, int16_t) {}
size_t audio_batch_cb(const int16_t *, size_t frames) { return frames; }
void input_poll_cb() {}
int16_t input_state_cb(unsigned port, unsigned device, unsigned, unsigned id) {
  if (port != 0 || device != RETRO_DEVICE_JOYPAD)
    return 0;
  const uint16_t b = frame_buttons(g_frame);
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
  bind(g.serialize_size, "retro_serialize_size");
  bind(g.serialize, "retro_serialize");
  bind(g.unserialize, "retro_unserialize");
  bind(g.get_system_info, "retro_get_system_info");
  bind(g.get_system_av_info, "retro_get_system_av_info");
}

void wire() {
  g.set_environment(env_cb);
  g.set_video_refresh(video_cb);
  g.set_audio_sample(audio_cb);
  g.set_audio_sample_batch(audio_batch_cb);
  g.set_input_poll(input_poll_cb);
  g.set_input_state(input_state_cb);
}

// Fresh core instance (true power-on): load DLL, bind, wire, init, load game.
void openCoreAndGame() {
  g_lib = dl_open(g_corePath);
  if (!g_lib) {
    std::fprintf(stderr, "FATAL: cannot load core '%s'\n", g_corePath);
    std::exit(2);
  }
  bindAll();
  wire();
  g.init();
  retro_game_info gi{};
  gi.path = g_romPath;
  gi.data = g_rom.data();
  gi.size = g_rom.size();
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

struct Sample {
  uint32_t frame;
  uint64_t hash;
};

bool shouldSample(uint32_t fr) {
  return (fr % g_every) == 0 || fr == g_frames - 1;
}

// One pass on a FRESH core. If `restore` is set, unserialize it before running
// (fresh-instance savestate); else run from power-on. If `saveBlob` is set,
// capture the serialized state at g_saveAt.
void runPass(std::vector<Sample> &out, uint32_t startFrame,
             const std::vector<uint8_t> *restore,
             std::vector<uint8_t> *saveBlob) {
  openCoreAndGame();
  const size_t ss = g.serialize_size();
  if (ss == 0) {
    std::fprintf(stderr, "FATAL: serialize_size()==0\n");
    std::exit(3);
  }
  std::vector<uint8_t> buf(ss);
  if (restore && !g.unserialize(restore->data(), restore->size())) {
    std::fprintf(stderr, "FATAL: unserialize failed\n");
    std::exit(3);
  }
  for (g_frame = startFrame; g_frame < g_frames; ++g_frame) {
    g.run();
    if (shouldSample(g_frame))
      out.push_back({g_frame, g_frameHash});
    if (saveBlob && g_frame == g_saveAt) {
      g.serialize(buf.data(), ss);
      saveBlob->assign(buf.begin(), buf.end());
    }
  }
  closeCore();
}

// Same-instance savestate reproducibility: on ONE live core, run to g_saveAt and
// serialize; keep running to g_frames (reference); then unserialize back to
// g_saveAt and replay. Reference vs replay must match.
void runReplayTest(std::vector<Sample> &ref, std::vector<Sample> &rep) {
  openCoreAndGame();
  const size_t ss = g.serialize_size();
  std::vector<uint8_t> buf(ss), blob;
  for (g_frame = 0; g_frame < g_frames; ++g_frame) {
    g.run();
    if (g_frame > g_saveAt && shouldSample(g_frame))
      ref.push_back({g_frame, g_frameHash});
    if (g_frame == g_saveAt) {
      g.serialize(buf.data(), ss);
      blob.assign(buf.begin(), buf.end());
    }
  }
  if (!g.unserialize(blob.data(), blob.size())) {
    std::fprintf(stderr, "FATAL: unserialize failed (replay)\n");
    std::exit(3);
  }
  for (g_frame = g_saveAt + 1; g_frame < g_frames; ++g_frame) {
    g.run();
    if (shouldSample(g_frame))
      rep.push_back({g_frame, g_frameHash});
  }
  closeCore();
}

bool samplesEqual(const std::vector<Sample> &a, const std::vector<Sample> &b) {
  if (a.size() != b.size())
    return false;
  for (size_t i = 0; i < a.size(); ++i)
    if (a[i].frame != b[i].frame || a[i].hash != b[i].hash)
      return false;
  return true;
}

// Tail of `a` for frames > g_saveAt (aligns with a fresh/replay run that starts
// after the savestate).
std::vector<Sample> tailAfterSave(const std::vector<Sample> &a) {
  std::vector<Sample> t;
  for (const auto &s : a)
    if (s.frame > g_saveAt)
      t.push_back(s);
  return t;
}

void testSeed(uint32_t seed, bool &powerOk, bool &sameOk, bool &freshOk) {
  g_seed = seed;
  std::vector<Sample> A, B, C, refD, repD;
  std::vector<uint8_t> blob;
  runPass(A, 0, nullptr, &blob);   // power-on, capture savestate at g_saveAt
  runPass(B, 0, nullptr, nullptr); // power-on again
  runPass(C, g_saveAt + 1, &blob, nullptr); // fresh-instance restore
  runReplayTest(refD, repD);                // same-instance restore
  powerOk = samplesEqual(A, B);
  freshOk = samplesEqual(tailAfterSave(A), C);
  sameOk = samplesEqual(refD, repD);
}

} // namespace

int main(int argc, char **argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  if (argc < 3) {
    std::fprintf(stderr,
                 "usage: %s <core.(dll|so)> <rom> [frames=4000] [seeds=8]\n",
                 argv[0]);
    return 1;
  }
  g_corePath = argv[1];
  g_romPath = argv[2];
  g_frames = argc > 3 ? static_cast<uint32_t>(std::atoi(argv[3])) : 4000;
  const uint32_t nseeds = argc > 4 ? static_cast<uint32_t>(std::atoi(argv[4])) : 8;
  g_every = 100;
  g_saveAt = g_frames / 2;

  {
    std::ifstream f(g_romPath, std::ios::binary);
    if (!f) {
      std::fprintf(stderr, "FATAL: cannot open rom '%s'\n", g_romPath);
      return 2;
    }
    g_rom.assign(std::istreambuf_iterator<char>(f),
                 std::istreambuf_iterator<char>());
  }
  {
    g_lib = dl_open(g_corePath);
    if (!g_lib) {
      std::fprintf(stderr, "FATAL: cannot load core '%s'\n", g_corePath);
      return 2;
    }
    bindAll();
    retro_system_info si{};
    g.get_system_info(&si);
    std::printf("[core] %s  version=%s\n",
                si.library_name ? si.library_name : "?",
                si.library_version ? si.library_version : "?");
    std::printf("[rom]  %s  (%zu bytes)\n", g_romPath, g_rom.size());
    std::printf("[cfg]  frames=%u  savestate-at=%u  seeds=1..%u\n\n", g_frames,
                g_saveAt, nseeds);
    dl_close(g_lib);
    g_lib = nullptr;
  }

  uint32_t powerFail = 0, sameFail = 0, freshFail = 0;
  std::printf("seed   power-on   savestate(same)   savestate(fresh)\n");
  std::printf("----   --------   ---------------   ----------------\n");
  for (uint32_t s = 1; s <= nseeds; ++s) {
    bool p, sm, fr;
    testSeed(s, p, sm, fr);
    std::printf("%4u   %-8s   %-15s   %-s\n", s, p ? "PASS" : "FAIL",
                sm ? "PASS" : "FAIL", fr ? "PASS" : "FAIL");
    powerFail += !p;
    sameFail += !sm;
    freshFail += !fr;
  }

  std::printf("\n== aggregate over %u seeds ==\n", nseeds);
  std::printf("power-on determinism        : %u/%u PASS\n", nseeds - powerFail, nseeds);
  std::printf("savestate (same instance)   : %u/%u PASS\n", nseeds - sameFail, nseeds);
  std::printf("savestate (fresh instance)  : %u/%u PASS\n", nseeds - freshFail, nseeds);

  const bool powerOk = (powerFail == 0);
  const bool saveReliable = (sameFail == 0);
  std::printf("\n");
  if (powerOk && saveReliable) {
    std::printf("GATE 1a PASS: stock core is power-on deterministic AND "
                "savestate-reliable.\n");
    std::printf("=> Re-recording / RNG-search / verification are sound on the "
                "stock core.\n");
    if (freshFail)
      std::printf("NOTE: fresh-instance savestate reload is not fully reliable "
                  "(%u/%u) -> prefer power-on-start movies for portability.\n",
                  nseeds - freshFail, nseeds);
    return 0;
  }
  std::printf("GATE 1a FAIL (for savestate-based authoring).\n");
  if (powerOk)
    std::printf("Power-on determinism is solid, but stock gambatte v0.5.0 "
                "SAVESTATES ARE UNRELIABLE (%u/%u same-instance failures).\n",
                sameFail, nseeds);
  std::printf("Re-recording and RNG-search need reliable savestate branching -> "
              "use the gambatte-speedrun accuracy core (Risk 1b) for TAS "
              "authoring, not stock gambatte.\n");
  return 4;
}
