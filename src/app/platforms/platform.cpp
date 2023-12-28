//
// Created by nicho on 12/23/2023.
//
#include "platform.hpp"

std::string get_display_name(const Platform::platform p) {
  switch (p) {
  case Platform::GB:
    return "gb";
  case Platform::GBC:
    return "gbc";
  case Platform::GBA:
    return "gba";
  case Platform::N64:
    return "n64";
  case Platform::SNES:
    return "snes";
  default:
    return "unknown";
  }
}

std::string get_display_name_by_extension(const std::string &s) {
  if (s == ".gb")
    return get_display_name(Platform::GB);
  if (s == ".gbc")
    return get_display_name(Platform::GBC);
  if (s == ".gba")
    return get_display_name(Platform::GBA);
  if (s == ".n64")
    return get_display_name(Platform::N64);
  if (s == ".z64")
    return get_display_name(Platform::N64);
  if (s == ".smc")
    return get_display_name(Platform::SNES);
  return "";
}
