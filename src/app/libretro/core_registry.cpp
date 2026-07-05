#include "core_registry.hpp"

#include <algorithm>
#include <firelight/settings/settings_service.hpp>
#include <platform_metadata.hpp>

namespace firelight {
  namespace {
    // RETRO_DEVICE_* base classes + the SUBCLASS macro, mirrored here so the
    // catalog reads like the core sources (libretro.h) without pulling that heavy
    // header into this TU. RETRO_DEVICE_SUBCLASS(base, id) = ((id+1) << 8) | base.
    constexpr unsigned JOYPAD = 1;
    constexpr unsigned MOUSE = 2;
    constexpr unsigned LIGHTGUN = 4;

    constexpr unsigned subclass(const unsigned base, const unsigned id) {
      return ((id + 1) << 8) | base;
    }

    using Class = input::GamepadInputClass;
  } // namespace

  CoreRegistry &CoreRegistry::instance() {
    static CoreRegistry registry;
    return registry;
  }

  CoreRegistry::CoreRegistry() {
    using PM = PlatformMetadata;

    // Bundled cores + the platforms each can run. Where a core can run several
    // platforms, its default platforms are those it's listed as the default for
    // in m_platformDefaults below; the rest are selectable alternates.
    m_cores = {
      {
        "gambatte_libretro", "Gambatte",
        {PM::PLATFORM_ID_GAMEBOY, PM::PLATFORM_ID_GAMEBOY_COLOR}
      },
      // mGBA is the GBA default and also plays GB/GBC (alternate there).
      {
        "mgba_libretro", "mGBA",
        {
          PM::PLATFORM_ID_GAMEBOY_ADVANCE, PM::PLATFORM_ID_GAMEBOY,
          PM::PLATFORM_ID_GAMEBOY_COLOR
        }
      },
      {"fceumm_libretro", "FCEUmm", {PM::PLATFORM_ID_NES}},
      {"snes9x_libretro", "Snes9x", {PM::PLATFORM_ID_SNES}},
      {"mupen64plus_next_libretro", "Mupen64Plus-Next", {PM::PLATFORM_ID_N64}},
      {"melondsds_libretro", "melonDS DS", {PM::PLATFORM_ID_NINTENDO_DS}},
      {
        "genesis_plus_gx_libretro", "Genesis Plus GX",
        {
          PM::PLATFORM_ID_SG1000, PM::PLATFORM_ID_SEGA_GENESIS,
          PM::PLATFORM_ID_SEGA_GAMEGEAR, PM::PLATFORM_ID_SEGA_MASTER_SYSTEM
        }
      },
      {"ppsspp_libretro", "PPSSPP", {PM::PLATFORM_ID_PLAYSTATION_PORTABLE}},
      {
        "mednafen_supergrafx_libretro", "Beetle SuperGrafx",
        {PM::PLATFORM_ID_TURBOGRAFX16, PM::PLATFORM_ID_SUPERGRAFX}
      },
      {"pokemini_libretro", "PokeMini", {PM::PLATFORM_ID_POKEMON_MINI}},
      {
        "mednafen_wswan_libretro", "Beetle WonderSwan",
        {PM::PLATFORM_ID_WONDERSWAN}
      },
      {
        "mednafen_ngp_libretro", "Beetle NeoPop",
        {PM::PLATFORM_ID_NEOGEO_POCKET}
      },
      // Bundled but not yet the default for a modeled platform.
      {"geolith_libretro", "Geolith", {}},
      {"mednafen_psx_hw_libretro", "Beetle PSX HW", {PM::PLATFORM_ID_PS1}}
    };

    // Per-platform default core (matches the historical getCoreName mapping).
    m_platformDefaults = {
      {PM::PLATFORM_ID_GAMEBOY, "gambatte_libretro"},
      {PM::PLATFORM_ID_GAMEBOY_COLOR, "gambatte_libretro"},
      {PM::PLATFORM_ID_GAMEBOY_ADVANCE, "mgba_libretro"},
      {PM::PLATFORM_ID_NES, "fceumm_libretro"},
      {PM::PLATFORM_ID_SNES, "snes9x_libretro"},
      {PM::PLATFORM_ID_N64, "mupen64plus_next_libretro"},
      {PM::PLATFORM_ID_NINTENDO_DS, "melondsds_libretro"},
      {PM::PLATFORM_ID_SG1000, "genesis_plus_gx_libretro"},
      {PM::PLATFORM_ID_SEGA_GENESIS, "genesis_plus_gx_libretro"},
      {PM::PLATFORM_ID_SEGA_GAMEGEAR, "genesis_plus_gx_libretro"},
      {PM::PLATFORM_ID_SEGA_MASTER_SYSTEM, "genesis_plus_gx_libretro"},
      {PM::PLATFORM_ID_PLAYSTATION_PORTABLE, "ppsspp_libretro"},
      {PM::PLATFORM_ID_TURBOGRAFX16, "mednafen_supergrafx_libretro"},
      {PM::PLATFORM_ID_SUPERGRAFX, "mednafen_supergrafx_libretro"},
      {PM::PLATFORM_ID_POKEMON_MINI, "pokemini_libretro"},
      {PM::PLATFORM_ID_WONDERSWAN, "mednafen_wswan_libretro"},
      {PM::PLATFORM_ID_NEOGEO_POCKET, "mednafen_ngp_libretro"},
      {PM::PLATFORM_ID_PS1, "mednafen_psx_hw_libretro"}
    };

    // Curated controller variants per core, verified against each core's source
    // (device subclass values + the input protocol they query). Cores absent
    // here expose only the standard joypad. The standard joypad is implicit (the
    // platform's default) and only listed where the core offers a real choice
    // (Genesis 3- vs 6-button). Cross-referenced at load with the core's runtime
    // SET_CONTROLLER_INFO advertisement.
    m_coreDevices = {
      {
        "genesis_plus_gx_libretro",
        {
          {subclass(JOYPAD, 0), Class::Joypad, "Control Pad (3-button)"},
          {
            subclass(JOYPAD, 1), Class::Joypad, "Control Pad (6-button)", {},
            true
          },
          {MOUSE, Class::Mouse, "Mega Mouse"},
          {subclass(LIGHTGUN, 1), Class::Lightgun, "Menacer"},
          {subclass(LIGHTGUN, 2), Class::Lightgun, "Justifier"},
          {subclass(LIGHTGUN, 0), Class::Lightgun, "Light Phaser"},
        }
      },
      {
        "snes9x_libretro",
        {
          {MOUSE, Class::Mouse, "Mouse"},
          {subclass(LIGHTGUN, 0), Class::Lightgun, "Super Scope"},
          {subclass(LIGHTGUN, 1), Class::Lightgun, "Justifier"},
        }
      },
      {
        "fceumm_libretro",
        {
          // Advertised as a MOUSE subclass, but reads LIGHTGUN ids in
          // "clightgun" mode (its default) — force it so aim/trigger work.
          {
            subclass(MOUSE, 0),
            Class::Lightgun,
            "Zapper",
            {{"fceumm_zapper_mode", "clightgun"}}
          },
        }
      },
    };
  }

