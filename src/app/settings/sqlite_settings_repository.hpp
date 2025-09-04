#pragma once

#include "SQLiteCpp/Database.h"
#include "settings_repository.hpp"

#include <QObject>

namespace firelight::settings {
class SqliteSettingsRepository : public QObject, public ISettingsRepository {
  Q_OBJECT
public:
  explicit SqliteSettingsRepository(std::string databaseFile);
  ~SqliteSettingsRepository() override;

  /** Settings level **/
  SettingsLevel getSettingsLevel(std::string contentHash) override;
  bool setSettingsLevel(std::string contentHash, SettingsLevel level) override;

  /** Platform values **/
  std::optional<std::string> getPlatformValue(int platformId,
                                              const std::string &key) override;
  bool setPlatformValue(int platformId, const std::string &key,
                        const std::string &value) override;
  bool resetPlatformValue(int platformId, const std::string &key) override;

  /** Game values **/
  std::optional<std::string> getGameValue(const std::string &contentHash,
                                          const std::string &key) override;
  bool setGameValue(const std::string &contentHash, const std::string &key,
                    const std::string &value) override;
  bool resetGameValue(const std::string &contentHash,
                      const std::string &key) override;

  /** Remove **/
  std::string getEffectiveValue(const std::string &contentHash, int platformId,
                                const std::string &key,
                                const std::string &defaultValue) override;

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
    setGameValue(contentHash.toStdString(), key.toStdString(),
                 value.toStdString());
  }

  Q_INVOKABLE QString getGameValue(const QString &contentHash,
                                   const int platformId, const QString &key) {
    auto value = getGameValue(contentHash.toStdString(), key.toStdString());
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
  std::string m_databaseFile;
  std::unique_ptr<SQLite::Database> m_database;
};

} // namespace firelight::settings