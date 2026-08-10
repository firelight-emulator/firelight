#include "qt_variant_group_proxy.hpp"

#include <library/variant_group_service.hpp>

namespace firelight::gui {

QtVariantGroupProxy::QtVariantGroupProxy(library::VariantGroupService &service, QObject *parent)
    : QObject(parent), m_service(service) {}

int QtVariantGroupProxy::createGroup(const QVariantList &entryIds) {
  std::vector<int> ids;
  ids.reserve(entryIds.size());

  for (const auto &entryId : entryIds) {
    ids.push_back(entryId.toInt());
  }

  return m_service.createGroupFrom(ids).value_or(-1);
}

bool QtVariantGroupProxy::addToGroup(const int entryId, const int groupId) {
  return m_service.addToGroup(entryId, groupId);
}

bool QtVariantGroupProxy::removeFromGroup(const int entryId) { return m_service.removeFromGroup(entryId); }

bool QtVariantGroupProxy::clearUserChoice(const int entryId) { return m_service.clearUserChoice(entryId); }

bool QtVariantGroupProxy::pinPrimary(const int groupId, const int entryId) {
  return m_service.pinPrimary(groupId, entryId);
}

bool QtVariantGroupProxy::unpinPrimary(const int groupId) { return m_service.unpinPrimary(groupId); }

bool QtVariantGroupProxy::setTitle(const int groupId, const QString &title) {
  return m_service.setTitle(groupId, title.toStdString());
}

bool QtVariantGroupProxy::setAutoLaunchPrimary(const int groupId, const bool autoLaunch) {
  return m_service.setAutoLaunchPrimary(groupId, autoLaunch);
}

int QtVariantGroupProxy::resolvePrimary(const int groupId) { return m_service.resolvePrimary(groupId).value_or(-1); }

} // namespace firelight::gui
