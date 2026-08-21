#pragma once

#include <firelight/input/controller_repository.hpp>
#include <firelight/input/gamepad_profile.hpp>

#include <memory>
#include <mutex>
#include <string>

namespace SQLite {
class Database;
}

namespace firelight::platforms {
class PlatformService;
}

namespace firelight::input {
class SqliteControllerRepository final : public IControllerRepository {
public:
  // `platformService` supplies the platform list used to seed per-platform
  // profile mappings (injected rather than reached via a global singleton)
  SqliteControllerRepository(std::string dbFilePath, platforms::PlatformService &platformService);

  ~SqliteControllerRepository() override;

  std::shared_ptr<GamepadProfile> getProfile(int id) override;
  std::shared_ptr<GamepadProfile> createProfile(std::string name, DeviceType device = DeviceType::Gamepad) override;
  std::vector<std::shared_ptr<GamepadProfile>> listProfiles() override;
  std::shared_ptr<GamepadProfile> cloneProfile(int sourceId, std::string newName) override;
  bool deleteProfile(int id) override;
  bool renameProfile(int id, std::string newName) override;
  void setProfileAnalogSettings(int profileId, const AnalogSettings &settings) override;
  void setProfilePresetId(int profileId, const std::string &presetId) override;
  std::string exportProfile(int id) override;
  std::shared_ptr<GamepadProfile> importProfile(const std::string &json) override;

  [[nodiscard]] std::optional<DeviceInfo> getDeviceInfo(DeviceIdentifier identifier) const override;

  void updateDeviceInfo(DeviceIdentifier identifier, const DeviceInfo &info) override;

  void setPlatformPreferredType(int platformId, int gamepadType) override;
  void clearPlatformPreferredType(int platformId) override;
  [[nodiscard]] std::optional<int> getPlatformPreferredType(int platformId) const override;

  [[nodiscard]] std::optional<int> getGameProfileOverride(const std::string &contentHash) const override;
  void setGameProfileOverride(const std::string &contentHash, int profileId) override;
  void clearGameProfileOverride(const std::string &contentHash) override;

private:
  std::string m_dbFilePath;
  platforms::PlatformService &m_platformService;
  std::unique_ptr<SQLite::Database> m_db;
  // Recursive because the public methods call each other (createProfile ->
  // loadProfileContents -> getOrCreateMapping; cloneProfile -> getProfile), and
  // the mapping sync callbacks write from the runtime thread as well
  mutable std::recursive_mutex m_mutex;

  std::shared_ptr<InputMapping> getOrCreateMapping(int profileId, int platformId, int controllerTypeId);
  // Loads a profile's per-platform mappings and shortcut mapping into it
  // (shared by createProfile/getProfile)
  void loadProfileContents(const std::shared_ptr<GamepadProfile> &profile);

  std::vector<std::shared_ptr<GamepadProfile>> m_profiles{};
  std::vector<std::shared_ptr<InputMapping>> m_inputMappings{};
};
} // namespace firelight::input
