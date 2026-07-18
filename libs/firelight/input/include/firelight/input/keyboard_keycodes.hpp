#pragma once

#include <cstdint>

namespace firelight::input {

// libretro's "no key". Its keycodes are frozen ABI, so they're mirrored here
// rather than pulling the libretro headers into this library
inline constexpr unsigned RETROK_UNKNOWN = 0;
inline constexpr uint16_t RETROKMOD_SHIFT = 0x01;
inline constexpr uint16_t RETROKMOD_CTRL = 0x02;
inline constexpr uint16_t RETROKMOD_ALT = 0x04;
inline constexpr uint16_t RETROKMOD_META = 0x08;

// The RETROK_* a Qt::Key means, or RETROK_UNKNOWN if the core has no notion of
// it. Only cores that ask for the keyboard (DOS, Amiga, MSX, ScummVM) read
// these; everything else goes through the retropad
unsigned retroKeyFor(int qtKey);

// The RETROKMOD_* mask for a Qt::KeyboardModifiers value
uint16_t retroModifiersFor(int qtModifiers);

} // namespace firelight::input
