// tools/tas/tas_movie.cpp
//
// Headless TAS movie engine for Firelight, driving a libretro core directly
// (mGBA for Game Boy -- gate 1a showed stock gambatte v0.5.0 savestates are
// unreliable; mGBA passes fully). No Qt dependency. Subcommands:
//
//   gen     <core> <rom> <out.fltm> [frames=4000] [seed=1]
//             Scripted-input movie (pipeline exercise) with embedded checkpoints.
//   compile <core> <rom> <script.txt> <out.fltm>
//             Author a movie from a human-readable input script (see below),
//             playing it through to stamp the header + embed checkpoints.
//   play    <core> <rom> <movie.fltm>
//             Deterministic replay; report checkpoint matches.
//   verify  <core> <rom> <movie.fltm>
//             Replay twice (determinism) AND check checkpoints (sync). Exit 0 iff ok.
//   shot    <core> <rom> <movie.fltm> <frame> <out.ppm>
//             Replay to <frame> and write the framebuffer as a binary PPM (P6).
//   ram     <core> <rom> <movie.fltm> <frame> <hexAddr> <len>
//             Replay to <frame> and hexdump <len> bytes of system RAM at <hexAddr>.
//
// Input-script format (one directive per line; '#' starts a comment):
//   <frameCount> <buttons>
//   buttons: '-' (none) or '+'-joined names: A B START SELECT UP DOWN LEFT RIGHT
//   e.g.   60 -          # wait 60 frames
//          2  START      # press Start for 2 frames
//          16 RIGHT      # walk right
//          2  RIGHT+A    # right + A
//
// Determinism is measured on the VIDEO framebuffer. Build via tools/tas/build.sh.

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "fltm.hpp"
#include "libretro.h"
#include "yellow_ram.hpp"

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
  void *(*get_memory_data)(unsigned);
  size_t (*get_memory_size)(unsigned);
  size_t (*serialize_size)();
  bool (*serialize)(void *, size_t);
  bool (*unserialize)(const void *, size_t);
};

Core g;
dl_t g_lib = nullptr;
uint32_t g_frame = 0;
uint64_t g_frameHash = 0;
const Movie *g_movie = nullptr;
const std::vector<uint8_t> *g_rom = nullptr;
const char *g_romPath = nullptr;

// video format (captured from SET_PIXEL_FORMAT; libretro default is 0RGB1555)
retro_pixel_format g_pixfmt = RETRO_PIXEL_FORMAT_0RGB1555;
int g_bpp = 2;

// framebuffer capture (only when g_wantFB, for `shot`)
bool g_wantFB = false;
unsigned g_fbW = 0, g_fbH = 0;
std::vector<uint8_t> g_fbData; // tightly packed g_fbW*g_fbH*g_bpp

// Memory-map descriptors captured from SET_MEMORY_MAPS -- lets us read arbitrary
// emulated addresses (e.g. HRAM RNG bytes that are NOT in SYSTEM_RAM).
struct MemDesc {
  uint64_t flags;
  uint8_t *ptr;
  size_t offset, start, select, disconnect, len;
  std::string space;
};
std::vector<MemDesc> g_memmap;

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
  case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
    g_pixfmt = *static_cast<const retro_pixel_format *>(data);
    g_bpp = (g_pixfmt == RETRO_PIXEL_FORMAT_XRGB8888) ? 4 : 2;
    return true;
  }
  case RETRO_ENVIRONMENT_GET_CAN_DUPE:
    *static_cast<bool *>(data) = true;
    return true;
  case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
  case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
    *static_cast<const char **>(data) = ".";
    return true;
  case RETRO_ENVIRONMENT_SET_MEMORY_MAPS: {
    const auto *mm = static_cast<const retro_memory_map *>(data);
    g_memmap.clear();
    for (unsigned i = 0; i < mm->num_descriptors; ++i) {
      const auto &d = mm->descriptors[i];
      g_memmap.push_back({d.flags, static_cast<uint8_t *>(d.ptr), d.offset,
                          d.start, d.select, d.disconnect, d.len,
                          d.addrspace ? d.addrspace : ""});
    }
    return true;
  }
  default:
    return false;
  }
}
void video_cb(const void *data, unsigned width, unsigned height, size_t pitch) {
  if (!data)
    return;
  const uint8_t *p = static_cast<const uint8_t *>(data);
  const size_t rowBytes = static_cast<size_t>(width) * g_bpp;
  uint64_t h = 1469598103934665603ull;
  for (unsigned y = 0; y < height; ++y)
    h = fnv1a(p + static_cast<size_t>(y) * pitch, rowBytes, h);
  g_frameHash = h;
  if (g_wantFB) {
    g_fbW = width;
    g_fbH = height;
    g_fbData.resize(rowBytes * height);
    for (unsigned y = 0; y < height; ++y)
      std::memcpy(&g_fbData[static_cast<size_t>(y) * rowBytes],
                  p + static_cast<size_t>(y) * pitch, rowBytes);
  }
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
  bind(g.get_memory_data, "retro_get_memory_data");
  bind(g.get_memory_size, "retro_get_memory_size");
  bind(g.serialize_size, "retro_serialize_size");
  bind(g.serialize, "retro_serialize");
  bind(g.unserialize, "retro_unserialize");
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
  // Savestate-anchored movies: restore the embedded start state right after
  // load_game, so the input log plays from there instead of from power-on.
  if (g_movie && g_movie->startMode == 1 && !g_movie->startState.empty()) {
    if (!g.unserialize(g_movie->startState.data(), g_movie->startState.size()))
      std::fprintf(stderr, "WARNING: failed to restore start savestate\n");
  }
}

