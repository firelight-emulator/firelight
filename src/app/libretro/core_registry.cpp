#include "core_registry.hpp"

#include <firelight/platforms/platform_service.hpp>
#include <firelight/settings/settings_service.hpp>

#include <QCoreApplication>
#include <algorithm>
#include <filesystem>

namespace firelight {
namespace {
// RETRO_DEVICE_* base classes + the SUBCLASS macro, mirrored here so the
// catalog reads like the core sources (libretro.h) without pulling that heavy
// header into this TU. RETRO_DEVICE_SUBCLASS(base, id) = ((id+1) << 8) | base
constexpr unsigned JOYPAD = 1;
constexpr unsigned MOUSE = 2;
constexpr unsigned LIGHTGUN = 4;

constexpr unsigned subclass(const unsigned base, const unsigned id) { return ((id + 1) << 8) | base; }

using Class = input::GamepadInputClass;

// Where bundled cores live and their dll suffix, per OS. Cores are shipped
// under system/_cores/<os>/, resolved against the executable so a launch from
// any working directory finds them
std::string coreDirectoryPath() {
  const auto base = QCoreApplication::applicationDirPath().toStdString() + "/system/_cores/";
#if defined(_WIN32)
  return base + "windows/";
#elif defined(__linux__)
  return base + "linux/";
#elif defined(__APPLE__)
  return base + "macosx/";
#else
  return "";
#endif
}

std::string coreDllExtension() {
#if defined(_WIN32)
  return ".dll";
#elif defined(__APPLE__)
  return ".dylib";
#elif defined(__linux__)
  return ".so";
#else
  return "";
#endif
}
} // namespace

CoreRegistry &CoreRegistry::instance() {
  static CoreRegistry registry;
  return registry;
}

CoreRegistry::CoreRegistry() {
  using PM = platforms::PlatformService;

  // Bundled cores, the platforms each can run, and the extensions each will open. Where a core can
  // run several platforms, its default platforms are those it's listed as the default for in
  // m_platformDefaults below; the rest are selectable alternates. Extension lists are read out of
  // each shipped binary's own valid_extensions rather than from documentation
  m_cores = {
      {"gambatte_libretro", "Gambatte", {PM::PLATFORM_ID_GAMEBOY, PM::PLATFORM_ID_GAMEBOY_COLOR}, {"gb", "gbc", "dmg"}},
      // mGBA is the GBA default and also plays GB/GBC (alternate there)
      {"mgba_libretro",
       "mGBA",
       {PM::PLATFORM_ID_GAMEBOY_ADVANCE, PM::PLATFORM_ID_GAMEBOY, PM::PLATFORM_ID_GAMEBOY_COLOR},
       {"gba", "gb", "gbc", "sgb"}},
      {"fceumm_libretro",
       "FCEUmm",
       {PM::PLATFORM_ID_NES, PM::PLATFORM_ID_FAMICOM_DISK_SYSTEM},
       {"fds", "nes", "unf", "unif"}},
      {"snes9x_libretro", "Snes9x", {PM::PLATFORM_ID_SNES}, {"smc", "sfc", "swc", "fig", "bs", "st"}},
      {"mupen64plus_next_libretro", "Mupen64Plus-Next", {PM::PLATFORM_ID_N64}, {"n64", "v64", "z64", "bin", "u1"}},
      {"melondsds_libretro", "melonDS DS", {PM::PLATFORM_ID_NINTENDO_DS}, {"nds", "dsi", "ids", "gba"}},
      {"genesis_plus_gx_libretro",
       "Genesis Plus GX",
       {PM::PLATFORM_ID_SG1000, PM::PLATFORM_ID_SEGA_GENESIS, PM::PLATFORM_ID_SEGA_GAMEGEAR,
        PM::PLATFORM_ID_SEGA_MASTER_SYSTEM, PM::PLATFORM_ID_SEGA_CD},
       {"m3u", "mdx", "md", "smd", "gen", "bin", "cue", "iso", "chd", "bms", "sms", "gg", "sg", "68k", "sgd"}},
      {"ppsspp_libretro", "PPSSPP", {PM::PLATFORM_ID_PLAYSTATION_PORTABLE}, {"elf", "iso", "cso", "prx", "pbp", "chd"}},
      {"mednafen_supergrafx_libretro",
       "Beetle SuperGrafx",
       {PM::PLATFORM_ID_TURBOGRAFX16, PM::PLATFORM_ID_PC_ENGINE_CD},
       {"pce", "sgx", "cue", "ccd", "chd", "toc", "m3u"}},
      {"pokemini_libretro", "PokeMini", {PM::PLATFORM_ID_POKEMON_MINI}, {"min"}},
      {"mednafen_wswan_libretro", "Beetle WonderSwan", {PM::PLATFORM_ID_WONDERSWAN}, {"ws", "wsc", "pc2"}},
      {"mednafen_ngp_libretro", "Beetle NeoPop", {PM::PLATFORM_ID_NEOGEO_POCKET}, {"ngp", "ngc", "ngpc", "npc"}},
      // Bundled but not yet the default for a modeled platform
      {"geolith_libretro", "Geolith", {}, {"neo"}},
      // Not shipped, so nothing has been read off it
      {"mednafen_psx_hw_libretro", "Beetle PSX HW", {PM::PLATFORM_ID_PS1}, {}},
      {"mednafen_vb_libretro", "Beetle VB", {PM::PLATFORM_ID_VIRTUAL_BOY}, {"vb", "vboy"}}};

  // Per-platform default core (matches the historical getCoreName mapping)
  m_platformDefaults = {{PM::PLATFORM_ID_GAMEBOY, "gambatte_libretro"},
                        {PM::PLATFORM_ID_GAMEBOY_COLOR, "gambatte_libretro"},
                        {PM::PLATFORM_ID_GAMEBOY_ADVANCE, "mgba_libretro"},
                        {PM::PLATFORM_ID_NES, "fceumm_libretro"},
                        {PM::PLATFORM_ID_FAMICOM_DISK_SYSTEM, "fceumm_libretro"},
                        {PM::PLATFORM_ID_SNES, "snes9x_libretro"},
                        {PM::PLATFORM_ID_N64, "mupen64plus_next_libretro"},
                        {PM::PLATFORM_ID_NINTENDO_DS, "melondsds_libretro"},
                        {PM::PLATFORM_ID_SG1000, "genesis_plus_gx_libretro"},
                        {PM::PLATFORM_ID_SEGA_GENESIS, "genesis_plus_gx_libretro"},
                        {PM::PLATFORM_ID_SEGA_GAMEGEAR, "genesis_plus_gx_libretro"},
                        {PM::PLATFORM_ID_SEGA_MASTER_SYSTEM, "genesis_plus_gx_libretro"},
                        {PM::PLATFORM_ID_SEGA_CD, "genesis_plus_gx_libretro"},
                        {PM::PLATFORM_ID_PLAYSTATION_PORTABLE, "ppsspp_libretro"},
                        {PM::PLATFORM_ID_TURBOGRAFX16, "mednafen_supergrafx_libretro"},
                        {PM::PLATFORM_ID_PC_ENGINE_CD, "mednafen_supergrafx_libretro"},
                        {PM::PLATFORM_ID_POKEMON_MINI, "pokemini_libretro"},
                        {PM::PLATFORM_ID_WONDERSWAN, "mednafen_wswan_libretro"},
                        {PM::PLATFORM_ID_NEOGEO_POCKET, "mednafen_ngp_libretro"},
                        {PM::PLATFORM_ID_PS1, "mednafen_psx_hw_libretro"},
                        {PM::PLATFORM_ID_VIRTUAL_BOY, "mednafen_vb_libretro"}};

  // Filenames and core-option keys read out of each shipped core's own binary; which of them a
  // system actually needs read from that core's documentation. A core/platform pair absent here
  // needs nothing
  using Bios = BiosNecessity;
  m_coreBios = {
      {"genesis_plus_gx_libretro",
       PM::PLATFORM_ID_SEGA_CD,
       {{{"bios_CD_U.bin", "bios_CD_E.bin", "bios_CD_J.bin"}, "Sega CD BIOS", Bios::PerRegion}}},
      {"genesis_plus_gx_libretro",
       PM::PLATFORM_ID_SEGA_GENESIS,
       {{{"bios_MD.bin"}, "Mega Drive startup ROM", Bios::Optional, "genesis_plus_gx_bios"},
        {{"sk.bin"}, "Sonic & Knuckles lock-on ROM", Bios::Optional, "genesis_plus_gx_lock_on", "sonic & knuckles"},
        {{"sk2chip.bin"}, "Sonic & Knuckles UPMEM ROM", Bios::Optional, "genesis_plus_gx_lock_on", "sonic & knuckles"},
        {{"areplay.bin"}, "Action Replay ROM", Bios::Optional, "genesis_plus_gx_lock_on", "action replay (pro)"},
        {{"ggenie.bin"}, "Game Genie ROM", Bios::Optional, "genesis_plus_gx_lock_on", "game genie"}}},
      {"genesis_plus_gx_libretro",
       PM::PLATFORM_ID_SEGA_MASTER_SYSTEM,
       {{{"bios_U.sms", "bios_E.sms", "bios_J.sms"},
         "Master System startup ROM",
         Bios::Optional,
         "genesis_plus_gx_bios"}}},
      {"genesis_plus_gx_libretro",
       PM::PLATFORM_ID_SEGA_GAMEGEAR,
       {{{"bios.gg"}, "Game Gear startup ROM", Bios::Optional, "genesis_plus_gx_bios"}}},
      {"mednafen_supergrafx_libretro", PM::PLATFORM_ID_PC_ENGINE_CD, {{{"syscard3.pce"}, "PC Engine CD system card"}}},
      {"fceumm_libretro", PM::PLATFORM_ID_FAMICOM_DISK_SYSTEM, {{{"disksys.rom"}, "Famicom Disk System BIOS"}}},
      // mGBA runs on its own high-level BIOS when the file is absent, so this is an accuracy
      // extra rather than something to boot from. Gambatte reaches the Game Boy platforms with
      // different filenames again, which is why neither key alone identifies a requirement
      {"mgba_libretro",
       PM::PLATFORM_ID_GAMEBOY_ADVANCE,
       {{{"gba_bios.bin"}, "Game Boy Advance BIOS", Bios::Optional, "mgba_use_bios", "ON"}}},
      // The one entry not taken from a binary, because the core is not shipped yet
      {"mednafen_psx_hw_libretro",
       PM::PLATFORM_ID_PS1,
       {{{"scph5500.bin", "scph5501.bin", "scph5502.bin"}, "PlayStation BIOS", Bios::PerRegion}}}};

  // Curated controller variants per core, verified against each core's source
  // (device subclass values + the input protocol they query). Cores absent
  // here expose only the standard joypad. The standard joypad is implicit (the
  // platform's default) and only listed where the core offers a real choice
  // (Genesis 3- vs 6-button). Cross-referenced at load with the core's runtime
  // SET_CONTROLLER_INFO advertisement
  m_coreDevices = {
      {"genesis_plus_gx_libretro",
       {
           {subclass(JOYPAD, 0), Class::Joypad, "Control Pad (3-button)"},
           {subclass(JOYPAD, 1), Class::Joypad, "Control Pad (6-button)", {}, true},
           {MOUSE, Class::Mouse, "Mega Mouse"},
           {subclass(LIGHTGUN, 1), Class::Lightgun, "Menacer"},
           {subclass(LIGHTGUN, 2), Class::Lightgun, "Justifier"},
           {subclass(LIGHTGUN, 0), Class::Lightgun, "Light Phaser"},
       }},
      {"snes9x_libretro",
       {
           {MOUSE, Class::Mouse, "Mouse"},
           {subclass(LIGHTGUN, 0), Class::Lightgun, "Super Scope"},
           {subclass(LIGHTGUN, 1), Class::Lightgun, "Justifier"},
       }},
      {"fceumm_libretro",
       {
           // Advertised as a MOUSE subclass, but reads LIGHTGUN ids in
           // "clightgun" mode (its default) — force it so aim/trigger work
           {subclass(MOUSE, 0), Class::Lightgun, "Zapper", {{"fceumm_zapper_mode", "clightgun"}}},
       }},
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

const std::vector<CoreDeviceVariant> &CoreRegistry::deviceCatalogForCore(const std::string &coreId) const {
  static const std::vector<CoreDeviceVariant> EMPTY;
  const auto it = m_coreDevices.find(coreId);
  return it != m_coreDevices.end() ? it->second : EMPTY;
}

std::vector<CoreDeviceVariant>
CoreRegistry::availableControllerVariants(const std::string &coreId,
                                          const std::vector<unsigned> &advertisedDeviceIds) const {
  const auto &catalog = deviceCatalogForCore(coreId);
  const auto advertised = [&](const unsigned id) {
    // No SET_CONTROLLER_INFO from the core -> trust the curated catalog
    return advertisedDeviceIds.empty() ||
           std::find(advertisedDeviceIds.begin(), advertisedDeviceIds.end(), id) != advertisedDeviceIds.end();
  };

  std::vector<CoreDeviceVariant> result;
  bool hasJoypad = false;
  for (const auto &v : catalog) {
    if (advertised(v.coreDeviceId)) {
      result.push_back(v);
      hasJoypad = hasJoypad || v.deviceClass == input::GamepadInputClass::Joypad;
    }
  }
  if (!hasJoypad) {
    // Every platform has a standard controller; synthesize it as the default
    // when the catalog doesn't enumerate joypad variants for this core
    result.insert(result.begin(), CoreDeviceVariant{.coreDeviceId = JOYPAD,
                                                    .deviceClass = Class::Joypad,
                                                    .friendlyName = "Controller",
                                                    .isDefault = true});
  }
  return result;
}

bool CoreRegistry::supportsPlatform(const std::string &coreId, const int platformId) const {
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

std::vector<PlatformCore> CoreRegistry::coresForPlatform(const int platformId) const {
  const auto defaultId = defaultCoreForPlatform(platformId);
  std::vector<PlatformCore> result;
  // Default first
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

const std::vector<std::string> &CoreRegistry::fileExtensionsFor(const std::string &coreId) const {
  static const std::vector<std::string> EMPTY;

  const auto *core = find(coreId);
  return core ? core->fileExtensions : EMPTY;
}

std::string CoreRegistry::dllPathFor(const std::string &coreId) const {
  if (coreId.empty() || !find(coreId)) {
    return {};
  }
  return coreDirectoryPath() + coreId + coreDllExtension();
}

std::optional<library::EntryProblem> CoreRegistry::coreProblemForPlatform(const int platformId,
                                                                          settings::SettingsService *settings) const {
  // The content hash is only consulted for the per-game tier, so leaving it empty answers for
  // the platform and its override alike
  const auto coreId = resolveCoreName(platformId, "", settings);

  if (coreId.empty()) {
    return library::EntryProblem::PlatformNotSupported;
  }

  std::error_code ec;

  if (!std::filesystem::exists(dllPathFor(coreId), ec)) {
    return library::EntryProblem::CoreNotInstalled;
  }

  const auto bios = biosStatusFor(coreId, platformId);

  if (!std::ranges::all_of(bios, [](const BiosStatus &status) { return status.isSatisfied; })) {
    return library::EntryProblem::BiosMissing;
  }

  return std::nullopt;
}

bool CoreRegistry::hasInstalledCore(const int platformId, settings::SettingsService *settings) const {
  const auto coreId = resolveCoreName(platformId, "", settings);

  std::error_code ec;
  return !coreId.empty() && std::filesystem::exists(dllPathFor(coreId), ec);
}

std::vector<BiosStatus> CoreRegistry::biosStatusForPlatform(const int platformId,
                                                            settings::SettingsService *settings) const {
  return biosStatusFor(resolveCoreName(platformId, "", settings), platformId);
}

std::vector<BiosStatus> CoreRegistry::biosStatusFor(const std::string &coreId, const int platformId) const {
  const auto entry = std::ranges::find_if(
      m_coreBios, [&](const CoreBios &bios) { return bios.coreId == coreId && bios.platformId == platformId; });

  if (entry == m_coreBios.end()) {
    return {};
  }

  std::vector<BiosStatus> statuses;
  statuses.reserve(entry->requirements.size());

  for (const auto &requirement : entry->requirements) {
    BiosStatus status;
    status.requirement = requirement;

    // Both lists stay empty rather than filing everything as absent, so nothing reads an
    // unset directory as a directory with nothing in it
    if (m_systemDirectory.empty()) {
      statuses.push_back(std::move(status));
      continue;
    }

    std::error_code ec;

    for (const auto &filename : requirement.filenames) {
      if (std::filesystem::exists(m_systemDirectory + "/" + filename, ec)) {
        status.presentFiles.push_back(filename);
      } else {
        status.missingFiles.push_back(filename);
      }
    }

    status.isSatisfied = requirement.necessity == BiosNecessity::Optional || !status.presentFiles.empty();
    statuses.push_back(std::move(status));
  }

  return statuses;
}

bool CoreRegistry::hasRequiredBios(const int platformId, settings::SettingsService *settings) const {
  const auto statuses = biosStatusForPlatform(platformId, settings);
  return std::ranges::all_of(statuses, [](const BiosStatus &status) { return status.isSatisfied; });
}

std::vector<int> CoreRegistry::platformsWithBios() const {
  std::vector<int> platformIds;
  platformIds.reserve(m_coreBios.size());

  for (const auto &bios : m_coreBios) {
    platformIds.push_back(bios.platformId);
  }

  std::ranges::sort(platformIds);
  const auto repeats = std::ranges::unique(platformIds);
  platformIds.erase(repeats.begin(), repeats.end());

  return platformIds;
}

void CoreRegistry::setSystemDirectory(const std::string &path) { m_systemDirectory = path; }

std::vector<CoreAvailability> CoreRegistry::checkAvailability() const {
  std::vector<CoreAvailability> result;
  result.reserve(m_cores.size());

  for (const auto &core : m_cores) {
    CoreAvailability availability;
    availability.coreId = core.id;
    availability.expectedPath = dllPathFor(core.id);
    availability.reachable = !core.supportedPlatformIds.empty();

    std::error_code ec;
    availability.present = !availability.expectedPath.empty() && std::filesystem::exists(availability.expectedPath, ec);

    for (const auto &[platformId, defaultCore] : m_platformDefaults) {
      if (defaultCore == core.id) {
        availability.defaultForPlatforms.push_back(platformId);
      }
    }

    result.push_back(std::move(availability));
  }

  return result;
}

void CoreRegistry::setSessionCoreOverride(const std::string &coreId) { m_sessionCoreOverride = coreId; }

std::string CoreRegistry::resolveCoreName(const int platformId, const std::string &contentHash,
                                          settings::SettingsService *settings) const {
  const auto def = defaultCoreForPlatform(platformId);

  // A CLI `--core` override wins over every stored tier (still guarded by
  // platform support so a mismatched force can't wedge the launch)
  if (!m_sessionCoreOverride.empty() && supportsPlatform(m_sessionCoreOverride, platformId)) {
    return m_sessionCoreOverride;
  }

  if (!settings) {
    return def;
  }

  // Per-game override wins, then per-platform; each honored only if it can run
  // the platform (a stale override must never wedge the platform)
  if (!contentHash.empty()) {
    if (const auto v = settings->getValueAtLevel(settings::Game, contentHash, platformId, CORE_SETTING_KEY);
        v && supportsPlatform(*v, platformId)) {
      return *v;
    }
  }
  if (const auto v = settings->getValueAtLevel(settings::Platform, contentHash, platformId, CORE_SETTING_KEY);
      v && supportsPlatform(*v, platformId)) {
    return *v;
  }
  return def;
}
} // namespace firelight
