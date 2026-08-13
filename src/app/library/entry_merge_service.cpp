#include "entry_merge_service.hpp"

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
}

} // namespace firelight::library
