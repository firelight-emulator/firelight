#pragma once

#include <firelight/activity/activity_log.hpp>
#include <firelight/event_dispatcher.hpp>
#include <firelight/saves/isave_manager.hpp>

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
  EntryMergeService(saves::ISaveManager &saveManager, activity::IActivityLog &activityLog);

  ~EntryMergeService() = default;

private:
  saves::ISaveManager &m_saveManager;
  activity::IActivityLog &m_activityLog;

  ScopedConnection m_entryAbsorbedConnection;
};

} // namespace firelight::library
