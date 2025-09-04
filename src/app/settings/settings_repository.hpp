/**
 * @file settings_repository.hpp
 * @brief Settings repository interface for managing game and platform settings
 */

#pragma once

#include <optional>
#include <string>

namespace firelight::settings {

/**
 * @enum SettingsLevel
 * @brief Defines the hierarchy level for settings
 */
enum SettingsLevel { Game, Platform, Unknown };

/**
 * @class ISettingsRepository
 * @brief Interface for settings repository implementations
 *
 * Provides methods for storing and retrieving settings at both platform
 * and game-specific levels, with support for hierarchical configuration.
 */
class ISettingsRepository {
public:
  /**
   * @brief Virtual destructor
   */
  virtual ~ISettingsRepository() = default;

  /**
   * @brief Get the settings level for a given content hash
   * @param contentHash The content hash identifier
   * @return The settings level (Game or Platform)
   */
  virtual SettingsLevel getSettingsLevel(std::string contentHash) = 0;

  /**
   * @brief Set the settings level for a given content hash
   * @param contentHash The content hash identifier
   * @param level The settings level to set
   * @return True if successful, false otherwise
   */
  virtual bool setSettingsLevel(std::string contentHash,
                                SettingsLevel level) = 0;

  /**
   * @brief Get a platform-specific setting value
   * @param platformId The platform identifier
   * @param key The setting key
   * @return Optional string value if the setting exists
   */
  virtual std::optional<std::string>
  getPlatformValue(int platformId, const std::string &key) = 0;

  /**
   * @brief Get a game-specific setting value
   * @param contentHash The game's content hash
   * @param key The setting key
   * @return Optional string value if the setting exists
   */
  virtual std::optional<std::string>
  getGameValue(const std::string &contentHash, const std::string &key) = 0;

  /**
   * @brief Set a platform-specific setting value
   * @param platformId The platform identifier
   * @param key The setting key
   * @param value The setting value
   * @return True if successful, false otherwise
   */
  virtual bool setPlatformValue(int platformId, const std::string &key,
                                const std::string &value) = 0;

  /**
   * @brief Set a game-specific setting value
   * @param contentHash The game's content hash
   * @param key The setting key
   * @param value The setting value
   * @return True if successful, false otherwise
   */
  virtual bool setGameValue(const std::string &contentHash,
                            const std::string &key,
                            const std::string &value) = 0;

  /**
   * @brief Reset a platform-specific setting to default
   * @param platformId The platform identifier
   * @param key The setting key
   * @return True if successful, false otherwise
   */
  virtual bool resetPlatformValue(int platformId, const std::string &key) = 0;

  /**
   * @brief Reset a game-specific setting to default
   * @param contentHash The game's content hash
   * @param key The setting key
   * @return True if successful, false otherwise
   */
  virtual bool resetGameValue(const std::string &contentHash,
                              const std::string &key) = 0;

  /// Remove below

  virtual std::string getEffectiveValue(const std::string &contentHash,
                                        int platformId, const std::string &key,
                                        const std::string &defaultValue) = 0;
};
} // namespace firelight::settings