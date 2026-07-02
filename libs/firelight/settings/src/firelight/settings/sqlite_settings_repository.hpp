#pragma once

#include <SQLiteCpp/Database.h>
#include <firelight/settings/settings_repository.hpp>

#include <QObject>

namespace firelight::settings {

class SqliteSettingsRepository : public QObject, public ISettingsRepository {
  Q_OBJECT
public:
  explicit SqliteSettingsRepository(std::string databaseFile);
  ~SqliteSettingsRepository() override;

  SettingsLevel getSettingsLevel(std::string contentHash) override;
  bool setSettingsLevel(std::string contentHash, SettingsLevel level) override;

  std::optional<std::string> getGlobalValue(const std::string &key) override;
  bool setGlobalValue(const std::string &key, const std::string &value) override;
  bool resetGlobalValue(const std::string &key) override;

  std::optional<std::string> getPlatformValue(int platformId,
                                              const std::string &key) override;
  bool setPlatformValue(int platformId, const std::string &key,
                        const std::string &value) override;
  bool resetPlatformValue(int platformId, const std::string &key) override;

  std::optional<std::string> getGameValue(const std::string &contentHash,
                                          const std::string &key) override;
  bool setGameValue(const std::string &contentHash, const std::string &key,
                    const std::string &value) override;
  bool resetGameValue(const std::string &contentHash, const std::string &key) override;

  // Canonical resolution: game override -> platform override -> global -> the
  // caller's catalog default.
  Q_INVOKABLE QString getEffectiveValue(const QString &contentHash, int platformId,
                                        const QString &key,
                                        const QString &defaultValue) {
    if (auto v = getGameValue(contentHash.toStdString(), key.toStdString())) {
      return QString::fromStdString(*v);
    }
    if (auto v = getPlatformValue(platformId, key.toStdString())) {
      return QString::fromStdString(*v);
    }
    if (auto v = getGlobalValue(key.toStdString())) {
      return QString::fromStdString(*v);
    }
    return defaultValue;
  }

  Q_INVOKABLE void setGameValue(const QString &contentHash, int platformId,
                                const QString &key, const QString &value) {
    setGameValue(contentHash.toStdString(), key.toStdString(), value.toStdString());
  }

  Q_INVOKABLE QString getGameValue(const QString &contentHash, int platformId,
                                   const QString &key) {
    auto value = getGameValue(contentHash.toStdString(), key.toStdString());
    return value ? QString::fromStdString(*value) : QString();
  }

signals:
  void globalValueChanged(const QString &key, const QString &value);
  void platformValueChanged(int platformId, const QString &key, const QString &value);
  void gameValueChanged(const QString &contentHash, int platformId, const QString &key,
                        const QString &value);
  void globalValueReset(const QString &key);
  void platformValueReset(int platformId, const QString &key);
  void gameValueReset(const QString &contentHash, int platformId, const QString &key);

private:
  std::string m_databaseFile;
  std::unique_ptr<SQLite::Database> m_database;
};

} // namespace firelight::settings