  const CoreInfo *CoreRegistry::find(const std::string &coreId) const {
    for (const auto &core: m_cores) {
      if (core.id == coreId) {
        return &core;
      }
    }
    return nullptr;
  }

  std::string CoreRegistry::defaultCoreForPlatform(const int platformId) const {
    const auto it = m_platformDefaults.find(platformId);
    return it != m_platformDefaults.end() ? it->second : std::string{};
  }

  const std::vector<CoreDeviceVariant> &
  CoreRegistry::deviceCatalogForCore(const std::string &coreId) const {
    static const std::vector<CoreDeviceVariant> EMPTY;
    const auto it = m_coreDevices.find(coreId);
    return it != m_coreDevices.end() ? it->second : EMPTY;
  }

  std::vector<CoreDeviceVariant> CoreRegistry::availableControllerVariants(
    const std::string &coreId,
    const std::vector<unsigned> &advertisedDeviceIds) const {
    const auto &catalog = deviceCatalogForCore(coreId);
    const auto advertised = [&](const unsigned id) {
      // No SET_CONTROLLER_INFO from the core -> trust the curated catalog.
      return advertisedDeviceIds.empty() ||
             std::find(advertisedDeviceIds.begin(), advertisedDeviceIds.end(),
                       id) != advertisedDeviceIds.end();
    };

    std::vector<CoreDeviceVariant> result;
    bool hasJoypad = false;
    for (const auto &v: catalog) {
      if (advertised(v.coreDeviceId)) {
        result.push_back(v);
        hasJoypad = hasJoypad || v.deviceClass == input::GamepadInputClass::Joypad;
      }
    }
    if (!hasJoypad) {
      // Every platform has a standard controller; synthesize it as the default
      // when the catalog doesn't enumerate joypad variants for this core.
      result.insert(result.begin(), CoreDeviceVariant{
                      .coreDeviceId = JOYPAD,
                      .deviceClass = Class::Joypad,
                      .friendlyName = "Controller",
                      .isDefault = true
                    });
    }
    return result;
  }

