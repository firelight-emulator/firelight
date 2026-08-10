#pragma once

#include <firelight/activity/activity_log.hpp>
#include <firelight/event_dispatcher.hpp>
#include <firelight/library/disc_set_service.hpp>
#include <firelight/saves/isave_manager.hpp>
#include <firelight/settings/settings_service.hpp>

namespace firelight::library {

/**
 * Carries a user's data across when two library entries turn out to be one game.
 *
 * Discs of a multi-disc game arrive as separate entries and are folded into one, which moves
 * the surviving entry's content hash. Saves and playtime are keyed on that hash and live in
 * modules the library does not depend on, so the library announces the merge and this joins
 * the three together
 */
class EntryMergeService {
public:
  static constexpr auto PLAYLIST_LOCATION_KEY = "library-playlist-location";

  EntryMergeService(saves::ISaveManager &saveManager, activity::IActivityLog &activityLog, DiscSetService &discSets,
                    settings::SettingsService &settings);

  ~EntryMergeService() = default;

private:
  /** Points the disc sets at wherever playlists are meant to go */
  void applyPlaylistLocation();

  saves::ISaveManager &m_saveManager;
  activity::IActivityLog &m_activityLog;
  DiscSetService &m_discSets;
  settings::SettingsService &m_settings;

  ScopedConnection m_entryAbsorbedConnection;
  ScopedConnection m_settingChangedConnection;
};

} // namespace firelight::library
