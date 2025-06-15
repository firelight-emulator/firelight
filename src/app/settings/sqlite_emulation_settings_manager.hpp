#pragma once

#include "emulation_settings_manager.hpp"

#include <QSqlDatabase>

namespace firelight::settings {
class SqliteEmulationSettingsManager : public QObject,
                                       public IEmulationSettingsManager {
  Q_OBJECT
public:
  enum SettingsLevel { Global, Platform, Game };
  Q_ENUM(SettingsLevel)

  explicit SqliteEmulationSettingsManager(const QString &databaseFile);
  ~SqliteEmulationSettingsManager() override;

  std::optional<std::string> getGlobalValue(const std::string &key) override;
  std::optional<std::string> getPlatformValue(int platformId,
                                              const std::string &key) override;
  std::optional<std::string> getGameValue(const std::string &contentHash,
                                          int platformId,
                                          const std::string &key) override;
  std::string getEffectiveValue(const std::string &contentHash, int platformId,
                                const std::string &key,
                                const std::string &defaultValue) override;
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

  Q_INVOKABLE QString getEffectiveValue(const QString &contentHash,
                                        int platformId, const QString &key,
                                        const QString &defaultValue) {
    return QString::fromStdString(
        getEffectiveValue(contentHash.toStdString(), platformId,
                          key.toStdString(), defaultValue.toStdString()));
  }

  Q_INVOKABLE void setGameValue(const QString &contentHash,
                                const int platformId, const QString &key,
                                const QString &value) {
    setGameValue(contentHash.toStdString(), platformId, key.toStdString(),
                 value.toStdString());
  }

  Q_INVOKABLE QString getGameValue(const QString &contentHash,
                                   const int platformId, const QString &key) {
    auto value =
        getGameValue(contentHash.toStdString(), platformId, key.toStdString());
    return value ? QString::fromStdString(*value) : QString();
  }
signals:
  void globalValueChanged(const QString &key, const QString &value);
  void platformValueChanged(int platformId, const QString &key,
                            const QString &value);
  void gameValueChanged(const QString &contentHash, int platformId,
                        const QString &key, const QString &value);

  void globalValueReset(const QString &key);
  void platformValueReset(int platformId, const QString &key);
  void gameValueReset(const QString &contentHash, int platformId,
                      const QString &key);

private:
  QString m_databaseFile;
  [[nodiscard]] QSqlDatabase getDatabase() const;
};

} // namespace firelight::settings