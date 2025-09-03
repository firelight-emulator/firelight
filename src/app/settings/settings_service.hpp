/**
 * @file settings_service.hpp
 * @brief Settings service providing high-level configuration management
 */

#pragma once
#include "settings_repository.hpp"

#include <optional>
#include <string>

namespace firelight::settings {

/**
 * @struct SettingsLevelChangedEvent
 * @brief Event data for settings level changes
 */
struct SettingsLevelChangedEvent {
  std::string contentHash; ///< The content hash identifier
  SettingsLevel level;     ///< The new settings level
};

/**
 * @struct GameSettingChangedEvent
 * @brief Event data for game setting value changes
 */
struct GameSettingChangedEvent {
  std::string contentHash; ///< The game's content hash
  std::string key;         ///< The setting key
  std::string value;       ///< The new setting value
};

/**
 * @struct GameSettingResetEvent
 * @brief Event data for game setting resets
 */
struct GameSettingResetEvent {
  std::string contentHash; ///< The game's content hash
  std::string key;         ///< The setting key that was reset
};

/**
 * @struct PlatformSettingChangedEvent
 * @brief Event data for platform setting value changes
 */
struct PlatformSettingChangedEvent {
  int platformId;    ///< The platform identifier
  std::string key;   ///< The setting key
  std::string value; ///< The new setting value
};

/**
 * @struct PlatformSettingResetEvent
 * @brief Event data for platform setting resets
 */
struct PlatformSettingResetEvent {
  int platformId;  ///< The platform identifier
  std::string key; ///< The setting key that was reset
};

/**
 * @struct EmulationSettingChangedEvent
 * @brief Event data for emulation setting changes. Occurs when a setting
 * affecting a running emulation instance is changed.
 */
struct EmulationSettingChangedEvent {
  std::string contentHash;
  std::string key;
};

/**
 * @class SettingsService
 * @brief High-level service for managing application settings
 *
 * Provides a service layer over the settings repository, handling both
 * platform-specific and game-specific configuration management with
 * hierarchical setting resolution.
 */
class SettingsService {
public:
  /**
   * @brief Constructs a settings service with the given repository
   * @param settingsRepo Reference to the settings repository implementation
   */
  explicit SettingsService(ISettingsRepository &settingsRepo);

  /**
   * @brief Default destructor
   */
  ~SettingsService() = default;

  static void setInstance(SettingsService *instance) { s_instance = instance; }
  static SettingsService *instance() { return s_instance; }

  /**
   * @brief Get the settings level for a given content hash
   * @param contentHash The content hash identifier
   * @return The settings level (Game or Platform)
   */
  SettingsLevel getSettingsLevel(const std::string &contentHash) const;

  /**
   * @brief Set the settings level for a given content hash
   * @param contentHash The content hash identifier
   * @param level The settings level to set
   * @return True if successful, false otherwise
   */
  bool setSettingsLevel(const std::string &contentHash, SettingsLevel level);

  /**
   * @brief Get a platform-specific setting value
   * @param platformId The platform identifier
   * @param key The setting key
   * @return Optional string value if the setting exists
   */
  std::optional<std::string> getPlatformValue(int platformId,
                                              const std::string &key);

  /**
   * @brief Set a platform-specific setting value
   * @param platformId The platform identifier
   * @param key The setting key
   * @param value The setting value
   * @return True if successful, false otherwise
   */
  bool setPlatformValue(int platformId, const std::string &key,
                        const std::string &value);

  /**
   * @brief Reset a platform-specific setting to default
   * @param platformId The platform identifier
   * @param key The setting key
   * @return True if successful, false otherwise
   */
  bool resetPlatformValue(int platformId, const std::string &key);

  /**
   * @brief Get a game-specific setting value
   * @param contentHash The game's content hash
   * @param key The setting key
   * @return Optional string value if the setting exists
   */
  std::optional<std::string> getGameValue(const std::string &contentHash,
                                          const std::string &key);

  /**
   * @brief Set a game-specific setting value
   * @param contentHash The game's content hash
   * @param key The setting key
   * @param value The setting value
   * @return True if successful, false otherwise
   */
  bool setGameValue(const std::string &contentHash, const std::string &key,
                    const std::string &value);

  /**
   * @brief Reset a game-specific setting to default
   * @param contentHash The game's content hash
   * @param key The setting key
   * @return True if successful, false otherwise
   */
  bool resetGameValue(const std::string &contentHash, const std::string &key);

  /**
   * @brief Set a setting value based on the content hash's settings level
   * @param level The settings level to use (Game or Platform)
   * @param contentHash The content hash identifier
   * @param platformId The platform identifier
   * @param key The setting key
   * @param value The setting value
   * @return True if successful, false otherwise
   */
  bool setValue(SettingsLevel level, const std::string &contentHash,
                int platformId, const std::string &key,
                const std::string &value);

  /**
   * @brief Get a setting value based on the content hash's settings level
   * @param level The settings level to use (Game or Platform)
   * @param contentHash The content hash identifier
   * @param platformId The platform identifier
   * @param key The setting key
   * @return Optional string value if the setting exists
   */
  std::optional<std::string> getValue(SettingsLevel level,
                                      const std::string &contentHash,
                                      int platformId, const std::string &key);

private:
  static SettingsService *s_instance;
  ISettingsRepository &m_settingsRepo; ///< Reference to the settings repository
};

} // namespace firelight::settings