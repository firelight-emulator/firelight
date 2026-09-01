// TODO: NEEDS REVIEW
#include "qt_disc_set_proxy.hpp"

#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/user_library_repository.hpp>

namespace firelight::gui {

QtDiscSetProxy::QtDiscSetProxy(library::DiscSetService &service, library::IUserLibraryRepository &library,
                               QObject *parent)
    : QObject(parent), m_service(service), m_library(library) {}

bool QtDiscSetProxy::setDiscNumber(const int memberId, const int discNumber) {
  auto member = m_library.getDiscSetMember(memberId);

  if (!member.has_value() || discNumber < 1) {
    return false;
  }

  // A number somebody typed is the strongest thing anything has said about where this disc sits
  member->m_discNumber = discNumber;
  member->m_source = library::DiscSource::User;
  member->m_isUncertain = false;

  if (!m_library.create(*member)) {
    return false;
  }

  // TODO
  // Renumbering can make a different disc the lowest, which moves what the set is identified by
  m_service.syncSetWayIn(member->m_discSetId);
  emit setsChanged();
  return true;
}

bool QtDiscSetProxy::removeFromSet(const int contentFileId) {
  if (!m_service.detachDisc(contentFileId)) {
    return false;
  }

  emit setsChanged();
  return true;
}

bool QtDiscSetProxy::clearUserChoice(const int entryId) {
  if (!m_service.clearUserChoice(entryId)) {
    return false;
  }

  emit setsChanged();
  return true;
}

bool QtDiscSetProxy::confirmPlacement(const int memberId) {
  auto member = m_library.getDiscSetMember(memberId);

  if (!member.has_value()) {
    return false;
  }

  member->m_source = library::DiscSource::User;
  member->m_isUncertain = false;

  if (!m_library.create(*member)) {
    return false;
  }

  emit setsChanged();
  return true;
}

} // namespace firelight::gui
