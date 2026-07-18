#pragma once

#include <QObject>
#include <QStringList>

namespace firelight::gui {

// TODO
// Read-only view of the settings catalog's layout, for QML. The rows themselves
// come from SettingsModel; this answers the structural question SettingsPage
// asks: "which groups does this page have, and in what order?"
class QtSettingsCatalogProxy : public QObject {
  Q_OBJECT

public:
  explicit QtSettingsCatalogProxy(QObject *parent = nullptr);

  // The page's group ids, in declared order. Empty for an unknown page
  [[nodiscard]] Q_INVOKABLE QStringList groupsForPage(const QString &pageId) const;
};

} // namespace firelight::gui
