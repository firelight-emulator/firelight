//
// Created by nicho on 12/23/2023.
//

#ifndef PLATFORM_H
#define PLATFORM_H
#include <string>

class Platform {

public:
  enum platform { GB, GBC, GBA, N64, SNES, UNKNOWN };
};

std::string get_display_name(Platform p);

std::string get_display_name_by_extension(const std::string &s);

Platform::platform get_platform_by_extension(const std::string &s);

#endif // PLATFORM_H
