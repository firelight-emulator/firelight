#include "qt_settings_catalog_proxy.hpp"

#include <firelight/settings/settings_catalog.hpp>

namespace firelight::gui {

QtSettingsCatalogProxy::QtSettingsCatalogProxy(QObject *parent)
    : QObject(parent) {}

QStringList
QtSettingsCatalogProxy::groupsForPage(const QString &pageId) const {
  QStringList ids;
  // groups() is already sorted by `order`, so declaration order is the render
  // order and nothing here re-sorts.
  for (const auto &group : settings::SettingsCatalog::instance().groups()) {
    if (group.pageId == pageId.toStdString()) {
      ids.append(QString::fromStdString(group.id));
    }
  }
  return ids;
}

} // namespace firelight::gui
