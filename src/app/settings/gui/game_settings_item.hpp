#pragma once

#include <QQuickItem>

#include "manager_accessor.hpp"

namespace firelight::settings {
class GameSettingsItem : public QQuickItem, public ManagerAccessor {
  Q_OBJECT
  Q_PROPERTY(int platformId READ getPlatformId WRITE setPlatformId NOTIFY
                 platformIdChanged)
  Q_PROPERTY(QString contentHash READ getContentHash WRITE setContentHash NOTIFY
                 contentHashChanged)
  Q_PROPERTY(QString pictureMode READ getPictureMode NOTIFY pictureModeChanged)
  Q_PROPERTY(
      bool rewindEnabled READ isRewindEnabled NOTIFY rewindEnabledChanged)

public:
  enum SettingsLevel { Global, Platform, Game };
  Q_ENUM(SettingsLevel)

  explicit GameSettingsItem(QQuickItem *parent = nullptr);

  ~GameSettingsItem() override = default;

  [[nodiscard]] int getPlatformId() const;

  void setPlatformId(int platformId);

  QString getContentHash();

  void setContentHash(const QString &contentHash);

  QString getPictureMode();

  bool isRewindEnabled() const;

  QString getValue(const QString &key, const QString &defaultValue) const;

  void setValue(const QString &key, const QString &value);

  Q_INVOKABLE [[nodiscard]] QString getValue(SettingsLevel level,
                                             const QString &key) const;

  Q_INVOKABLE void setValue(SettingsLevel level, const QString &key,
                            const QString &value);

  Q_INVOKABLE bool valueIsOverridingHigherLevel(SettingsLevel level,
                                                const QString &key) const;

  void resetValue(const QString &key);

signals:
  void platformIdChanged();

  void contentHashChanged();

  void pictureModeChanged();

  void rewindEnabledChanged();

  void settingChanged(QString key, QString value);

private:
  QMap<QString, std::string> m_defaultSettings{};
  int m_platformId = 0;
  QString m_contentHash;
};
} // namespace firelight::settings
