#include "core_registry.hpp"

#include <firelight/settings/settings_service.hpp>
#include <platform_metadata.hpp>

namespace firelight {

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
      {"gambatte_libretro", "Gambatte",
       {PM::PLATFORM_ID_GAMEBOY, PM::PLATFORM_ID_GAMEBOY_COLOR}},
      // mGBA is the GBA default and also plays GB/GBC (alternate there).
      {"mgba_libretro", "mGBA",
       {PM::PLATFORM_ID_GAMEBOY_ADVANCE, PM::PLATFORM_ID_GAMEBOY,
        PM::PLATFORM_ID_GAMEBOY_COLOR}},
      {"fceumm_libretro", "FCEUmm", {PM::PLATFORM_ID_NES}},
      {"snes9x_libretro", "Snes9x", {PM::PLATFORM_ID_SNES}},
      {"mupen64plus_next_libretro", "Mupen64Plus-Next", {PM::PLATFORM_ID_N64}},
      {"melondsds_libretro", "melonDS DS", {PM::PLATFORM_ID_NINTENDO_DS}},
      {"genesis_plus_gx_libretro", "Genesis Plus GX",
       {PM::PLATFORM_ID_SG1000, PM::PLATFORM_ID_SEGA_GENESIS,
        PM::PLATFORM_ID_SEGA_GAMEGEAR, PM::PLATFORM_ID_SEGA_MASTER_SYSTEM}},
      {"ppsspp_libretro", "PPSSPP", {PM::PLATFORM_ID_PLAYSTATION_PORTABLE}},
      {"mednafen_supergrafx_libretro", "Beetle SuperGrafx",
       {PM::PLATFORM_ID_TURBOGRAFX16, PM::PLATFORM_ID_SUPERGRAFX}},
      {"pokemini_libretro", "PokeMini", {PM::PLATFORM_ID_POKEMON_MINI}},
      {"mednafen_wswan_libretro", "Beetle WonderSwan",
       {PM::PLATFORM_ID_WONDERSWAN}},
      {"mednafen_ngp_libretro", "Beetle NeoPop",
       {PM::PLATFORM_ID_NEOGEO_POCKET}},
      // Bundled but not yet the default for a modeled platform.
      {"geolith_libretro", "Geolith", {}},
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
  };
}

const CoreInfo *CoreRegistry::find(const std::string &coreId) const {
  for (const auto &core : m_cores) {
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

bool CoreRegistry::supportsPlatform(const std::string &coreId,
                                    const int platformId) const {
  const auto *core = find(coreId);
  if (!core) {
    return false;
  }
  for (const int id : core->supportedPlatformIds) {
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
  for (const auto &core : m_cores) {
    if (core.id == defaultId && supportsPlatform(core.id, platformId)) {
      result.push_back({core.id, core.displayName, true});
    }
  }
  for (const auto &core : m_cores) {
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

std::string CoreRegistry::resolveCoreName(const int platformId,
                                          const std::string &contentHash) const {
  const auto def = defaultCoreForPlatform(platformId);

  // A CLI `--core` override wins over every stored tier (still guarded by
  // platform support so a mismatched force can't wedge the launch).
  if (!m_sessionCoreOverride.empty() &&
      supportsPlatform(m_sessionCoreOverride, platformId)) {
    return m_sessionCoreOverride;
  }

  auto *settings = settings::SettingsService::instance();
  if (!settings) {
    return def;
  }

  // Per-game override wins, then per-platform; each honored only if it can run
  // the platform (a stale override must never wedge the platform).
  if (!contentHash.empty()) {
    if (const auto v = settings->getValueAtLevel(settings::Game, contentHash,
                                                 platformId, kCoreSettingKey);
        v && supportsPlatform(*v, platformId)) {
      return *v;
    }
  }
  if (const auto v = settings->getValueAtLevel(settings::Platform, contentHash,
                                               platformId, kCoreSettingKey);
      v && supportsPlatform(*v, platformId)) {
    return *v;
  }
  return def;
}

} // namespace firelight
