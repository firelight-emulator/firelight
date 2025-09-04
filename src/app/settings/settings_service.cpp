#include "settings_service.hpp"

#include <event_dispatcher.hpp>
#include <utility>

namespace firelight::settings {
SettingsService *SettingsService::s_instance = nullptr;

SettingsService::SettingsService(ISettingsRepository &settingsRepo)
    : m_settingsRepo(settingsRepo) {}

/** Settings level **/
SettingsLevel
SettingsService::getSettingsLevel(const std::string &contentHash) const {
  return m_settingsRepo.getSettingsLevel(contentHash);
}

bool SettingsService::setSettingsLevel(const std::string &contentHash,
                                       const SettingsLevel level) {
  const auto result = m_settingsRepo.setSettingsLevel(contentHash, level);
  if (result) {
    EventDispatcher::instance().publish(
        SettingsLevelChangedEvent{.contentHash = contentHash, .level = level});
  }

  return result;
}

/** Platform settings **/
std::optional<std::string>
SettingsService::getPlatformValue(const int platformId,
                                  const std::string &key) {
  return m_settingsRepo.getPlatformValue(platformId, key);
}

bool SettingsService::setPlatformValue(int platformId, const std::string &key,
                                       const std::string &value) {
  const auto result = m_settingsRepo.setPlatformValue(platformId, key, value);
  if (result) {
    EventDispatcher::instance().publish(PlatformSettingChangedEvent{
        .platformId = platformId,
        .key = key,
        .value = value,
    });
  }

  return result;
}
bool SettingsService::resetPlatformValue(int platformId,
                                         const std::string &key) {
  const auto result = m_settingsRepo.resetPlatformValue(platformId, key);
  if (result) {
    EventDispatcher::instance().publish(PlatformSettingResetEvent{
        .platformId = platformId,
        .key = key,
    });
  }

  return result;
}

/** Game settings **/
std::optional<std::string>
SettingsService::getGameValue(const std::string &contentHash,
                              const std::string &key) {
  return m_settingsRepo.getGameValue(contentHash, key);
}
bool SettingsService::setGameValue(const std::string &contentHash,
                                   const std::string &key,
                                   const std::string &value) {
  const auto result = m_settingsRepo.setGameValue(contentHash, key, value);
  if (result) {
    EventDispatcher::instance().publish(GameSettingChangedEvent{
        .contentHash = contentHash,
        .key = key,
        .value = value,
    });
  }

  return result;
}
bool SettingsService::resetGameValue(const std::string &contentHash,
                                     const std::string &key) {
  const auto result = m_settingsRepo.resetGameValue(contentHash, key);
  if (result) {
    EventDispatcher::instance().publish(GameSettingResetEvent{
        .contentHash = contentHash,
        .key = key,
    });
  }

  return result;
}

/** Delegating methods **/
bool SettingsService::setValue(SettingsLevel level,
                               const std::string &contentHash, int platformId,
                               const std::string &key,
                               const std::string &value) {
  if (level == Game) {
    return setGameValue(contentHash, key, value);
  }

  return setPlatformValue(platformId, key, value);
}

std::optional<std::string>
SettingsService::getValue(SettingsLevel level, const std::string &contentHash,
                          int platformId, const std::string &key) {
  if (level == Game) {
    return getGameValue(contentHash, key);
  }

  return getPlatformValue(platformId, key);
}
} // namespace firelight::settings