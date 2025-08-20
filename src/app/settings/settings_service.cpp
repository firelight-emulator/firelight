#include "settings_service.hpp"

namespace firelight::settings {

SettingsService::SettingsService(ISettingsRepository &settingsRepo)
    : m_settingsRepo(settingsRepo) {}
SettingsLevel SettingsService::getSettingsLevel(std::string contentHash) {}
void SettingsService::setSettingsLevel(std::string contentHash,
                                       SettingsLevel level) {}

std::optional<std::string>
SettingsService::getPlatformValue(int platformId, const std::string &key) {
  return m_settingsRepo.getPlatformValue(platformId, key);
}

void SettingsService::setPlatformValue(int platformId, const std::string &key,
                                       const std::string &value) {
  m_settingsRepo.setPlatformValue(platformId, key, value);
}
void SettingsService::resetPlatformValue(int platformId,
                                         const std::string &key) {
  m_settingsRepo.resetPlatformValue(platformId, key);
}
std::optional<std::string>
SettingsService::getGameValue(const std::string &contentHash,
                              const std::string &key) {
  m_settingsRepo.getGameValue(contentHash, -1, key);
}
void SettingsService::setGameValue(const std::string &contentHash,
                                   const std::string &key,
                                   const std::string &value) {
  m_settingsRepo.setGameValue(contentHash, -1, key, value);
}
void SettingsService::resetGameValue(const std::string &contentHash,
                                     const std::string &key) {
  m_settingsRepo.resetGameValue(contentHash, -1, key);
}
} // namespace firelight::settings