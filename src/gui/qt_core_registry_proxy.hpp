#pragma once

#include <QObject>
#include <QVariantList>

namespace firelight::gui {

// TODO
// QML access to the core registry + the per-platform/per-game core override
// (stored under the reserved "core" setting key). Levels use SettingsLevel
// ints: 0 = Game, 1 = Platform, 2 = Global.
class QtCoreRegistryProxy : public QObject {
  Q_OBJECT
public:
  explicit QtCoreRegistryProxy(QObject *parent = nullptr);

  // Cores that can run a platform, default first: [{id, name, isDefault}]
  Q_INVOKABLE QVariantList coresForPlatform(int platformId) const;

  // The effective core for a scope (default -> platform -> game override)
  Q_INVOKABLE QString resolvedCore(int platformId,
                                   const QString &contentHash) const;

  // The core explicitly set at a tier ("" if none / inheriting)
  Q_INVOKABLE QString coreOverride(int level, int platformId,
                                   const QString &contentHash) const;

  Q_INVOKABLE void setCoreOverride(int level, int platformId,
                                   const QString &contentHash,
                                   const QString &coreId);
  Q_INVOKABLE void clearCoreOverride(int level, int platformId,
                                     const QString &contentHash);

  Q_INVOKABLE QString displayNameFor(const QString &coreId) const;
};

} // namespace firelight::gui
