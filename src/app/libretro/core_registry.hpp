#pragma once

#include <map>
#include <string>
#include <vector>

namespace firelight {

// A libretro core Firelight knows about. `id` is the DLL base name (e.g.
// "mupen64plus_libretro"), which is also the key the settings/core-options
// catalogs use. Bundled cores ship in system/_cores; user-supplied cores
// (a later phase) will set `bundled = false` and carry their own path.
struct CoreInfo {
  std::string id;
  std::string displayName;
  std::vector<int> supportedPlatformIds;
  bool bundled = true;
};

// One core as offered for a specific platform (default marked).
struct PlatformCore {
  std::string id;
  std::string displayName;
  bool isDefault = false;
};

// The authority on which cores exist, which platforms each can run, and each
// platform's default core. Decouples the core from the platform: the platform
// is the stable identity, the core is a resolvable/overridable attribute.
class CoreRegistry {
public:
  static CoreRegistry &instance();

  // The known-good default core for a platform ("" if none).
  [[nodiscard]] std::string defaultCoreForPlatform(int platformId) const;

  // Every core that can run the platform, default first ("" list if none).
  [[nodiscard]] std::vector<PlatformCore> coresForPlatform(int platformId) const;

  [[nodiscard]] std::string displayNameFor(const std::string &coreId) const;
  [[nodiscard]] bool supportsPlatform(const std::string &coreId,
                                      int platformId) const;

  // Absolute-ish path to the core's DLL ("" if unknown).
  [[nodiscard]] std::string dllPathFor(const std::string &coreId) const;

  [[nodiscard]] const std::vector<CoreInfo> &cores() const { return m_cores; }

  // Resolves the effective core for a scope: per-game override -> per-platform
  // override -> platform default. An override is honored only if that core
  // supports the platform (a stale/invalid override falls back). Reads
  // SettingsService::instance() (the "core" key); returns the default if unset
  // or the service is unavailable.
  [[nodiscard]] std::string resolveCoreName(int platformId,
                                            const std::string &contentHash) const;

  // Forces a specific core for this session (CLI `--core`), winning over the
  // stored per-game/per-platform overrides in resolveCoreName. Honored only
  // where the core actually supports the platform (a mismatched override falls
  // through to the normal resolution). Empty string clears it. Not persisted.
  void setSessionCoreOverride(const std::string &coreId);

  // Reserved SettingsService key that stores a platform/game core override.
  static constexpr const char *kCoreSettingKey = "core";

private:
  CoreRegistry();
  [[nodiscard]] const CoreInfo *find(const std::string &coreId) const;

  std::vector<CoreInfo> m_cores;
  std::map<int, std::string> m_platformDefaults;
  std::string m_sessionCoreOverride; // CLI `--core`; empty = none.
};

} // namespace firelight
