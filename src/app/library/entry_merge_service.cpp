// TODO: NEEDS REVIEW
#include "entry_merge_service.hpp"

#include <firelight/library/library_events.hpp>
#include <firelight/library/user_library_repository.hpp>

#include <spdlog/spdlog.h>

namespace firelight::library {

EntryMergeService::EntryMergeService(saves::ISaveManager &saveManager, activity::IActivityLog &activityLog)
    : m_saveManager(saveManager), m_activityLog(activityLog) {
  m_entryAbsorbedConnection =
      EventDispatcher::instance().subscribe<EntryAbsorbedEvent>([this](const EntryAbsorbedEvent &event) {
        if (event.absorbedContentHash == event.survivingContentHash) {
          return;
        }

        const auto movedSaves = m_saveManager.transferSaves(event.absorbedContentHash, event.survivingContentHash);
        const auto movedSessions =
            m_activityLog.transferSessions(event.absorbedContentHash, event.survivingContentHash);

        if (movedSaves || movedSessions) {
          spdlog::info("Moved saves({}) and playtime({}) from {} to entry {}", movedSaves, movedSessions,
                       event.absorbedContentHash, event.survivingEntryId);
        }
      });

  // TODO
  // A set moving onto a lower-numbered disc is the same move under a different name: what was
  // keyed on the old dump has to follow the entry to the new one
  m_entryIdentityChangedConnection =
      EventDispatcher::instance().subscribe<EntryIdentityChangedEvent>([this](const EntryIdentityChangedEvent &event) {
        if (event.previousContentHash.empty() || event.previousContentHash == event.contentHash) {
          return;
        }

        const auto movedSaves = m_saveManager.transferSaves(event.previousContentHash, event.contentHash);
        const auto movedSessions = m_activityLog.transferSessions(event.previousContentHash, event.contentHash);

        if (movedSaves || movedSessions) {
          spdlog::info("Moved saves({}) and playtime({}) from {} to entry {}", movedSaves, movedSessions,
                       event.previousContentHash, event.entryId);
        }
      });
}

} // namespace firelight::library
