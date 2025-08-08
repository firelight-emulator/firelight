#include "shortcut_mapping.hpp"

namespace firelight::input {

ShortcutMapping::ShortcutMapping(const int id, const int profileId)
    : m_id(id), m_profileId(profileId) {
  m_mappings.emplace(OpenRewindMenu, InputSequence{{LeftTrigger}, {WestFace}});
}

int ShortcutMapping::getId() const { return m_id; }

int ShortcutMapping::getProfileId() const { return m_profileId; }

std::map<Shortcut, InputSequence> ShortcutMapping::getMappings() const {
  return m_mappings;
}

} // namespace firelight::input