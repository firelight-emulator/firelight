// tools/tas/determinism_test.cpp
//
// Gate 1a: internal-determinism check for a libretro core (the stock gambatte
// core Firelight bundles). Proves the core replays IDENTICALLY given the same
// inputs and savestates -- the prerequisite for TAS re-recording, RNG-search,
// and playback verification. It does NOT test console/BizHawk accuracy (that is
// Risk 1b, deferred); it tests self-consistency of THIS core build.
//
// Three passes, driven by a deterministic scripted input stream:
//   A (power-on determinism): fresh load + run N frames, hashing serialized
//     state every K frames.
//   B (power-on determinism, repeat): fresh load + run N frames with the SAME
//     inputs. Every hash must equal pass A. (Same input + same start => same state.)
//   C (savestate determinism): restore the state serialized at frame M in pass A
//     into a fresh load, run M+1..N with the same inputs. Hashes for those frames
//     must equal pass A -- i.e. serialize/unserialize is frame-exact and the core
//     does not report RETRO_SERIALIZATION_QUIRK_INCOMPLETE in a way that breaks sync.
//
// Standalone: dlopen/LoadLibrary the core at runtime; no Firelight/Qt deps.
// Build with tools/tas/build.sh. Usage:
//   determinism_test <core.(dll|so)> <rom> [frames=4000] [seed=1]
//
// Exit code 0 = all passes matched (core is internally deterministic). Non-zero
// = a mismatch or a load/serialize failure (details printed).

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
#else
#include <dlfcn.h>
using dl_t = void *;
static dl_t dl_open(const char *p) { return dlopen(p, RTLD_NOW); }
static void *dl_sym(dl_t h, const char *s) { return dlsym(h, s); }
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
};

Core g;
dl_t g_lib = nullptr;

template <class T> void bind(T &fn, const char *name) {
  fn = reinterpret_cast<T>(dl_sym(g_lib, name));
  if (!fn) {
    std::fprintf(stderr, "FATAL: core missing symbol %s\n", name);
    std::exit(2);
  }
}

// --- scripted, purely-frame-derived input (so it is reproducible across passes) ---
uint32_t g_frame = 0;
uint32_t g_seed = 1;

// Only Game Boy joypad ids: B(0) SELECT(2) START(3) UP(4) DOWN(5) LEFT(6)
// RIGHT(7) A(8). Bitmask 0x1FD selects exactly those bit positions.
uint16_t frame_buttons(uint32_t frame) {
  uint32_t x = frame * 2654435761u + g_seed * 40503u + 12345u;
  x ^= x >> 13;
  x *= 1274126177u;
  x ^= x >> 16;
  return static_cast<uint16_t>(x & 0x1FDu);
}

// --- libretro callbacks (all side-effect-free w.r.t. timing) ---
bool env_cb(unsigned cmd, void *data) {
  switch (cmd) {
  case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
    return true; // accept whatever the core wants
  case RETRO_ENVIRONMENT_GET_CAN_DUPE:
    *static_cast<bool *>(data) = true;
    return true;
  case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
  case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
    *static_cast<const char **>(data) = ".";
    return true;
  case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS: {
    uint64_t *q = static_cast<uint64_t *>(data);
    std::printf("[env] core serialization quirks = 0x%llx%s\n",
                static_cast<unsigned long long>(*q),
                (*q & RETRO_SERIALIZATION_QUIRK_INCOMPLETE)
                    ? "  <-- WARNING: INCOMPLETE (savestates not frame-reliable)"
                    : "");
    return true;
  }
  default:
    return false; // use core defaults for everything else
  }
}
void video_cb(const void *, unsigned, unsigned, size_t) {}
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

void wire() {
  g.set_environment(env_cb);
  g.set_video_refresh(video_cb);
  g.set_audio_sample(audio_cb);
  g.set_audio_sample_batch(audio_batch_cb);
  g.set_input_poll(input_poll_cb);
  g.set_input_state(input_state_cb);
}

uint64_t fnv1a(const void *p, size_t n) {
  const uint8_t *b = static_cast<const uint8_t *>(p);
  uint64_t h = 1469598103934665603ull;
  for (size_t i = 0; i < n; ++i) {
    h ^= b[i];
    h *= 1099511628211ull;
  }
  return h;
}

struct Sample {
  uint32_t frame;
  uint64_t hash;
};

std::vector<uint8_t> g_rom;
const char *g_romPath = nullptr;

bool loadGame() {
  retro_game_info gi{};
  gi.path = g_romPath;
  gi.data = g_rom.data();
  gi.size = g_rom.size();
  gi.meta = nullptr;
  return g.load_game(&gi);
}

} // namespace

