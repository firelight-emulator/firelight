#include "qt_core_registry_proxy.hpp"

#include "libretro/core_registry.hpp"

#include <firelight/settings/settings_service.hpp>

namespace firelight::gui {

QtCoreRegistryProxy::QtCoreRegistryProxy(QObject *parent) : QObject(parent) {}

QVariantList QtCoreRegistryProxy::coresForPlatform(const int platformId) const {
  QVariantList result;
  for (const auto &core : CoreRegistry::instance().coresForPlatform(platformId)) {
    result.append(QVariantMap{{"id", QString::fromStdString(core.id)},
                              {"name", QString::fromStdString(core.displayName)},
                              {"isDefault", core.isDefault}});
  }
  return result;
}

QString QtCoreRegistryProxy::resolvedCore(const int platformId,
                                          const QString &contentHash) const {
  return QString::fromStdString(CoreRegistry::instance().resolveCoreName(
      platformId, contentHash.toStdString(),
      settings::SettingsService::instance()));
}

QString QtCoreRegistryProxy::coreOverride(const int level, const int platformId,
                                          const QString &contentHash) const {
  auto *settings = settings::SettingsService::instance();
  if (!settings) {
    return {};
  }
  const auto value = settings->getValueAtLevel(
      static_cast<settings::SettingsLevel>(level), contentHash.toStdString(),
      platformId, CoreRegistry::kCoreSettingKey);
  return value ? QString::fromStdString(*value) : QString{};
}

void QtCoreRegistryProxy::setCoreOverride(const int level, const int platformId,
                                          const QString &contentHash,
                                          const QString &coreId) {
  if (auto *settings = settings::SettingsService::instance()) {
    settings->setValueAtLevel(static_cast<settings::SettingsLevel>(level),
                              contentHash.toStdString(), platformId,
                              CoreRegistry::kCoreSettingKey,
                              coreId.toStdString());
  }
}

void QtCoreRegistryProxy::clearCoreOverride(const int level,
                                            const int platformId,
                                            const QString &contentHash) {
  if (auto *settings = settings::SettingsService::instance()) {
    settings->resetValueAtLevel(static_cast<settings::SettingsLevel>(level),
                                contentHash.toStdString(), platformId,
                                CoreRegistry::kCoreSettingKey);
  }
}

QString QtCoreRegistryProxy::displayNameFor(const QString &coreId) const {
  return QString::fromStdString(
      CoreRegistry::instance().displayNameFor(coreId.toStdString()));
}

} // namespace firelight::gui
