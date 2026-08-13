#pragma once

#include <firelight/input/gamepad_input.hpp>
#include <firelight/library/entry_status.hpp>

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace firelight {

namespace settings {
class SettingsService;
}

// A libretro core Firelight knows about. `id` is the DLL base name (e.g.
// "mupen64plus_libretro"), which is also the key the settings/core-options
// catalogs use. Bundled cores ship in system/_cores; user-supplied cores
// (a later phase) will set `bundled = false` and carry their own path
struct CoreInfo {
  std::string id;
  std::string displayName;
  std::vector<int> supportedPlatformIds;
  // What the core will actually open, read out of its own `valid_extensions`. A platform declares
  // the formats its dumps come in, which is a different fact and can disagree with this one
  std::vector<std::string> fileExtensions;
  bool bundled = true;
};

// How badly a platform wants a system file
enum class BiosNecessity {
  // Nothing on the platform boots without it
  Required,
  // The core picks one file by the game's region and does not fall back, so owning one covers
  // that region's games and no others. Having any of them clears the platform, because badging
  // every game would be wrong for the ones that do run
  PerRegion,
  // Enables something extra, and its absence is not a problem
  Optional,
};

// A system file a core boots from, which Firelight does not ship. Any one of `filenames`
// counts as satisfying it
struct BiosRequirement {
  std::vector<std::string> filenames;
  std::string description;
  BiosNecessity necessity = BiosNecessity::Required;
  // Core option key that has to carry `coreOptionValue` before the core will read the file at
  // all ("" where it is read unconditionally). The file on its own does nothing
  std::string coreOption;
  // The value that option has to hold, or "" where merely enabling it is enough
  std::string coreOptionValue;
};

// What one core needs to run one platform. Neither key identifies a requirement on its own: the
// same platform under a different core wants different files, and the same core running a
// different platform wants different files again
struct CoreBios {
  std::string coreId;
  int platformId = 0;
  std::vector<BiosRequirement> requirements;
};

// One requirement resolved against the system directory
struct BiosStatus {
  BiosRequirement requirement;
  std::vector<std::string> presentFiles;
  std::vector<std::string> missingFiles;
  // Whether the platform is held back by this. Always true for an optional file
  bool isSatisfied = true;
};

// One core as offered for a specific platform (default marked)
struct PlatformCore {
  std::string id;
  std::string displayName;
  bool isDefault = false;
};

// A concrete, selectable controller *variant* a specific core can expose on a
// port (e.g. Genesis Plus GX "6-Button Pad", FCEUmm "Zapper"). This is the
// curated, friendly catalog; it's cross-referenced at load against the core's
// runtime SET_CONTROLLER_INFO advertisement (ICore::getControllerDevices) to
// decide availability + the exact device id, and augmented by any uncatalogued
// devices the core reports (advanced only). `deviceClass` selects which
// platform display + bindings apply (Joypad variants share one bindings set)
struct CoreDeviceVariant {
  unsigned coreDeviceId = 1; // RETRO_DEVICE_* (possibly a SUBCLASS)
  input::GamepadInputClass deviceClass = input::GamepadInputClass::Joypad;
  std::string friendlyName;
  // Core options to force when this variant is chosen so the core queries the
  // expected input protocol (e.g. FCEUmm Zapper -> fceumm_zapper_mode=clightgun)
  std::vector<std::pair<std::string, std::string>> companionOptions;
  bool isDefault = false; // the port's default device for this core
};

// Whether a registered core is actually installed and reachable
struct CoreAvailability {
  std::string coreId;
  std::string expectedPath;
  bool present = false; // the file exists on disk
  // Reachable through resolveCoreName: a core with no supported platforms can
  // never be selected
  bool reachable = false;
  // Platforms that name this core as their default and so cannot launch without
  // it
  std::vector<int> defaultForPlatforms;
};

// The authority on which cores exist, which platforms each can run, and each
// platform's default core. Decouples the core from the platform: the platform
// is the stable identity, the core is a resolvable/overridable attribute
class CoreRegistry {
public:
  static CoreRegistry &instance();

  // Every registered core cross-referenced against what is installed on disk
  [[nodiscard]] std::vector<CoreAvailability> checkAvailability() const;

  // The known-good default core for a platform ("" if none)
  [[nodiscard]] std::string defaultCoreForPlatform(int platformId) const;

  // What stands between a platform and running anything. Computed rather than stored, so
  // installing a core makes its games playable without a rescan. Pass the settings service to
  // honor a per-platform core override; without it the platform default is answered
  [[nodiscard]] std::optional<library::EntryProblem> coreProblemForPlatform(int platformId,
                                                                            settings::SettingsService *settings) const;

  // Whether the core a platform would resolve to is actually installed. Says nothing about
  // whether that core has what it needs to boot
  [[nodiscard]] bool hasInstalledCore(int platformId, settings::SettingsService *settings = nullptr) const;

  // Every system file the core this platform resolves to can use, each resolved against the
  // system directory. Empty where that core needs nothing for this platform
  [[nodiscard]] std::vector<BiosStatus> biosStatusForPlatform(int platformId,
                                                              settings::SettingsService *settings = nullptr) const;

  // The same for a named core, skipping resolution
  [[nodiscard]] std::vector<BiosStatus> biosStatusFor(const std::string &coreId, int platformId) const;

  // Whether the platform has what it needs to boot anything. True for a platform that needs
  // nothing, and true while the system directory is unset: not knowing where to look is not the
  // same as having looked. An optional file never decides this
  [[nodiscard]] bool hasRequiredBios(int platformId, settings::SettingsService *settings = nullptr) const;

  // Every platform some core wants a system file for, ascending and without repeats
  [[nodiscard]] std::vector<int> platformsWithBios() const;

  // Where cores are handed their system files. Set once at startup, before any status is computed
  void setSystemDirectory(const std::string &path);

  // Every core that can run the platform, default first ("" list if none)
  [[nodiscard]] std::vector<PlatformCore> coresForPlatform(int platformId) const;

  [[nodiscard]] std::string displayNameFor(const std::string &coreId) const;

  // What the named core will open ("" list for an unknown core)
  [[nodiscard]] const std::vector<std::string> &fileExtensionsFor(const std::string &coreId) const;
  [[nodiscard]] bool supportsPlatform(const std::string &coreId, int platformId) const;

  // Absolute-ish path to the core's DLL ("" if unknown)
  [[nodiscard]] std::string dllPathFor(const std::string &coreId) const;

  [[nodiscard]] const std::vector<CoreInfo> &cores() const { return m_cores; }

  // The curated controller variants a core can expose (empty = joypad only)
  // Not filtered by platform/ROM — callers cross-reference the core's runtime
  // advertisement (ICore::getControllerDevices) to get the game's actual list
  [[nodiscard]] const std::vector<CoreDeviceVariant> &deviceCatalogForCore(const std::string &coreId) const;

  // The controller variants actually selectable for a port: the curated catalog
  // filtered to what the core advertises (`advertisedDeviceIds`, from
  // getControllerDevices — pass empty to trust the catalog when the core gave no
  // info), with the standard joypad synthesized as the default when the catalog
  // offers no joypad variant. Ordered default-first
  [[nodiscard]] std::vector<CoreDeviceVariant>
  availableControllerVariants(const std::string &coreId, const std::vector<unsigned> &advertisedDeviceIds) const;

  // Resolves the effective core for a scope: per-game override -> per-platform
  // override -> platform default. An override is honored only if that core
  // supports the platform (a stale/invalid override falls back). The override
  // tiers come from the passed SettingsService (the "core" key); pass nullptr
  // (or an unset service) to get just the platform default
  [[nodiscard]] std::string resolveCoreName(int platformId, const std::string &contentHash,
                                            settings::SettingsService *settings) const;

  // Forces a specific core for this session (CLI `--core`), winning over the
  // stored per-game/per-platform overrides in resolveCoreName. Honored only
  // where the core actually supports the platform (a mismatched override falls
  // through to the normal resolution). Empty string clears it. Not persisted
  void setSessionCoreOverride(const std::string &coreId);

  // Reserved SettingsService key that stores a platform/game core override
  static constexpr const char *CORE_SETTING_KEY = "core";

private:
  CoreRegistry();
  [[nodiscard]] const CoreInfo *find(const std::string &coreId) const;

  std::vector<CoreInfo> m_cores;
  std::map<int, std::string> m_platformDefaults;
  std::vector<CoreBios> m_coreBios;
  std::map<std::string, std::vector<CoreDeviceVariant>> m_coreDevices;
  std::string m_sessionCoreOverride; // CLI `--core`; empty = none
  std::string m_systemDirectory;     // where cores look for BIOS; empty until startup sets it
};

} // namespace firelight
