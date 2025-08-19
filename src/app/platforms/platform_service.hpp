#pragma once

#include "models/platform.hpp"

namespace firelight::platforms {

class PlatformService {
public:
  static constexpr int PLATFORM_ID_UNKNOWN = -1;
  static constexpr int PLATFORM_ID_GAMEBOY = 1;
  static constexpr int PLATFORM_ID_GAMEBOY_COLOR = 2;
  static constexpr int PLATFORM_ID_GAMEBOY_ADVANCE = 3;
  static constexpr int PLATFORM_ID_NES = 5;
  static constexpr int PLATFORM_ID_SNES = 6;
  static constexpr int PLATFORM_ID_N64 = 7;
  static constexpr int PLATFORM_ID_NINTENDO_DS = 10;
  static constexpr int PLATFORM_ID_SEGA_MASTER_SYSTEM = 12;
  static constexpr int PLATFORM_ID_SEGA_GENESIS = 13;
  static constexpr int PLATFORM_ID_SEGA_GAMEGEAR = 14;
  static constexpr int PLATFORM_ID_SEGA_SATURN = 15;
  static constexpr int PLATFORM_ID_SEGA_32X = 16;
  static constexpr int PLATFORM_ID_SEGA_CD = 17;
  static constexpr int PLATFORM_ID_PS1 = 18;
  static constexpr int PLATFORM_ID_PS2 = 19;
  static constexpr int PLATFORM_ID_PLAYSTATION_PORTABLE = 20;
  static constexpr int PLATFORM_ID_TURBOGRAFX16 = 21;
  static constexpr int PLATFORM_ID_SUPERGRAFX = 22;
  static constexpr int PLATFORM_ID_POKEMON_MINI = 23;
  static constexpr int PLATFORM_ID_WONDERSWAN = 24;
  static constexpr int PLATFORM_ID_SG1000 = 25;
  static constexpr int PLATFORM_ID_NEOGEO_POCKET = 27;

  static PlatformService &getInstance() {
    static PlatformService instance;
    return instance;
  }

  [[nodiscard]] std::optional<Platform> getPlatform(unsigned id) const;
  [[nodiscard]] std::vector<Platform> listPlatforms() const;

  PlatformService(PlatformService const &) = delete;

private:
  std::vector<Platform> m_platforms;
  PlatformService();
};

} // namespace firelight::platforms