  bool CoreRegistry::supportsPlatform(const std::string &coreId,
                                      const int platformId) const {
    const auto *core = find(coreId);
    if (!core) {
      return false;
    }
    for (const int id: core->supportedPlatformIds) {
      if (id == platformId) {
        return true;
      }
    }
    return false;
  }

  std::vector<PlatformCore>
  CoreRegistry::coresForPlatform(const int platformId) const {
    const auto defaultId = defaultCoreForPlatform(platformId);
    std::vector<PlatformCore> result;
    // Default first.
    for (const auto &core: m_cores) {
      if (core.id == defaultId && supportsPlatform(core.id, platformId)) {
        result.push_back({core.id, core.displayName, true});
      }
    }
    for (const auto &core: m_cores) {
      if (core.id != defaultId && supportsPlatform(core.id, platformId)) {
        result.push_back({core.id, core.displayName, false});
      }
    }
    return result;
  }

  std::string CoreRegistry::displayNameFor(const std::string &coreId) const {
    const auto *core = find(coreId);
    return core ? core->displayName : coreId;
  }

  std::string CoreRegistry::dllPathFor(const std::string &coreId) const {
    if (coreId.empty() || !find(coreId)) {
      return {};
    }
    return PlatformMetadata::getCoreDirectoryPath() + coreId +
           PlatformMetadata::getCoreDllExtension();
  }

  void CoreRegistry::setSessionCoreOverride(const std::string &coreId) {
    m_sessionCoreOverride = coreId;
  }

  std::string
  CoreRegistry::resolveCoreName(const int platformId,
                                const std::string &contentHash,
                                settings::SettingsService *settings) const {
    const auto def = defaultCoreForPlatform(platformId);

    // A CLI `--core` override wins over every stored tier (still guarded by
    // platform support so a mismatched force can't wedge the launch).
    if (!m_sessionCoreOverride.empty() &&
        supportsPlatform(m_sessionCoreOverride, platformId)) {
      return m_sessionCoreOverride;
    }

    if (!settings) {
      return def;
    }

    // Per-game override wins, then per-platform; each honored only if it can run
    // the platform (a stale override must never wedge the platform).
    if (!contentHash.empty()) {
      if (const auto v = settings->getValueAtLevel(settings::Game, contentHash,
                                                   platformId, CORE_SETTING_KEY);
        v && supportsPlatform(*v, platformId)) {
        return *v;
      }
    }
    if (const auto v = settings->getValueAtLevel(settings::Platform, contentHash,
                                                 platformId, CORE_SETTING_KEY);
      v && supportsPlatform(*v, platformId)) {
      return *v;
    }
    return def;
  }
} // namespace firelight
