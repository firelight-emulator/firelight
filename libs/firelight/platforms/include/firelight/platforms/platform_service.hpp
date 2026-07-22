#pragma once

#include <firelight/platforms/platform.hpp>

#include <optional>
#include <string>
#include <vector>

namespace firelight::platforms {

class IPlatformService {
public:
  virtual ~IPlatformService() = default;

  /**
   * @param id The platform ID to look up
   * @return The platform corresponding to the given ID, or std::nullopt if no platform is found
   */
  [[nodiscard]] virtual std::optional<Platform> getPlatform(unsigned id) const = 0;

  /**
   * @return A list of all the platforms known to the service, in no particular order
   */
  [[nodiscard]] virtual std::vector<Platform> listPlatforms() const = 0;
  /**
   * @param extension The file extension (without the dot) to look up the platform for
   * @return The platform ID corresponding to the given file extension, or PLATFORM_ID_UNKNOWN if no platform is found
   */
  [[nodiscard]] virtual int platformIdForExtension(const std::string &extension) const = 0;

  /**
   * @param rcConsoleId The RetroAchievements console ID to look up the platform for
   * @return The platform ID corresponding to the given RetroAchievements console ID, or PLATFORM_ID_UNKNOWN if no
   * platform is found
   */
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
  [[nodiscard]] int platformIdForExtension(const std::string &extension) const override;
  [[nodiscard]] int platformIdForRcConsole(int rcConsoleId) const override;

private:
  std::vector<Platform> m_platforms;
};

} // namespace firelight::platforms