void closeCore() {
  g.unload_game();
  g.deinit();
  dl_close(g_lib);
  g_lib = nullptr;
}

// Replay the current g_movie on a fresh core; run to (and including) `toFrame`
// (or the whole movie if toFrame==UINT32_MAX). Samples checkpoints into `out`
// every `every` frames when `out` is non-null. Leaves the core CLOSED unless
// `keepOpen`.
void replayTo(const char *corePath, uint32_t toFrame,
              std::vector<std::pair<uint32_t, uint64_t>> *out, uint32_t every) {
  openCore(corePath);
  const uint32_t n = static_cast<uint32_t>(g_movie->input.size());
  const uint32_t last = (toFrame == UINT32_MAX) ? n : std::min(toFrame + 1, n);
  for (g_frame = 0; g_frame < last; ++g_frame) {
    g.run();
    if (out && ((g_frame % every) == 0 || g_frame == n - 1))
      out->push_back({g_frame, g_frameHash});
  }
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

std::string upper(std::string s) {
  for (char &c : s)
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  return s;
}

// Button name -> RETRO_DEVICE_ID_JOYPAD_*, or -1.
int buttonBit(const std::string &raw) {
  const std::string n = upper(raw);
  if (n == "A") return RETRO_DEVICE_ID_JOYPAD_A;
  if (n == "B") return RETRO_DEVICE_ID_JOYPAD_B;
  if (n == "START") return RETRO_DEVICE_ID_JOYPAD_START;
  if (n == "SELECT") return RETRO_DEVICE_ID_JOYPAD_SELECT;
  if (n == "UP") return RETRO_DEVICE_ID_JOYPAD_UP;
  if (n == "DOWN") return RETRO_DEVICE_ID_JOYPAD_DOWN;
  if (n == "LEFT") return RETRO_DEVICE_ID_JOYPAD_LEFT;
  if (n == "RIGHT") return RETRO_DEVICE_ID_JOYPAD_RIGHT;
  return -1;
}

// Parse an input script into a per-frame input log. Returns false + err on error.
bool parseScript(const char *path, std::vector<InputFrame> &out,
                 std::string &err) {
  std::ifstream f(path);
  if (!f) {
    err = "cannot open script";
    return false;
  }
  std::string line;
  int lineNo = 0;
  while (std::getline(f, line)) {
    ++lineNo;
    if (const auto h = line.find('#'); h != std::string::npos)
      line.erase(h);
    std::istringstream is(line);
    long count;
    if (!(is >> count)) // blank / comment-only line
      continue;
    if (count < 0) {
      err = "line " + std::to_string(lineNo) + ": negative frame count";
      return false;
    }
    InputFrame fr;
    std::string tok;
    while (is >> tok) {
      if (tok == "-" || upper(tok) == "NONE")
        continue;
      std::stringstream ts(tok);
      std::string part;
      while (std::getline(ts, part, '+')) {
        if (part.empty())
          continue;
        const int bit = buttonBit(part);
        if (bit < 0) {
          err = "line " + std::to_string(lineNo) + ": unknown button '" + part +
                "'";
          return false;
        }
        fr.set(static_cast<unsigned>(bit), true);
      }
    }
    for (long i = 0; i < count; ++i)
      out.push_back(fr);
  }
  return true;
}

bool writePPM(const char *path) {
  if (g_fbData.empty()) {
    std::fprintf(stderr, "FATAL: no framebuffer captured\n");
    return false;
  }
  std::ofstream o(path, std::ios::binary);
  if (!o)
    return false;
  o << "P6\n" << g_fbW << " " << g_fbH << "\n255\n";
  const auto pixel = [&](unsigned i, uint8_t &r, uint8_t &gc, uint8_t &b) {
    if (g_bpp == 4) { // XRGB8888, little-endian bytes B,G,R,X
      const uint8_t *px = &g_fbData[static_cast<size_t>(i) * 4];
      b = px[0];
      gc = px[1];
      r = px[2];
    } else {
      const uint16_t px = g_fbData[static_cast<size_t>(i) * 2] |
                          (g_fbData[static_cast<size_t>(i) * 2 + 1] << 8);
      if (g_pixfmt == RETRO_PIXEL_FORMAT_RGB565) {
        uint8_t r5 = (px >> 11) & 31, g6 = (px >> 5) & 63, b5 = px & 31;
        r = (r5 << 3) | (r5 >> 2);
        gc = (g6 << 2) | (g6 >> 4);
        b = (b5 << 3) | (b5 >> 2);
      } else { // 0RGB1555
        uint8_t r5 = (px >> 10) & 31, g5 = (px >> 5) & 31, b5 = px & 31;
        r = (r5 << 3) | (r5 >> 2);
        gc = (g5 << 3) | (g5 >> 2);
        b = (b5 << 3) | (b5 >> 2);
      }
    }
  };
  for (unsigned i = 0; i < g_fbW * g_fbH; ++i) {
    uint8_t r, gc, b;
    pixel(i, r, gc, b);
    o.put(static_cast<char>(r));
    o.put(static_cast<char>(gc));
    o.put(static_cast<char>(b));
  }
  return static_cast<bool>(o);
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

// Play `m` through a fresh core, stamping core identity + checkpoints into it.
void stampAndCheckpoint(const char *corePath, Movie &m, uint32_t every) {
  g_movie = &m;
  openCore(corePath);
  retro_system_info si{};
  g.get_system_info(&si);
  m.coreName = si.library_name ? si.library_name : "?";
  m.coreVersion = si.library_version ? si.library_version : "?";
  const uint32_t n = static_cast<uint32_t>(m.input.size());
  m.checkpoints.clear();
  for (g_frame = 0; g_frame < n; ++g_frame) {
    g.run();
    if ((g_frame % every) == 0 || g_frame == n - 1)
      m.checkpoints.push_back({g_frame, g_frameHash});
  }
  closeCore();
}

uint16_t scriptedButtons(uint32_t frame, uint32_t seed) {
  uint32_t x = frame * 2654435761u + seed * 40503u + 12345u;
  x ^= x >> 13;
  x *= 1274126177u;
  x ^= x >> 16;
  return static_cast<uint16_t>(x & 0x1FDu);
}

// Read one byte at emulated GB address `addr` via the memory-map descriptors
// (WRAM banks, I/O incl. rDIV, HRAM incl. the RNG bytes). -1 if unmapped.
int readGB(uint32_t addr) {
  for (const auto &d : g_memmap) {
    if (!d.ptr)
      continue;
    size_t rel;
    if (d.select == 0) {
      if (addr < d.start || addr >= d.start + d.len)
        continue;
      rel = addr - d.start;
    } else {
      if ((addr & d.select) != (d.start & d.select))
        continue;
      rel = addr & ~d.select;
      if (d.len && rel >= d.len)
        continue;
    }
    return d.ptr[d.offset + rel];
  }
  return -1;
}

// Read `size` bytes big-endian at GB address `addr` (via readGB).
uint32_t readN(uint32_t addr, uint32_t size) {
  uint32_t v = 0;
  for (uint32_t i = 0; i < size; ++i) {
    const int b = readGB(addr + i);
    v = (v << 8) | (b < 0 ? 0u : static_cast<uint32_t>(b));
  }
  return v;
}

bool compareOp(uint32_t a, const std::string &op, uint32_t b) {
  if (op == "==") return a == b;
  if (op == "!=") return a != b;
  if (op == "<") return a < b;
  if (op == ">") return a > b;
  if (op == "<=") return a <= b;
  if (op == ">=") return a >= b;
  return false;
}

// A route = an ordered plan an LLM planner emits: input segments, RAM assertions
// (verify a segment reached the intended state), and search slots (insert idle
// frames until a target RAM/RNG condition holds -- luck manipulation). Directives
// in the route file: '@name <text>', '@assert <addr> <op> <val> [size=N]',
// '@search <addr> <op> <val> [size=N] [maxdelay=N]'; other lines are input-script
// lines ('<frameCount> <buttons>').
struct RouteOp {
  enum Kind { INPUT, ASSERT, SEARCH, NAME } kind;
  uint32_t count = 0;
  uint16_t buttons = 0;
  uint16_t addr = 0;
  std::string op;
  uint32_t val = 0, size = 1, maxDelay = 30;
  std::string name;
};

bool parseRoute(const char *path, std::vector<RouteOp> &out, std::string &err) {
  std::ifstream f(path);
  if (!f) {
    err = "cannot open route file";
    return false;
  }
  std::string line;
  int ln = 0;
  while (std::getline(f, line)) {
    ++ln;
    if (const auto h = line.find('#'); h != std::string::npos)
      line.erase(h);
    std::istringstream is(line);
    std::string tok;
    if (!(is >> tok))
      continue;
    if (tok == "@name") {
      RouteOp o;
      o.kind = RouteOp::NAME;
      std::getline(is, o.name);
      out.push_back(o);
    } else if (tok == "@assert" || tok == "@search") {
      RouteOp o;
      o.kind = (tok == "@assert") ? RouteOp::ASSERT : RouteOp::SEARCH;
      std::string a, op, v;
      if (!(is >> a >> op >> v)) {
        err = "line " + std::to_string(ln) +
              ": @assert/@search needs <gbAddr> <op> <value>";
        return false;
      }
      o.addr = static_cast<uint16_t>(std::strtoul(a.c_str(), nullptr, 0));
      o.op = op;
      o.val = std::strtoul(v.c_str(), nullptr, 0);
      std::string kv;
      while (is >> kv) {
        const auto eq = kv.find('=');
        if (eq == std::string::npos)
          continue;
        const uint32_t n = std::strtoul(kv.c_str() + eq + 1, nullptr, 0);
        if (kv.compare(0, eq, "size") == 0)
          o.size = n;
        else if (kv.compare(0, eq, "maxdelay") == 0)
          o.maxDelay = n;
      }
      out.push_back(o);
    } else {
      RouteOp o;
      o.kind = RouteOp::INPUT;
      o.count = std::strtoul(tok.c_str(), nullptr, 0);
      InputFrame fr;
      std::string b;
      while (is >> b) {
        if (b == "-" || upper(b) == "NONE")
          continue;
        std::stringstream bs(b);
        std::string part;
        while (std::getline(bs, part, '+')) {
          if (part.empty())
            continue;
          const int bit = buttonBit(part);
          if (bit < 0) {
            err = "line " + std::to_string(ln) + ": unknown button '" + part + "'";
            return false;
          }
          fr.set(static_cast<unsigned>(bit), true);
        }
      }
      o.buttons = fr.buttons;
      out.push_back(o);
    }
  }
  return true;
}

int usage(const char *a0) {
  std::fprintf(stderr,
               "usage:\n"
               "  %s gen     <core> <rom> <out.fltm> [frames=4000] [seed=1]\n"
               "  %s compile <core> <rom> <script.txt> <out.fltm> [anchor.fltm]\n"
               "  %s savestate <core> <rom> <in.fltm> <atFrame> <out.fltm>\n"
               "  %s play    <core> <rom> <movie.fltm>\n"
               "  %s verify  <core> <rom> <movie.fltm>\n"
               "  %s shot    <core> <rom> <movie.fltm> <frame> <out.ppm>\n"
               "  %s ram     <core> <rom> <movie.fltm> <frame> <hexAddr> <len>\n"
               "  %s read    <core> <rom> <movie.fltm> <frame> <gbAddr> <len>\n"
               "  %s sweep   <core> <rom> <movie.fltm> <atFrame> <maxDelay> <gbAddr> [len=2]\n"
               "  %s watch   <core> <rom> <movie.fltm> <frame>   (decode Pokemon Yellow state)\n"
               "  %s route   <core> <rom> <route.txt> <out.fltm> [anchor.fltm]\n"
               "  %s dump    <core> <rom> <movie.fltm> <outdir> [everyN=2] [from] [to]\n"
               "  %s maps    <core> <rom> _\n",
               a0, a0, a0, a0, a0, a0, a0, a0, a0, a0, a0, a0, a0);
  return 1;
}

} // namespace

int main(int argc, char **argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  if (argc < 5)
    return usage(argv[0]);

  const std::string cmd = argv[1];
  const char *corePath = argv[2];
  g_romPath = argv[3];
  const uint32_t every = 100;

  const std::vector<uint8_t> rom = readFile(g_romPath);
  if (rom.empty()) {
    std::fprintf(stderr, "FATAL: cannot read rom '%s'\n", g_romPath);
    return 2;
  }
  g_rom = &rom;
  const uint64_t romHash = fnv1a(rom.data(), rom.size());

  // ---- gen ----
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
    stampAndCheckpoint(corePath, m, every);
    if (!m.save(argv[4])) {
      std::fprintf(stderr, "FATAL: cannot write '%s'\n", argv[4]);
      return 3;
    }
    std::printf("wrote %s\n  core=%s %s  frames=%u  checkpoints=%zu  seed=%u\n",
                argv[4], m.coreName.c_str(), m.coreVersion.c_str(), frames,
                m.checkpoints.size(), seed);
    return 0;
  }

  // ---- savestate: capture a savestate anchor from <in.fltm> at <atFrame> into a
  // new <out.fltm> (empty input, startMode=savestate) for anchored authoring. ----
  if (cmd == "savestate") {
    if (argc < 7)
      return usage(argv[0]);
    Movie in;
    if (!in.load(argv[4])) {
      std::fprintf(stderr, "FATAL: cannot load movie '%s'\n", argv[4]);
      return 2;
    }
    const uint32_t atFrame = std::strtoul(argv[5], nullptr, 0);
    g_movie = &in;
    replayTo(corePath, atFrame, nullptr, every); // leaves core open at atFrame
    const size_t ss = g.serialize_size();
    std::vector<uint8_t> blob(ss);
    if (!g.serialize(blob.data(), ss)) {
      std::fprintf(stderr, "FATAL: serialize failed\n");
      return 3;
    }
    closeCore();
    Movie out;
    out.coreName = in.coreName;
    out.coreVersion = in.coreVersion;
    out.romName = in.romName.empty() ? baseName(g_romPath) : in.romName;
    out.romHash = romHash;
    out.romSize = static_cast<uint32_t>(rom.size());
    out.startMode = 1;
    out.startState = std::move(blob);
    if (!out.save(argv[6])) {
      std::fprintf(stderr, "FATAL: cannot write '%s'\n", argv[6]);
      return 3;
    }
    std::printf("savestate anchor -> %s : from %s @frame %u, %zu-byte state\n",
                argv[6], argv[4], atFrame, out.startState.size());
    return 0;
  }

  // ---- compile ----
  if (cmd == "compile") {
    if (argc < 6)
      return usage(argv[0]);
    const char *scriptPath = argv[4];
    const char *outPath = argv[5];
    Movie m;
    std::string err;
    if (!parseScript(scriptPath, m.input, err)) {
      std::fprintf(stderr, "FATAL: %s\n", err.c_str());
      return 2;
    }
    m.romName = baseName(g_romPath);
    m.romHash = romHash;
    m.romSize = static_cast<uint32_t>(rom.size());
    if (argc > 6) { // optional savestate anchor to author from
      Movie base;
      if (!base.load(argv[6])) {
        std::fprintf(stderr, "FATAL: cannot load base '%s'\n", argv[6]);
        return 2;
      }
      m.startMode = base.startMode;
      m.startState = base.startState;
    }
    stampAndCheckpoint(corePath, m, every);
    if (!m.save(outPath)) {
      std::fprintf(stderr, "FATAL: cannot write '%s'\n", outPath);
      return 3;
    }
    std::printf("compiled %s -> %s\n  core=%s %s  frames=%zu  checkpoints=%zu\n",
                scriptPath, outPath, m.coreName.c_str(), m.coreVersion.c_str(),
                m.input.size(), m.checkpoints.size());
    return 0;
  }

  // ---- route: execute an ordered route plan (input segments + RAM assertions
  // + idle-frame search slots) into a verified movie. An LLM planner emits the
  // plan; this grinds the searches and checks the assertions. ----
  if (cmd == "route") {
    if (argc < 6)
      return usage(argv[0]);
    std::vector<RouteOp> ops;
    std::string err;
    if (!parseRoute(argv[4], ops, err)) {
      std::fprintf(stderr, "FATAL: %s\n", err.c_str());
      return 2;
    }
    Movie m;
    m.romName = baseName(g_romPath);
    m.romHash = romHash;
    m.romSize = static_cast<uint32_t>(rom.size());
    if (argc > 6) { // optional savestate anchor to author from
      Movie base;
      if (!base.load(argv[6])) {
        std::fprintf(stderr, "FATAL: cannot load base '%s'\n", argv[6]);
        return 2;
      }
      m.startMode = base.startMode;
      m.startState = base.startState;
    }
    g_movie = &m;
    openCore(corePath);
    static Movie idle;
    int failures = 0;
    const auto runLast = [&]() {
      g_frame = static_cast<uint32_t>(m.input.size()) - 1;
      g.run();
    };
    for (const auto &o : ops) {
      if (o.kind == RouteOp::NAME) {
        std::printf("== %s ==\n", o.name.c_str());
      } else if (o.kind == RouteOp::INPUT) {
        InputFrame fr;
        fr.buttons = o.buttons;
        for (uint32_t i = 0; i < o.count; ++i) {
          m.input.push_back(fr);
          runLast();
        }
      } else if (o.kind == RouteOp::ASSERT) {
        const uint32_t v = readN(o.addr, o.size);
        const bool ok = compareOp(v, o.op, o.val);
        std::printf("  assert 0x%04x %s 0x%x : %s (got 0x%x) @frame %zu\n", o.addr,
                    o.op.c_str(), o.val, ok ? "PASS" : "FAIL", v, m.input.size());
        failures += !ok;
      } else { // SEARCH
        const size_t ss = g.serialize_size();
        std::vector<uint8_t> snap(ss);
        g.serialize(snap.data(), ss);
        int found = -1;
        for (uint32_t d = 0; d <= o.maxDelay; ++d) {
          g.unserialize(snap.data(), ss);
          const Movie *saved = g_movie;
          g_movie = &idle;
          for (uint32_t k = 0; k < d; ++k) {
            g_frame = k;
            g.run();
          }
          g_movie = saved;
          if (compareOp(readN(o.addr, o.size), o.op, o.val)) {
            found = static_cast<int>(d);
            break;
          }
        }
        g.unserialize(snap.data(), ss);
        if (found < 0) {
          std::printf("  search 0x%04x %s 0x%x : FAIL (no delay in 0..%u)\n",
                      o.addr, o.op.c_str(), o.val, o.maxDelay);
          ++failures;
        } else {
          InputFrame blank;
          for (int k = 0; k < found; ++k) {
            m.input.push_back(blank);
            runLast();
          }
          std::printf("  search 0x%04x %s 0x%x : delay=%d (+%d idle frames) "
                      "@frame %zu\n",
                      o.addr, o.op.c_str(), o.val, found, found, m.input.size());
        }
      }
    }
    closeCore();
    stampAndCheckpoint(corePath, m, every); // re-stamp + embed checkpoints
    if (!m.save(argv[5])) {
      std::fprintf(stderr, "FATAL: cannot write '%s'\n", argv[5]);
      return 3;
    }
    std::printf("\nroute -> %s : %zu frames, %d failure(s)\n", argv[5],
                m.input.size(), failures);
    return failures == 0 ? 0 : 4;
  }

  // ---- maps: print the core's memory-map descriptors (no movie needed) ----
  if (cmd == "maps") {
    static Movie empty;
    g_movie = &empty;
    openCore(corePath);
    const size_t sysRam = g.get_memory_size(RETRO_MEMORY_SYSTEM_RAM);
    closeCore();
    std::printf("SYSTEM_RAM size = %zu bytes (0x%zx)\n", sysRam, sysRam);
    std::printf("memory-map descriptors: %zu\n", g_memmap.size());
    for (const auto &d : g_memmap)
      std::printf("  start=0x%05zx len=0x%05zx offset=0x%05zx select=0x%05zx "
                  "flags=0x%llx space='%s'\n",
                  d.start, d.len, d.offset, d.select,
                  static_cast<unsigned long long>(d.flags), d.space.c_str());
    return 0;
  }

  // ---- everything else loads a movie ----
  Movie m;
  if (!m.load(argv[4])) {
    std::fprintf(stderr, "FATAL: cannot load movie '%s'\n", argv[4]);
    return 2;
  }
  g_movie = &m;
  std::printf("[movie] %s  core=%s %s  frames=%zu  checkpoints=%zu  rom=%s\n",
              argv[4], m.coreName.c_str(), m.coreVersion.c_str(), m.input.size(),
              m.checkpoints.size(), m.romName.c_str());
  if (m.romHash != romHash)
    std::printf("  WARNING: ROM hash mismatch (movie made for a different ROM)\n");

  if (cmd == "play") {
    std::vector<std::pair<uint32_t, uint64_t>> got;
    replayTo(corePath, UINT32_MAX, &got, every);
    closeCore();
    std::printf("replayed %zu frames; final framebuffer hash = 0x%016llx\n",
                m.input.size(),
                static_cast<unsigned long long>(got.empty() ? 0
                                                            : got.back().second));
    if (!m.checkpoints.empty()) {
      uint32_t bad = 0;
      std::printf(cmpCheckpoints(got, m.checkpoints, bad)
                      ? "checkpoints: all match\n"
                      : "checkpoints: MISMATCH near frame %u\n",
                  bad);
    }
    return 0;
  }

  if (cmd == "verify") {
    std::vector<std::pair<uint32_t, uint64_t>> a, b;
    replayTo(corePath, UINT32_MAX, &a, every);
    closeCore();
    replayTo(corePath, UINT32_MAX, &b, every);
    closeCore();
    const bool det = (a == b);
    uint32_t bad = 0;
    const bool cp =
        m.checkpoints.empty() ? true : cmpCheckpoints(a, m.checkpoints, bad);
    std::printf("\ndeterminism (replay x2) : %s\n", det ? "PASS" : "FAIL");
    std::printf("checkpoint match        : %s\n",
                m.checkpoints.empty() ? "(none)" : (cp ? "PASS" : "FAIL"));
    const bool ok = det && cp;
    std::printf("\n%s\n", ok ? "MOVIE VERIFIED" : "MOVIE FAILED VERIFICATION");
    return ok ? 0 : 4;
  }

  if (cmd == "shot") {
    if (argc < 7)
      return usage(argv[0]);
    const uint32_t frame = std::strtoul(argv[5], nullptr, 0);
    g_wantFB = true;
    replayTo(corePath, frame, nullptr, every);
    closeCore();
    if (!writePPM(argv[6]))
      return 3;
    std::printf("wrote %s  (%ux%u, frame %u, fmt=%s)\n", argv[6], g_fbW, g_fbH,
                frame,
                g_bpp == 4 ? "XRGB8888"
                           : (g_pixfmt == RETRO_PIXEL_FORMAT_RGB565 ? "RGB565"
                                                                    : "0RGB1555"));
    return 0;
  }

  if (cmd == "ram") {
    if (argc < 8)
      return usage(argv[0]);
    const uint32_t frame = std::strtoul(argv[5], nullptr, 0);
    const uint32_t addr = std::strtoul(argv[6], nullptr, 0);
    const uint32_t len = std::strtoul(argv[7], nullptr, 0);
    replayTo(corePath, frame, nullptr, every);
    const auto *ram = static_cast<const uint8_t *>(
        g.get_memory_data(RETRO_MEMORY_SYSTEM_RAM));
    const size_t ramSize = g.get_memory_size(RETRO_MEMORY_SYSTEM_RAM);
    std::printf("system RAM: %zu bytes; dump @0x%04x len %u (frame %u):\n",
                ramSize, addr, len, frame);
    for (uint32_t i = 0; i < len; ++i) {
      if (addr + i >= ramSize) {
        std::printf(" <oob>");
        break;
      }
      if (i % 16 == 0)
        std::printf("\n  %04x:", addr + i);
      std::printf(" %02x", ram[addr + i]);
    }
    std::printf("\n");
    closeCore();
    return 0;
  }

  if (cmd == "read") {
    if (argc < 8)
      return usage(argv[0]);
    const uint32_t frame = std::strtoul(argv[5], nullptr, 0);
    const uint32_t addr = std::strtoul(argv[6], nullptr, 0);
    const uint32_t len = std::strtoul(argv[7], nullptr, 0);
    replayTo(corePath, frame, nullptr, every);
    std::printf("GB memory @0x%04x len %u (frame %u):", addr, len, frame);
    for (uint32_t i = 0; i < len; ++i) {
      const int v = readGB(addr + i);
      if (i % 16 == 0)
        std::printf("\n  %04x:", addr + i);
      if (v < 0)
        std::printf(" --");
      else
        std::printf(" %02x", v);
    }
    std::printf("\n");
    closeCore();
    return 0;
  }

  // ---- sweep: the luck-manipulation lever. Savestate at <atFrame>, then for
  // each idle-frame delay 0..maxDelay, restore and insert that many blank frames
  // and read <gbAddr>. Shows how a target (e.g. the RNG bytes) shifts with timing
  // -- the search would pick the delay that yields the wanted value. ----
  if (cmd == "sweep") {
    if (argc < 8)
      return usage(argv[0]);
    const uint32_t atFrame = std::strtoul(argv[5], nullptr, 0);
    const uint32_t maxDelay = std::strtoul(argv[6], nullptr, 0);
    const uint32_t addr = std::strtoul(argv[7], nullptr, 0);
    const uint32_t len = argc > 8 ? std::strtoul(argv[8], nullptr, 0) : 2;
    replayTo(corePath, atFrame, nullptr, every);
    const size_t ss = g.serialize_size();
    std::vector<uint8_t> snap(ss);
    g.serialize(snap.data(), ss);
    static Movie idle; // no input during the inserted idle frames
    std::printf("sweep at frame %u; addr 0x%04x; idle-frame delay 0..%u:\n",
                atFrame, addr, maxDelay);
    for (uint32_t delay = 0; delay <= maxDelay; ++delay) {
      g.unserialize(snap.data(), ss);
      const Movie *saved = g_movie;
      g_movie = &idle;
      for (uint32_t k = 0; k < delay; ++k) {
        g_frame = k;
        g.run();
      }
      g_movie = saved;
      std::printf("  delay=%3u:", delay);
      for (uint32_t i = 0; i < len; ++i) {
        const int v = readGB(addr + i);
        if (v < 0)
          std::printf(" --");
        else
          std::printf(" %02x", v);
      }
      std::printf("\n");
    }
    closeCore();
    return 0;
  }

  // ---- watch: decode named Pokemon Yellow state at a frame (yellow_ram.hpp) ----
  if (cmd == "watch") {
    const uint32_t frame = std::strtoul(argv[5] ? argv[5] : "0", nullptr, 0);
    replayTo(corePath, frame, nullptr, every);
    const auto r8 = [](uint16_t a) { return readGB(a); };
    const auto r16 = [&](uint16_t a) { // big-endian
      return (r8(a) << 8) | r8(a + 1);
    };
    using namespace yellow;
    std::printf("frame %u:\n", frame);
    std::printf("  RNG      Add=%02x Sub=%02x  DSum=%02x  rDIV=%02x frameCtr=%02x\n",
                r8(hRandomAdd), r8(hRandomSub),
                (r8(hRandomAdd) + r8(hRandomSub)) & 0xFF, r8(rDIV),
                r8(hFrameCounter));
    std::printf("  world    map=0x%02x pos=(%d,%d) dir=0x%02x tileset=0x%02x\n",
                r8(wCurMap), r8(wXCoord), r8(wYCoord), r8(wPlayerDirection),
                r8(wCurMapTileset));
    std::printf("  party    count=%d  lead: HP=%d/%d Lv=%d DVs=%04x\n",
                r8(wPartyCount), r16(wPartyMon1HP), r16(wPartyMon1MaxHP),
                r8(wPartyMon1Level), r16(wPartyMon1DVs));
    std::printf("  battle   you: sp=0x%02x HP=%d/%d Lv=%d | foe: sp=0x%02x "
                "HP=%d/%d Lv=%d DVs=%04x\n",
                r8(wBattleMonSpecies), r16(wBattleMonHP), r16(wBattleMonMaxHP),
                r8(wBattleMonLevel), r8(wEnemyMonSpecies), r16(wEnemyMonHP),
                r16(wEnemyMonMaxHP), r8(wEnemyMonLevel), r16(wEnemyMonDVs));
    std::printf("  ids/dmg  trainerID=%04x  crit=%02x damage=%d\n",
                r16(wPlayerID), r8(wCriticalHitOrOHKO), r16(wDamage));
    closeCore();
    return 0;
  }

  // ---- dump: render every Nth frame's framebuffer to <outdir>/f_NNNNNN.ppm in
  // ONE replay pass (for encoding to a GIF/MP4). ----
  if (cmd == "dump") {
    const std::string outdir = argv[5] ? argv[5] : ".";
    const uint32_t everyN = argc > 6 ? std::strtoul(argv[6], nullptr, 0) : 2;
    const uint32_t from = argc > 7 ? std::strtoul(argv[7], nullptr, 0) : 0;
    const uint32_t to = argc > 8 ? std::strtoul(argv[8], nullptr, 0) : UINT32_MAX;
    g_wantFB = true;
    openCore(corePath);
    const uint32_t n = static_cast<uint32_t>(m.input.size());
    uint32_t idx = 0;
    char path[1024];
    for (g_frame = 0; g_frame < n; ++g_frame) {
      g.run();
      if (g_frame >= from && g_frame <= to && (g_frame % (everyN ? everyN : 1)) == 0) {
        std::snprintf(path, sizeof path, "%s/f_%06u.ppm", outdir.c_str(), idx++);
        writePPM(path);
      }
    }
    closeCore();
    std::printf("dumped %u frames to %s\n", idx, outdir.c_str());
    return 0;
  }

  return usage(argv[0]);
}