int main(int argc, char **argv) {
  if (argc < 3) {
    std::fprintf(stderr,
                 "usage: %s <core.(dll|so)> <rom> [frames=4000] [seed=1]\n",
                 argv[0]);
    return 1;
  }
  const char *corePath = argv[1];
  g_romPath = argv[2];
  const uint32_t frames = argc > 3 ? static_cast<uint32_t>(std::atoi(argv[3])) : 4000;
  g_seed = argc > 4 ? static_cast<uint32_t>(std::atoi(argv[4])) : 1;
  const uint32_t every = 100;
  const uint32_t saveAt = frames / 2;

  g_lib = dl_open(corePath);
  if (!g_lib) {
    std::fprintf(stderr, "FATAL: cannot load core '%s'\n", corePath);
    return 2;
  }
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

  {
    std::ifstream f(g_romPath, std::ios::binary);
    if (!f) {
      std::fprintf(stderr, "FATAL: cannot open rom '%s'\n", g_romPath);
      return 2;
    }
    g_rom.assign(std::istreambuf_iterator<char>(f),
                 std::istreambuf_iterator<char>());
  }

  retro_system_info si{};
  g.get_system_info(&si);
  std::printf("[core] %s  version=%s\n", si.library_name ? si.library_name : "?",
              si.library_version ? si.library_version : "?");
  std::printf("[rom]  %s  (%zu bytes)\n", g_romPath, g_rom.size());
  std::printf("[cfg]  frames=%u  sample-every=%u  savestate-at=%u  seed=%u\n\n",
              frames, every, saveAt, g_seed);

  wire();
  g.init();

  const size_t ssize = g.serialize_size();
  if (ssize == 0) {
    std::fprintf(stderr, "FATAL: core reports serialize_size()==0\n");
    return 3;
  }
  std::vector<uint8_t> buf(ssize);
  std::vector<uint8_t> saveBlob;

  auto shouldSample = [&](uint32_t fr) {
    return (fr % every) == 0 || fr == frames - 1;
  };

  // --- Pass A: power-on, collect hashes, capture savestate at saveAt ---
  std::vector<Sample> A;
  if (!loadGame()) {
    std::fprintf(stderr, "FATAL: load_game failed (pass A)\n");
    return 3;
  }
  for (g_frame = 0; g_frame < frames; ++g_frame) {
    g.run();
    if (shouldSample(g_frame)) {
      g.serialize(buf.data(), ssize);
      A.push_back({g_frame, fnv1a(buf.data(), ssize)});
    }
    if (g_frame == saveAt) {
      g.serialize(buf.data(), ssize);
      saveBlob.assign(buf.begin(), buf.end());
    }
  }
  g.unload_game();

  // --- Pass B: power-on again, same inputs; must equal A ---
  std::vector<Sample> B;
  if (!loadGame()) {
    std::fprintf(stderr, "FATAL: load_game failed (pass B)\n");
    return 3;
  }
  for (g_frame = 0; g_frame < frames; ++g_frame) {
    g.run();
    if (shouldSample(g_frame)) {
      g.serialize(buf.data(), ssize);
      B.push_back({g_frame, fnv1a(buf.data(), ssize)});
    }
  }
  g.unload_game();

  // --- Pass C: restore savestate at saveAt, run forward; must equal A's tail ---
  std::vector<Sample> C;
  if (!loadGame()) {
    std::fprintf(stderr, "FATAL: load_game failed (pass C)\n");
    return 3;
  }
  if (!g.unserialize(saveBlob.data(), saveBlob.size())) {
    std::fprintf(stderr, "FATAL: unserialize failed (pass C)\n");
    return 3;
  }
  for (g_frame = saveAt + 1; g_frame < frames; ++g_frame) {
    g.run();
    if (shouldSample(g_frame)) {
      g.serialize(buf.data(), ssize);
      C.push_back({g_frame, fnv1a(buf.data(), ssize)});
    }
  }
  g.unload_game();
  g.deinit();

  // --- Compare ---
  int failures = 0;

  // A vs B (power-on determinism)
  uint32_t abMismatch = 0;
  for (size_t i = 0; i < A.size(); ++i) {
    if (i >= B.size() || A[i].hash != B[i].hash) {
      if (abMismatch == 0)
        std::printf("  A/B first mismatch at frame %u\n", A[i].frame);
      ++abMismatch;
    }
  }
  std::printf("[A] power-on determinism : %s  (%zu samples, %u mismatches)\n",
              abMismatch == 0 ? "PASS" : "FAIL", A.size(), abMismatch);
  failures += (abMismatch != 0);

  // A tail vs C (savestate determinism)
  uint32_t acMismatch = 0;
  size_t ci = 0;
  for (size_t i = 0; i < A.size(); ++i) {
    if (A[i].frame <= saveAt)
      continue;
    if (ci >= C.size() || A[i].frame != C[ci].frame || A[i].hash != C[ci].hash) {
      if (acMismatch == 0)
        std::printf("  savestate first mismatch at/near frame %u\n", A[i].frame);
      ++acMismatch;
    }
    ++ci;
  }
  std::printf("[C] savestate determinism: %s  (%zu samples, %u mismatches)\n",
              acMismatch == 0 ? "PASS" : "FAIL", C.size(), acMismatch);
  failures += (acMismatch != 0);

  std::printf("\n%s: gambatte-libretro is %sinternally deterministic on this ROM.\n",
              failures == 0 ? "GATE 1a PASS" : "GATE 1a FAIL",
              failures == 0 ? "" : "NOT ");
  if (failures == 0)
    std::printf("=> Re-recording, RNG-search, and playback verification are sound "
                "on the stock core.\n");
  return failures == 0 ? 0 : 4;
}
