#pragma once

#include <firelight/platforms/platform.hpp>
#include <optional>
#include <string>
#include <vector>

namespace firelight::platforms {

class IPlatformService {
public:
  virtual ~IPlatformService() = default;
  [[nodiscard]] virtual std::optional<Platform> getPlatform(unsigned id) const = 0;
  [[nodiscard]] virtual std::vector<Platform> listPlatforms() const = 0;
  // TODO
  // The platform whose file associations include `extension` (lowercase, no
  // dot), or PLATFORM_ID_UNKNOWN. Cartridge extensions only; ambiguous disc
  // extensions are identified by content, not extension
  [[nodiscard]] virtual int
  platformIdForExtension(const std::string &extension) const = 0;
  // Maps an rcheevos console id (RC_CONSOLE_*) to a Firelight platform id
  [[nodiscard]] virtual int platformIdForRcConsole(int rcConsoleId) const = 0;
};

class PlatformService : public IPlatformService {
public:
  static constexpr int PLATFORM_ID_UNKNOWN = -1;
  static constexpr int PLATFORM_ID_GAMEBOY = 1;
  static constexpr int PLATFORM_ID_GAMEBOY_COLOR = 2;
  static constexpr int PLATFORM_ID_GAMEBOY_ADVANCE = 3;
  static constexpr int PLATFORM_ID_VIRTUAL_BOY = 4;
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

  PlatformService();
  PlatformService(PlatformService const &) = delete;

  [[nodiscard]] std::optional<Platform> getPlatform(unsigned id) const override;
  [[nodiscard]] std::vector<Platform> listPlatforms() const override;
  [[nodiscard]] int
  platformIdForExtension(const std::string &extension) const override;
  [[nodiscard]] int platformIdForRcConsole(int rcConsoleId) const override;

private:
  std::vector<Platform> m_platforms;
};

} // namespace firelight::platforms
