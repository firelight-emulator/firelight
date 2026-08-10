#include "variant_group_service.hpp"

#include <spdlog/spdlog.h>

namespace firelight::library {

VariantGroupService::VariantGroupService(UserLibraryService &library, settings::SettingsService &settings)
    : m_library(library), m_settings(settings) {
  m_settingChangedConnection = EventDispatcher::instance().subscribe<settings::GlobalSettingChangedEvent>(
      [this](const settings::GlobalSettingChangedEvent &event) {
        if (event.key == REGION_PRIORITY_KEY || event.key == LANGUAGE_PRIORITY_KEY) {
          reevaluateAll();
        }
      });

  // A scrape filling in regions, a rename, or a file going missing all change which
  // entry should stand for the group
  m_entryUpdatedConnection =
      EventDispatcher::instance().subscribe<EntryUpdatedEvent>([this](const EntryUpdatedEvent &event) {
        // TODO
        // Everything below writes, and a write publishes this event again. The guard is
        // what keeps that one level deep
        if (m_applying) {
          return;
        }

        m_applying = true;

        if (const auto groupId = groupOf(event.entryId)) {
          reevaluate(*groupId);
        } else {
          autoGroupByTitle(event.entryId);
        }

        m_applying = false;
      });
}

namespace {
// TODO
// The pin holds for as long as what it points at can be launched. When it cannot, the group
// falls to the next best rather than standing for something unplayable
std::optional<int> pickFrom(const VariantGroup &group, const std::vector<VariantCandidate> &candidates,
                            const VariantPreference &preference) {
  if (group.primaryUserSet && group.primaryEntryId.has_value()) {
    const auto pinned = std::find_if(candidates.begin(), candidates.end(), [&](const VariantCandidate &candidate) {
      return candidate.entryId == *group.primaryEntryId && !candidate.hidden;
    });

    if (pinned != candidates.end()) {
      return group.primaryEntryId;
    }
  }

  return choosePrimaryEntry(candidates, preference);
}

VariantCandidate candidateFor(const Entry &entry) {
  return VariantCandidate{
      .entryId = entry.id,
      .regions = entry.metadata.regions,
      .languages = entry.metadata.languages,
      .revision = entry.metadata.revision,
      .flags = entry.metadata.flags,
      .hidden = entry.hidden,
  };
}
} // namespace

std::optional<int> VariantGroupService::createGroupFrom(const std::vector<int> &entryIds) {
  return createGroup(entryIds, true);
}

std::optional<int> VariantGroupService::createGroup(const std::vector<int> &entryIds, const bool isUserChoice) {
  if (entryIds.size() < 2) {
    return std::nullopt;
  }

  std::vector<Entry> members;
  members.reserve(entryIds.size());

  for (const auto entryId : entryIds) {
    const auto entry = m_library.getEntry(entryId);

    if (!entry.has_value()) {
      return std::nullopt;
    }

    if (!members.empty() && entry->platformId != members.front().platformId) {
      spdlog::warn("Refusing to group entry {} with a different platform", entryId);
      return std::nullopt;
    }

    members.push_back(*entry);
  }

  VariantGroup group;
  group.title = members.front().displayName;

  if (!m_library.createVariantGroup(group)) {
    return std::nullopt;
  }

  for (const auto &member : members) {
    m_library.setEntryVariantGroup(member.id, group.id, isUserChoice);
  }

  reevaluate(group.id);
  // TODO
  // Titled once, from whichever entry came to stand for the set. It does not follow the
  // primary after this
  retitleFromPrimary(group.id);
  EventDispatcher::instance().publish(VariantGroupUpdatedEvent{.groupId = group.id});
  return group.id;
}

bool VariantGroupService::autoGroupByTitle(const int entryId) {
  const auto entry = m_library.getEntry(entryId);

  if (!entry.has_value() || entry->normalizedTitle.empty() || entry->variantGroupId.has_value()) {
    return false;
  }

  if (entry->variantGroupUserSet) {
    return false;
  }

  const auto peerIds = m_library.getEntryIdsWithNormalizedTitle(entry->platformId, entry->normalizedTitle);

  std::vector<int> ungrouped;
  std::optional<int> existingGroup;

  for (const auto peerId : peerIds) {
    if (peerId == entryId) {
      continue;
    }

    const auto peer = m_library.getEntry(peerId);

    if (!peer.has_value()) {
      continue;
    }

    // TODO
    // Automatic grouping only ever touches entries nobody has decided about, and only joins
    // sets it could have built itself. Without this a peer somebody took out is pulled back in
    // the next time one of its siblings changes
    if (peer->variantGroupUserSet) {
      continue;
    }

    // TODO
    // Discs of one game share a title once the disc tag is stripped, so without this they
    // read as alternate releases of each other and all but one disappears behind a primary
    if (peer->discNumber != entry->discNumber) {
      continue;
    }

    // TODO
    // Joining the group one of them already has is what stops a third release making a
    // second group beside the one holding the first two
    if (peer->variantGroupId.has_value()) {
      existingGroup = peer->variantGroupId;
      break;
    }

    ungrouped.push_back(peerId);
  }

  if (existingGroup.has_value()) {
    return attachToGroup(entryId, *existingGroup, false);
  }

  if (ungrouped.empty()) {
    return false;
  }

  ungrouped.push_back(entryId);
  return createGroup(ungrouped, false).has_value();
}

bool VariantGroupService::addToGroup(const int entryId, const int groupId) {
  return attachToGroup(entryId, groupId, true);
}

bool VariantGroupService::attachToGroup(const int entryId, const int groupId, const bool isUserChoice) {
  const auto entry = m_library.getEntry(entryId);
  const auto members = m_library.getEntriesInVariantGroup(groupId);

  if (!entry.has_value() || members.empty()) {
    return false;
  }

  if (entry->platformId != members.front().platformId) {
    spdlog::warn("Refusing to add entry {} to a group on a different platform", entryId);
    return false;
  }

  const auto previous = entry->variantGroupId;

  if (!m_library.setEntryVariantGroup(entryId, groupId, isUserChoice)) {
    return false;
  }

  if (previous.has_value() && *previous != groupId && !dissolveIfUndersized(*previous)) {
    reevaluate(*previous);
  }

  reevaluate(groupId);
  EventDispatcher::instance().publish(VariantGroupUpdatedEvent{.groupId = groupId});
  return true;
}

bool VariantGroupService::removeFromGroup(const int entryId) {
  const auto groupId = groupOf(entryId);

  if (!groupId.has_value()) {
    return false;
  }

  // TODO
  // Recorded as the user's doing, which is what stops the automatic grouper putting it back
  if (!m_library.setEntryVariantGroup(entryId, std::nullopt, true)) {
    return false;
  }

  if (!dissolveIfUndersized(*groupId)) {
    reevaluate(*groupId);
    EventDispatcher::instance().publish(VariantGroupUpdatedEvent{.groupId = *groupId});
  }

  return true;
}

bool VariantGroupService::setAutoLaunchPrimary(const int groupId, const bool autoLaunch) {
  auto group = m_library.getVariantGroup(groupId);

  if (!group.has_value()) {
    return false;
  }

  group->autoLaunchPrimary = autoLaunch;

  if (!m_library.updateVariantGroup(*group)) {
    return false;
  }

  EventDispatcher::instance().publish(VariantGroupUpdatedEvent{.groupId = groupId});
  return true;
}

bool VariantGroupService::pinPrimary(const int groupId, const int entryId) {
  if (!m_library.setVariantGroupPrimary(groupId, entryId)) {
    return false;
  }

  EventDispatcher::instance().publish(VariantGroupUpdatedEvent{.groupId = groupId});
  return true;
}

bool VariantGroupService::unpinPrimary(const int groupId) {
  if (!m_library.clearVariantGroupPrimary(groupId)) {
    return false;
  }

  reevaluate(groupId);
  EventDispatcher::instance().publish(VariantGroupUpdatedEvent{.groupId = groupId});
  return true;
}

bool VariantGroupService::setTitle(const int groupId, const std::string &title) {
  auto group = m_library.getVariantGroup(groupId);

  if (!group.has_value()) {
    return false;
  }

  group->title = title;
  group->titleUserSet = true;

  if (!m_library.updateVariantGroup(*group)) {
    return false;
  }

  EventDispatcher::instance().publish(VariantGroupUpdatedEvent{.groupId = groupId});
  return true;
}

bool VariantGroupService::clearUserChoice(const int entryId) {
  const auto entry = m_library.getEntry(entryId);

  if (!entry.has_value() || !entry->variantGroupUserSet) {
    return false;
  }

  if (!m_library.setEntryVariantGroup(entryId, entry->variantGroupId, false)) {
    return false;
  }

  autoGroupByTitle(entryId);
  return true;
}

void VariantGroupService::retitleFromPrimary(const int groupId) {
  auto group = m_library.getVariantGroup(groupId);

  if (!group.has_value() || group->titleUserSet) {
    return;
  }

  const auto primary = resolvePrimary(groupId);

  if (!primary.has_value()) {
    return;
  }

  const auto entry = m_library.getEntry(*primary);

  if (!entry.has_value() || entry->displayName == group->title) {
    return;
  }

  group->title = entry->displayName;
  m_library.updateVariantGroup(*group);
}

VariantResolution VariantGroupService::resolveAll(const std::vector<Entry> &entries,
                                                  const std::vector<VariantGroup> &groups) const {
  std::unordered_map<int, std::vector<VariantCandidate>> candidatesByGroup;

  for (const auto &entry : entries) {
    if (entry.variantGroupId.has_value()) {
      candidatesByGroup[*entry.variantGroupId].push_back(candidateFor(entry));
    }
  }

  const auto preference = currentPreference();

  VariantResolution resolution;
  for (const auto &group : groups) {
    const auto found = candidatesByGroup.find(group.id);

    if (found == candidatesByGroup.end()) {
      continue;
    }

    resolution.memberCountByGroup[group.id] = static_cast<int>(found->second.size());

    if (const auto chosen = pickFrom(group, found->second, preference)) {
      resolution.primaryByGroup[group.id] = *chosen;
    }
  }

  return resolution;
}

VariantPreference VariantGroupService::currentPreference() const {
  VariantPreference preference;
  preference.regionOrder = parsePreferenceOrder(m_settings.getGlobalValue(REGION_PRIORITY_KEY).value_or(""));
  preference.languageOrder = parsePreferenceOrder(m_settings.getGlobalValue(LANGUAGE_PRIORITY_KEY).value_or(""));
  return preference;
}

std::optional<int> VariantGroupService::groupOf(const int entryId) const {
  const auto entry = m_library.getEntry(entryId);

  if (!entry.has_value()) {
    return std::nullopt;
  }

  return entry->variantGroupId;
}

std::optional<int> VariantGroupService::resolvePrimary(const int groupId) {
  const auto group = m_library.getVariantGroup(groupId);

  if (!group.has_value()) {
    return std::nullopt;
  }

  const auto members = m_library.getEntriesInVariantGroup(groupId);

  std::vector<VariantCandidate> candidates;
  candidates.reserve(members.size());

  for (const auto &entry : members) {
    candidates.push_back(candidateFor(entry));
  }

  return pickFrom(*group, candidates, currentPreference());
}

void VariantGroupService::reevaluate(const int groupId) {
  auto group = m_library.getVariantGroup(groupId);

  if (!group.has_value()) {
    return;
  }

  // TODO
  // A pin is never overwritten, not even while what it points at is missing.
  // Storing the fallback here would destroy the choice it is standing in for, so
  // the fallback lives only in resolvePrimary
  if (group->primaryUserSet) {
    return;
  }

  const auto chosen = resolvePrimary(groupId);

  if (chosen == group->primaryEntryId) {
    return;
  }

  group->primaryEntryId = chosen;
  m_library.updateVariantGroup(*group);
  EventDispatcher::instance().publish(VariantGroupUpdatedEvent{.groupId = groupId});
}

void VariantGroupService::reevaluateAll() {
  for (const auto &group : m_library.getVariantGroups()) {
    reevaluate(group.id);
  }
}

bool VariantGroupService::dissolveIfUndersized(const int groupId) {
  const auto members = m_library.getEntriesInVariantGroup(groupId);

  if (members.size() > 1) {
    return false;
  }

  for (const auto &entry : members) {
    m_library.setEntryVariantGroup(entry.id, std::nullopt, entry.variantGroupUserSet);
  }

  m_library.deleteVariantGroup(groupId);
  EventDispatcher::instance().publish(VariantGroupUpdatedEvent{.groupId = groupId});
  return true;
}

} // namespace firelight::library
