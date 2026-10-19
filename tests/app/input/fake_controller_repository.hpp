// TODO: NEEDS REVIEW
#pragma once

#include <firelight/input/controller_repository.hpp>
#include <firelight/input/shortcut_mapping.hpp>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace firelight::input {

// TODO
/** An in-memory profile store that records what the models write */
class FakeControllerRepository final : public IControllerRepository {
public:
  // TODO
  /** One renameProfile call */
  struct RenameCall {
    int profileId;
    std::string name;
  };

  // TODO
  /** One setProfileAnalogSettings call */
  struct AnalogCall {
    int profileId;
    AnalogSettings settings;
  };

  std::vector<RenameCall> renameCalls;
  std::vector<AnalogCall> analogCalls;

  // TODO
  /** Adds a profile under a fixed id, with an empty shortcut mapping */
  std::shared_ptr<GamepadProfile> addProfile(const int id, const std::string &name, const bool isKeyboard = false,
                                             const std::string &icon = {}) {
    auto profile = std::make_shared<GamepadProfile>(id);
    profile->setName(name);
    profile->setIsKeyboardProfile(isKeyboard);
    profile->setIcon(icon);
    profile->setShortcutMapping(std::make_shared<ShortcutMapping>());
    m_profiles[id] = profile;
    return profile;
  }

  std::optional<DeviceInfo> getDeviceInfo(DeviceIdentifier) const override { return std::nullopt; }

  void updateDeviceInfo(DeviceIdentifier, const DeviceInfo &) override {}

  std::shared_ptr<GamepadProfile> getProfile(const int id) override {
    const auto it = m_profiles.find(id);
    return it == m_profiles.end() ? nullptr : it->second;
  }

  std::shared_ptr<GamepadProfile> createProfile(std::string name, const DeviceType device) override {
    return addProfile(m_nextId++, name, device == DeviceType::Keyboard);
  }

  std::vector<std::shared_ptr<GamepadProfile>> listProfiles() override {
    std::vector<std::shared_ptr<GamepadProfile>> profiles;
    for (const auto &[id, profile] : m_profiles) {
      profiles.push_back(profile);
    }

    return profiles;
  }

  std::shared_ptr<GamepadProfile> cloneProfile(int, std::string) override { return nullptr; }

  bool deleteProfile(const int id) override { return m_profiles.erase(id) > 0; }

  // TODO
  /** Refuses a name another profile already has */
  bool renameProfile(const int id, std::string newName) override {
    renameCalls.push_back({id, newName});
    const auto profile = getProfile(id);
    if (!profile) {
      return false;
    }

    for (const auto &[otherId, other] : m_profiles) {
      if (otherId != id && other->getName() == newName) {
        return false;
      }
    }

    profile->setName(std::move(newName));
    return true;
  }

  void setProfileAnalogSettings(const int profileId, const AnalogSettings &settings) override {
    analogCalls.push_back({profileId, settings});
    if (const auto profile = getProfile(profileId)) {
      profile->setDefaultAnalogSettings(settings);
    }
  }

  void setProfilePresetId(const int profileId, const std::string &presetId) override {
    if (const auto profile = getProfile(profileId)) {
      profile->setPresetId(presetId);
    }
  }

  std::string exportProfile(int) override { return {}; }

  std::shared_ptr<GamepadProfile> importProfile(const std::string &) override { return nullptr; }

  void setPlatformPreferredType(const int platformId, const int gamepadType) override {
    m_preferredTypes[platformId] = gamepadType;
  }

  void clearPlatformPreferredType(const int platformId) override { m_preferredTypes.erase(platformId); }

  std::optional<int> getPlatformPreferredType(const int platformId) const override {
    const auto it = m_preferredTypes.find(platformId);
    return it == m_preferredTypes.end() ? std::nullopt : std::optional(it->second);
  }

  std::optional<int> getGameProfileOverride(const std::string &contentHash) const override {
    const auto it = m_gameOverrides.find(contentHash);
    return it == m_gameOverrides.end() ? std::nullopt : std::optional(it->second);
  }

  void setGameProfileOverride(const std::string &contentHash, const int profileId) override {
    m_gameOverrides[contentHash] = profileId;
  }

  void clearGameProfileOverride(const std::string &contentHash) override { m_gameOverrides.erase(contentHash); }

private:
  std::map<int, std::shared_ptr<GamepadProfile>> m_profiles;
  std::map<int, int> m_preferredTypes;
  std::map<std::string, int> m_gameOverrides;
  int m_nextId = 1;
};

} // namespace firelight::input
