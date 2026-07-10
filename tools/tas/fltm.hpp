#pragma once
// FLTM -- Firelight TAS Movie. A minimal, versioned container:
//   header (core name+version, ROM identity, rerecord count)
//   + per-frame input log (InputFrame, mirroring Firelight's 10-byte record)
//   + optional verification checkpoints (frame -> video-framebuffer hash).
// Little-endian binary. Header-only.

#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace tas {

// One frame of input. Mirrors Firelight's InputFrame: a 16-bit button bitmask
// over RETRO_DEVICE_ID_JOYPAD_* ids (0..15) plus four int16 analog axes (unused
// on Game Boy, kept for forward-compatibility / other platforms).
struct InputFrame {
  uint16_t buttons = 0;
  int16_t lx = 0, ly = 0, rx = 0, ry = 0;

  [[nodiscard]] bool button(unsigned id) const {
    return id < 16 && ((buttons >> id) & 1u) != 0;
  }
  void set(unsigned id, bool on) {
    if (id >= 16)
      return;
    const uint16_t bit = static_cast<uint16_t>(1u << id);
    buttons = on ? static_cast<uint16_t>(buttons | bit)
                 : static_cast<uint16_t>(buttons & ~bit);
  }
};

struct Movie {
  static constexpr uint32_t kVersion = 1;

  std::string coreName, coreVersion, romName;
  uint64_t romHash = 0; // FNV-1a of ROM bytes (tool-local identity check)
  uint32_t romSize = 0;
  uint32_t rerecordCount = 0;
  std::vector<InputFrame> input;
  std::vector<std::pair<uint32_t, uint64_t>> checkpoints; // frame -> fb hash

  bool save(const std::string &path) const;
  bool load(const std::string &path);
};

namespace detail {
inline void put(std::vector<uint8_t> &o, uint16_t v) {
  o.push_back(v & 0xFF);
  o.push_back((v >> 8) & 0xFF);
}
inline void put(std::vector<uint8_t> &o, uint32_t v) {
  for (int i = 0; i < 4; ++i)
    o.push_back((v >> (8 * i)) & 0xFF);
}
inline void put(std::vector<uint8_t> &o, uint64_t v) {
  for (int i = 0; i < 8; ++i)
    o.push_back((v >> (8 * i)) & 0xFF);
}
inline void puts(std::vector<uint8_t> &o, const std::string &s) {
  put(o, static_cast<uint16_t>(s.size()));
  o.insert(o.end(), s.begin(), s.end());
}
struct Reader {
  const uint8_t *p, *e;
  bool ok = true;
  Reader(const uint8_t *d, size_t n) : p(d), e(d + n) {}
  uint8_t u8() {
    if (p >= e) { ok = false; return 0; }
    return *p++;
  }
  uint16_t u16() {
    uint16_t v = u8();
    v |= static_cast<uint16_t>(u8()) << 8;
    return v;
  }
  uint32_t u32() {
    uint32_t v = 0;
    for (int i = 0; i < 4; ++i)
      v |= static_cast<uint32_t>(u8()) << (8 * i);
    return v;
  }
  uint64_t u64() {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i)
      v |= static_cast<uint64_t>(u8()) << (8 * i);
    return v;
  }
  std::string str() {
    uint16_t n = u16();
    std::string s;
    for (uint16_t i = 0; i < n && ok; ++i)
      s.push_back(static_cast<char>(u8()));
    return s;
  }
};
} // namespace detail

inline bool Movie::save(const std::string &path) const {
  std::vector<uint8_t> o = {'F', 'L', 'T', 'M'};
  detail::put(o, kVersion);
  detail::puts(o, coreName);
  detail::puts(o, coreVersion);
  detail::puts(o, romName);
  detail::put(o, romHash);
  detail::put(o, romSize);
  detail::put(o, rerecordCount);
  detail::put(o, static_cast<uint32_t>(input.size()));
  for (const auto &f : input) {
    detail::put(o, f.buttons);
    detail::put(o, static_cast<uint16_t>(f.lx));
    detail::put(o, static_cast<uint16_t>(f.ly));
    detail::put(o, static_cast<uint16_t>(f.rx));
    detail::put(o, static_cast<uint16_t>(f.ry));
  }
  detail::put(o, static_cast<uint32_t>(checkpoints.size()));
  for (const auto &c : checkpoints) {
    detail::put(o, c.first);
    detail::put(o, c.second);
  }
  std::ofstream f(path, std::ios::binary);
  if (!f)
    return false;
  f.write(reinterpret_cast<const char *>(o.data()),
          static_cast<std::streamsize>(o.size()));
  return static_cast<bool>(f);
}

inline bool Movie::load(const std::string &path) {
  std::ifstream f(path, std::ios::binary);
  if (!f)
    return false;
  std::vector<uint8_t> d((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
  detail::Reader r(d.data(), d.size());
  if (r.u8() != 'F' || r.u8() != 'L' || r.u8() != 'T' || r.u8() != 'M')
    return false;
  const uint32_t ver = r.u32();
  if (ver != kVersion)
    return false;
  coreName = r.str();
  coreVersion = r.str();
  romName = r.str();
  romHash = r.u64();
  romSize = r.u32();
  rerecordCount = r.u32();
  const uint32_t n = r.u32();
  input.clear();
  input.reserve(n);
  for (uint32_t i = 0; i < n && r.ok; ++i) {
    InputFrame fr;
    fr.buttons = r.u16();
    fr.lx = static_cast<int16_t>(r.u16());
    fr.ly = static_cast<int16_t>(r.u16());
    fr.rx = static_cast<int16_t>(r.u16());
    fr.ry = static_cast<int16_t>(r.u16());
    input.push_back(fr);
  }
  const uint32_t cc = r.u32();
  checkpoints.clear();
  for (uint32_t i = 0; i < cc && r.ok; ++i) {
    const uint32_t fr = r.u32();
    const uint64_t h = r.u64();
    checkpoints.emplace_back(fr, h);
  }
  return r.ok;
}

} // namespace tas
