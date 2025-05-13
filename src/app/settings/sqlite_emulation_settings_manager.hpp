#pragma once

#include "emulation_settings_manager.hpp"

#include <QSqlDatabase>

namespace firelight::settings {
class SqliteEmulationSettingsManager : public IEmulationSettingsManager {
public:
  explicit SqliteEmulationSettingsManager(const QString &databaseFile);
  ~SqliteEmulationSettingsManager() override;

  std::optional<std::string> getGlobalValue(const std::string &key) override;
  std::optional<std::string> getPlatformValue(int platformId,
                                              const std::string &key) override;
  std::optional<std::string> getGameValue(const std::string &contentHash,
                                          int platformId,
                                          const std::string &key) override;
  void setGlobalValue(const std::string &key,
                      const std::string &value) override;
  void setPlatformValue(int platformId, const std::string &key,
                        const std::string &value) override;
  void setGameValue(const std::string &contentHash, int platformId,
                    const std::string &key, const std::string &value) override;
  void resetGlobalValue(const std::string &key) override;
  void resetPlatformValue(int platformId, const std::string &key) override;
  void resetGameValue(const std::string &contentHash, int platformId,
                      const std::string &key) override;
  void setOnGlobalValueChanged(
      std::function<void(const std::string &key, const std::string &value)>
          callback) override;
  void setOnPlatformValueChanged(
      std::function<void(int platformId, const std::string &key,
                         const std::string &value)>
          callback) override;
  void setOnGameValueChanged(
      std::function<void(const std::string &contentHash, int platformId,
                         const std::string &key, const std::string &value)>
          callback) override;

private:
  QString m_databaseFile;
  QSqlDatabase getDatabase() const;

  std::function<void(const std::string &key, const std::string &value)>
      m_globalValueChangedCallback;

  std::function<void(int platformId, const std::string &key,
                     const std::string &value)>
      m_platformValueChangedCallback;
  std::function<void(const std::string &contentHash, int platformId,
                     const std::string &key, const std::string &value)>
      m_gameValueChangedCallback;
};

} // namespace firelight::settings