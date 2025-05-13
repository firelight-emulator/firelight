#include "game_settings_item.hpp"

namespace firelight::settings {
GameSettingsItem::GameSettingsItem(QQuickItem *parent) : QQuickItem(parent) {}

void GameSettingsItem::setPlatformId(const int platformId) {
  if (m_platformId == platformId) {
    return;
  }

  m_platformId = platformId;
  emit platformIdChanged();
}

int GameSettingsItem::getPlatformId() const { return m_platformId; }

QString GameSettingsItem::getContentHash() { return m_contentHash; }
void GameSettingsItem::setContentHash(const QString &contentHash) {
  if (m_contentHash == contentHash) {
    return;
  }

  m_contentHash = contentHash;
  emit contentHashChanged();
}
QString GameSettingsItem::getPictureMode() {
  return getValue("picture-mode", "aspect-ratio-fill");
}

void GameSettingsItem::setPictureMode(const QString &pictureMode) {
  setValue("picture-mode", pictureMode);
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
  }
}
void GameSettingsItem::resetValue(const QString &key) {}
} // namespace firelight::settings
