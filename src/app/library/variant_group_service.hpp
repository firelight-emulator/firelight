#pragma once

#include <firelight/event_dispatcher.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/library/variant_group.hpp>
#include <firelight/settings/settings_service.hpp>

#include <atomic>
#include <unordered_map>
#include <vector>

namespace firelight::library {

/**
 * Which entry stands for each group, and how many entries each group holds, keyed by group id
 */
struct VariantResolution {
  std::unordered_map<int, int> primaryByGroup;
  std::unordered_map<int, int> memberCountByGroup;
};

/**
 * Keeps each variant group pointed at the entry that should stand for it.
 *
 * Lives here rather than in the library module because the ordering it ranks by is a user setting,
 * and the library does not know about settings. The ranking itself is a pure function in the library
 */
class VariantGroupService {
public:
  /** The setting holding the preferred regions, best first */
  static constexpr auto REGION_PRIORITY_KEY = "library-region-priority";

  /** The setting holding the preferred languages, best first */
  static constexpr auto LANGUAGE_PRIORITY_KEY = "library-language-priority";

  VariantGroupService(UserLibraryService &library, settings::SettingsService &settings);

  ~VariantGroupService() = default;

  /**
   * The entry standing for every group, and how many entries each holds.
   *
   * Answers for the whole library from entries the caller already has, so a model rebuilding
   * itself does not go back to the database once per group
   */
  [[nodiscard]] VariantResolution resolveAll(const std::vector<Entry> &entries,
                                             const std::vector<VariantGroup> &groups) const;

  /**
   * Groups entries that are the same game, titling the set after whichever one comes to stand
   * for it. Refuses entries from more than one platform, and refuses fewer than two
   *
   * @return The new group's id, or nothing when the entries cannot form one
   */
  std::optional<int> createGroupFrom(const std::vector<int> &entryIds);

  /**
   * Puts an entry with the same platform and derived title as another into a group with it,
   * joining the group one of them already has rather than making a second.
   *
   * Only an exact title match counts, so a regional retitle is left for someone to group by hand
   *
   * @return True if the entry ended up in a group it was not already in
   */
  bool autoGroupByTitle(int entryId);

  /**
   * Adds an entry to a group, leaving whichever group it was in. Refuses an entry whose platform
   * differs from the group's
   */
  bool addToGroup(int entryId, int groupId);

  /**
   * Removes an entry from its group, dissolving the group if too little of it is left.
   *
   * Records that a person decided this, so automatic grouping will not put it back
   */
  bool removeFromGroup(int entryId);

  /**
   * Forgets that a person decided this entry's membership, letting automatic grouping have it
   * again, and groups it now if it matches something
   */
  bool clearUserChoice(int entryId);

  /**
   * Whether launching the group goes straight to its primary instead of asking
   */
  bool setAutoLaunchPrimary(int groupId, bool autoLaunch);

  /**
   * Pins the entry that stands for a group, so the ordering stops choosing it
   */
  bool pinPrimary(int groupId, int entryId);

  /**
   * Hands the choice of primary back to the ordering
   */
  bool unpinPrimary(int groupId);

  /**
   * Renames a group, and stops the title following the primary
   */
  bool setTitle(int groupId, const std::string &title);

  /**
   * The entry that should stand for a group right now.
   *
   * A pin wins while what it points at can be launched; when that entry is hidden the group falls to
   * the next best instead. This is what a caller should ask, because the stored primary is the user's
   * choice rather than the currently usable one
   */
  [[nodiscard]] std::optional<int> resolvePrimary(int groupId);

  /**
   * Restores the stored primary to whatever the ordering now prefers. Does nothing to a group whose
   * primary the user pinned
   */
  void reevaluate(int groupId);

  /**
   * Re-picks every group whose primary the user did not pin
   */
  void reevaluateAll();

  /**
   * Removes a group that no longer holds more than one entry, clearing the survivor's membership.
   *
   * @return True if the group was dissolved
   */
  bool dissolveIfUndersized(int groupId);

private:
  /** The ordering as the settings currently hold it */
  [[nodiscard]] VariantPreference currentPreference() const;

  /** The group an entry belongs to, or nothing when it belongs to none */
  [[nodiscard]] std::optional<int> groupOf(int entryId) const;

  /** Names the group after whatever stands for it, unless the user typed a name */
  void retitleFromPrimary(int groupId);

  /**
   * Does the work of the two grouping paths.
   *
   * @param isUserChoice Whether to record the membership as decided by a person, which is what
   *   makes it survive automatic grouping
   */
  std::optional<int> createGroup(const std::vector<int> &entryIds, bool isUserChoice);

  bool attachToGroup(int entryId, int groupId, bool isUserChoice);

  UserLibraryService &m_library;
  settings::SettingsService &m_settings;

  // TODO
  // Set while a write of this service's own is in flight, so the event that write
  // publishes does not start another round. Per thread, because a write on one thread says
  // nothing about whether an event arriving on another is re-entrant
  static thread_local bool s_applying;

  ScopedConnection m_settingChangedConnection;
  ScopedConnection m_entryUpdatedConnection;
};

} // namespace firelight::library
