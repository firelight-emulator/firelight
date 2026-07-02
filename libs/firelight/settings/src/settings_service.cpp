#include <firelight/settings/settings_service.hpp>
#include <firelight/event_dispatcher.hpp>

namespace firelight::settings {

SettingsService *SettingsService::s_instance = nullptr;

SettingsService::SettingsService(ISettingsRepository &settingsRepo)
    : m_settingsRepo(settingsRepo) {}

SettingsLevel SettingsService::getSettingsLevel(const std::string &contentHash) const {
  return m_settingsRepo.getSettingsLevel(contentHash);
}

bool SettingsService::setSettingsLevel(const std::string &contentHash,
                                       SettingsLevel level) {
  const auto result = m_settingsRepo.setSettingsLevel(contentHash, level);
  if (result) {
    EventDispatcher::instance().publish(
        SettingsLevelChangedEvent{.contentHash = contentHash, .level = level});
  }
  return result;
}

std::optional<std::string>
SettingsService::getGlobalValue(const std::string &key) {
  return m_settingsRepo.getGlobalValue(key);
}

bool SettingsService::setGlobalValue(const std::string &key,
                                     const std::string &value) {
  const auto result = m_settingsRepo.setGlobalValue(key, value);
  if (result) {
    EventDispatcher::instance().publish(
        GlobalSettingChangedEvent{.key = key, .value = value});
  }
  return result;
}

bool SettingsService::resetGlobalValue(const std::string &key) {
  const auto result = m_settingsRepo.resetGlobalValue(key);
  if (result) {
    EventDispatcher::instance().publish(GlobalSettingResetEvent{.key = key});
  }
  return result;
}

std::optional<std::string> SettingsService::getPlatformValue(int platformId,
                                                              const std::string &key) {
  return m_settingsRepo.getPlatformValue(platformId, key);
}

bool SettingsService::setPlatformValue(int platformId, const std::string &key,
                                       const std::string &value) {
  const auto result = m_settingsRepo.setPlatformValue(platformId, key, value);
  if (result) {
    EventDispatcher::instance().publish(PlatformSettingChangedEvent{
        .platformId = platformId, .key = key, .value = value});
  }
  return result;
}

bool SettingsService::resetPlatformValue(int platformId, const std::string &key) {
  const auto result = m_settingsRepo.resetPlatformValue(platformId, key);
  if (result) {
    EventDispatcher::instance().publish(
        PlatformSettingResetEvent{.platformId = platformId, .key = key});
  }
  return result;
}

std::optional<std::string> SettingsService::getGameValue(const std::string &contentHash,
                                                          const std::string &key) {
  return m_settingsRepo.getGameValue(contentHash, key);
}

bool SettingsService::setGameValue(const std::string &contentHash, const std::string &key,
                                   const std::string &value) {
  const auto result = m_settingsRepo.setGameValue(contentHash, key, value);
  if (result) {
    EventDispatcher::instance().publish(GameSettingChangedEvent{
        .contentHash = contentHash, .key = key, .value = value});
  }
  return result;
}

bool SettingsService::resetGameValue(const std::string &contentHash,
                                     const std::string &key) {
  const auto result = m_settingsRepo.resetGameValue(contentHash, key);
  if (result) {
    EventDispatcher::instance().publish(
        GameSettingResetEvent{.contentHash = contentHash, .key = key});
  }
  return result;
}

bool SettingsService::setValueAtLevel(SettingsLevel level,
                                      const std::string &contentHash,
                                      int platformId, const std::string &key,
                                      const std::string &value) {
  switch (level) {
  case Game:
    return setGameValue(contentHash, key, value);
  case Global:
    return setGlobalValue(key, value);
  default:
    return setPlatformValue(platformId, key, value);
  }
}

std::optional<std::string> SettingsService::getValueAtLevel(
    SettingsLevel level, const std::string &contentHash, int platformId,
    const std::string &key) {
  switch (level) {
  case Game:
    return getGameValue(contentHash, key);
  case Global:
    return getGlobalValue(key);
  default:
    return getPlatformValue(platformId, key);
  }
}

bool SettingsService::resetValueAtLevel(SettingsLevel level,
                                        const std::string &contentHash,
                                        int platformId, const std::string &key) {
  switch (level) {
  case Game:
    return resetGameValue(contentHash, key);
  case Global:
    return resetGlobalValue(key);
  default:
    return resetPlatformValue(platformId, key);
  }
}

std::optional<std::string>
SettingsService::getEffectiveValue(const std::string &contentHash,
                                   int platformId, const std::string &key) {
  if (auto v = getGameValue(contentHash, key)) {
    return v;
  }
  if (auto v = getPlatformValue(platformId, key)) {
    return v;
  }
  return getGlobalValue(key);
}

} // namespace firelight::settings
