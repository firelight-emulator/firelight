#pragma once

#include <firelight/input/analog_settings.hpp>
#include <firelight/input/device_identifier.hpp>
#include <firelight/input/device_info.hpp>
#include <firelight/input/gamepad_profile.hpp>
#include <optional>
#include <string>
#include <vector>

namespace firelight::input {
class IControllerRepository {
public:
  virtual ~IControllerRepository() = default;

  // --- Devices ---
  virtual std::optional<DeviceInfo>
  getDeviceInfo(DeviceIdentifier identifier) const = 0;

  virtual void updateDeviceInfo(DeviceIdentifier identifier,
                                const DeviceInfo &info) = 0;

  // --- Profiles ---
  virtual std::shared_ptr<GamepadProfile> getProfile(int id) = 0;

  // TODO
  // `device` decides which shipped preset seeds the new profile's shortcuts,
  // and is persisted so the keyboard's profile is still a keyboard profile
  // next launch
  virtual std::shared_ptr<GamepadProfile>
  createProfile(std::string name, DeviceType device = DeviceType::Gamepad) = 0;

  // Returns every stored profile (built-in and user)
  virtual std::vector<std::shared_ptr<GamepadProfile>> listProfiles() = 0;

  // Deep-copies a profile (bindings, analog settings, shortcuts) under a new
  // name. Returns nullptr if the source does not exist or the name is taken
  virtual std::shared_ptr<GamepadProfile> cloneProfile(int sourceId,
                                                       std::string newName) = 0;

  // Built-in profiles cannot be deleted. Returns false if not deletable/found
  virtual bool deleteProfile(int id) = 0;

  virtual bool renameProfile(int id, std::string newName) = 0;

  // Persists a profile's default analog tuning
  virtual void setProfileAnalogSettings(int profileId,
                                        const AnalogSettings &settings) = 0;

  // Persists which preset a profile's shortcuts are measured against, after the
  // editor applies a different one. Doesn't touch the bindings
  virtual void setProfilePresetId(int profileId, const std::string &presetId) = 0;

  // Serializes a complete profile (analog + per-platform bindings + shortcuts)
  // to a portable JSON document, and recreates one from such a document
  virtual std::string exportProfile(int id) = 0;
  virtual std::shared_ptr<GamepadProfile>
  importProfile(const std::string &json) = 0;

  // --- Preferred controller type per platform (GamepadType as int) ---
  virtual void setPlatformPreferredType(int platformId, int gamepadType) = 0;
  virtual void clearPlatformPreferredType(int platformId) = 0;
  virtual std::optional<int>
  getPlatformPreferredType(int platformId) const = 0;

  // --- Per-game profile overrides (keyed by library content hash) ---
  virtual std::optional<int>
  getGameProfileOverride(const std::string &contentHash) const = 0;

  virtual void setGameProfileOverride(const std::string &contentHash,
                                      int profileId) = 0;

  virtual void clearGameProfileOverride(const std::string &contentHash) = 0;
};
} // namespace firelight::input
