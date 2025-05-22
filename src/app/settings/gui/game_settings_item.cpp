#include "game_settings_item.hpp"

namespace firelight::settings {
GameSettingsItem::GameSettingsItem(QQuickItem *parent) : QQuickItem(parent) {
  m_defaultSettings.insert("rewind-enabled", "true");
  m_defaultSettings.insert("picture-mode", "aspect-ratio-fill");
}

void GameSettingsItem::setPlatformId(const int platformId) {
  if (m_platformId == platformId) {
    return;
  }

  m_platformId = platformId;
  emit pictureModeChanged();
  emit rewindEnabledChanged();
  emit platformIdChanged();
}

int GameSettingsItem::getPlatformId() const { return m_platformId; }

QString GameSettingsItem::getContentHash() { return m_contentHash; }
void GameSettingsItem::setContentHash(const QString &contentHash) {
  if (m_contentHash == contentHash) {
    return;
  }

  m_contentHash = contentHash;
  emit pictureModeChanged();
  emit rewindEnabledChanged();
  emit contentHashChanged();
}
QString GameSettingsItem::getPictureMode() {
  auto value = getValue("picture-mode", "aspect-ratio-fill");
  spdlog::info("Getting picture-mode for {}, {}: {}",
               m_contentHash.toStdString(), m_platformId, value.toStdString());
  return value;
}
bool GameSettingsItem::isRewindEnabled() const {
  spdlog::info("Getting rewind-enabled for {}, {}", m_contentHash.toStdString(),
               m_platformId);
  return getValue("rewind-enabled", "true") == "true";
}

QString GameSettingsItem::getValue(const QString &key,
                                   const QString &defaultValue) const {
  if (m_contentHash.isEmpty() || m_platformId <= 0) {
    return defaultValue;
  }

  const auto gameValue = getEmulationSettingsManager()->getGameValue(
      m_contentHash.toStdString(), m_platformId, key.toStdString());
  if (gameValue.has_value()) {
    return QString::fromStdString(gameValue.value());
  }

  const auto platformValue = getEmulationSettingsManager()->getPlatformValue(
      m_platformId, key.toStdString());
  if (platformValue.has_value()) {
    return QString::fromStdString(platformValue.value());
  }

  const auto globalValue =
      getEmulationSettingsManager()->getGlobalValue(key.toStdString());
  if (globalValue.has_value()) {
    return QString::fromStdString(globalValue.value());
  }

  return defaultValue;
}
void GameSettingsItem::setValue(const QString &key, const QString &value) {
  const auto currentValue = getEmulationSettingsManager()->getGameValue(
      m_contentHash.toStdString(), m_platformId, key.toStdString());

  if (currentValue.has_value() &&
      QString::fromStdString(currentValue.value()) == value) {
    return;
  }

  getEmulationSettingsManager()->setGameValue(m_contentHash.toStdString(),
                                              m_platformId, key.toStdString(),
                                              value.toStdString());

  emit settingChanged(key, value);
  if (key == "picture-mode") {
    emit pictureModeChanged();
  } else if (key == "rewind-enabled") {
    emit rewindEnabledChanged();
  }
}

QString GameSettingsItem::getValue(const SettingsLevel level,
                                   const QString &key) const {
  switch (level) {
  case Global:
    return QString::fromStdString(getEmulationSettingsManager()
                                      ->getGlobalValue(key.toStdString())
                                      .value_or(m_defaultSettings[key]));
  case Platform:
    return QString::fromStdString(
        getEmulationSettingsManager()
            ->getPlatformValue(m_platformId, key.toStdString())
            .value_or(m_defaultSettings[key]));
  case Game:
    return QString::fromStdString(
        getEmulationSettingsManager()
            ->getGameValue(m_contentHash.toStdString(), m_platformId,
                           key.toStdString())
            .value_or(m_defaultSettings[key]));
  default:
    return {};
  }
}
void GameSettingsItem::setValue(const SettingsLevel level, const QString &key,
                                const QString &value) {
  switch (level) {
  case Global:
    getEmulationSettingsManager()->setGlobalValue(key.toStdString(),
                                                  value.toStdString());
    if (key == "rewind-enabled") {
      emit rewindEnabledChanged();
    } else if (key == "picture-mode") {
      emit pictureModeChanged();
    }
    emit settingChanged(key, value);
    return;
  case Platform:
    getEmulationSettingsManager()->setPlatformValue(
        m_platformId, key.toStdString(), value.toStdString());
    if (key == "rewind-enabled") {
      emit rewindEnabledChanged();
    } else if (key == "picture-mode") {
      emit pictureModeChanged();
    }
    emit settingChanged(key, value);
    return;
  case Game:
    getEmulationSettingsManager()->setGameValue(m_contentHash.toStdString(),
                                                m_platformId, key.toStdString(),
                                                value.toStdString());
    if (key == "rewind-enabled") {
      emit rewindEnabledChanged();
    } else if (key == "picture-mode") {
      emit pictureModeChanged();
    }
    emit settingChanged(key, value);
    return;
  }
}

bool GameSettingsItem::valueIsOverridingHigherLevel(SettingsLevel level,
                                                    const QString &key) const {

  switch (level) {
  case Global:
    return false;
  case Platform: {
    auto platformValue = getEmulationSettingsManager()->getPlatformValue(
        m_platformId, key.toStdString());
    auto globalValue =
        getEmulationSettingsManager()->getGlobalValue(key.toStdString());

    if (platformValue.has_value() && globalValue.has_value()) {
      return platformValue.value() != globalValue.value();
    }

    return false;
  }
  case Game: {
    auto gameValue = getEmulationSettingsManager()->getPlatformValue(
        m_platformId, key.toStdString());
    auto platformValue =
        getEmulationSettingsManager()->getGlobalValue(key.toStdString());

    if (gameValue.has_value() && platformValue.has_value()) {
      return gameValue.value() != platformValue.value();
    }

    return false;
  }
  default:
    return {};
  }
}

void GameSettingsItem::resetValue(const QString &key) {}
} // namespace firelight::settings
