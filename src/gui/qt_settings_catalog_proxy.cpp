#include "qt_settings_catalog_proxy.hpp"

#include <firelight/settings/settings_catalog.hpp>

namespace firelight::gui {

QtSettingsCatalogProxy::QtSettingsCatalogProxy(QObject *parent) : QObject(parent) {}

QStringList QtSettingsCatalogProxy::groupsForPage(const QString &pageId) const {
  const auto &catalog = settings::SettingsCatalog::instance();
  const auto *page = catalog.findPage(pageId.toStdString());

  if (page == nullptr) {
    return {};
  }

  QStringList ids;

  // TODO
  // Only groups the catalog declares, in the order the page lists them
  for (const auto &groupId : page->groupIds) {
    if (catalog.findGroup(groupId) != nullptr) {
      ids.append(QString::fromStdString(groupId));
    }
  }

  return ids;
}

} // namespace firelight::gui
