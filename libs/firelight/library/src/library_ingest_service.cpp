// TODO: NEEDS REVIEW
#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/library_events.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/user_library_repository.hpp>

#include <algorithm>
#include <utility>

namespace firelight::library {

namespace {
// Holds the re-entrancy flag for the length of a write, restoring whatever it was so a nested call
// does not clear it on the way out
class ApplyGuard {
public:
  explicit ApplyGuard(bool &flag) : m_flag(flag), m_previous(std::exchange(flag, true)) {}

  ~ApplyGuard() { m_flag = m_previous; }

private:
  bool &m_flag;
  bool m_previous;
};
} // namespace

thread_local bool LibraryIngestService::s_applying = false;

LibraryIngestService::LibraryIngestService(IUserLibraryRepository &library, DiscSetService &discSets)
    : m_library(library), m_discSets(discSets) {
  // A newly added content file gets a run configuration. These events are
  // published on the scanner thread, so the handlers run synchronously there
  // (the per-thread database connection makes this safe)
  m_contentFileAddedConnection =
      EventDispatcher::instance().subscribe<ContentFileAddedEvent>([this](const ContentFileAddedEvent &event) {
        const auto file = m_library.getContentFile(event.id);

        // TODO
        // A disc finds its set first, because the set is what decides how it launches. Only the
        // disc a set is identified by keeps a way in; the rest are reached through it
        if (file.has_value()) {
          if (const auto setId = m_discSets.place(*file)) {
            m_discSets.syncSetWayIn(*setId);
            return;
          }
        }

        m_library.createRunConfiguration(event.id, event.filePath, event.platformId, event.contentHash);
      });

  // A run configuration implies a library entry, so create one for content nothing stands for yet
  m_runConfigurationCreatedConnection = EventDispatcher::instance().subscribe<RunConfigurationCreatedEvent>(
      [this](const RunConfigurationCreatedEvent &event) {
        if (const auto entry = m_library.getEntryWithContentHash(event.contentHash)) {
          EventDispatcher::instance().publish(EntryUpdatedEvent{.entryId = entry->id});
        } else {
          // Display name is the final path segment (basename)
          const auto slash = event.filePath.find_last_of('/');
          auto newEntry = Entry{
              .displayName = slash == std::string::npos ? event.filePath : event.filePath.substr(slash + 1),
              .contentHash = event.contentHash,
              .platformId = static_cast<unsigned>(event.platformId),
          };
          m_library.createEntry(newEntry);
        }
      });

  // Losing a way in changes what the entry can do, and the grid only learns that from an entry event
  m_runConfigurationDeletedConnection = EventDispatcher::instance().subscribe<RunConfigurationDeletedEvent>(
      [this](const RunConfigurationDeletedEvent &event) {
        if (const auto entry = m_library.getEntryWithContentHash(event.contentHash)) {
          EventDispatcher::instance().publish(EntryUpdatedEvent{.entryId = entry->id});
        }
      });

  // A file going away leaves its ways in alone, so nothing else would say the entry changed
  m_contentFileMissingConnection = EventDispatcher::instance().subscribe<ContentFileMissingEvent>(
      [this](const ContentFileMissingEvent &event) { announceContentChange(event.contentHash, event.discSetId); });

  m_contentFileRestoredConnection = EventDispatcher::instance().subscribe<ContentFileRestoredEvent>(
      [this](const ContentFileRestoredEvent &event) { announceContentChange(event.contentHash, event.discSetId); });
}

void LibraryIngestService::announceContentChange(const std::string &contentHash, const std::optional<int> discSetId) {
  if (s_applying) {
    return;
  }

  const ApplyGuard guard(s_applying);

  if (const auto entry = m_library.getEntryWithContentHash(contentHash)) {
    EventDispatcher::instance().publish(EntryUpdatedEvent{.entryId = entry->id});
  }

  // An absorbed disc has no entry of its own, so nothing else would tell the set its disc count
  // changed
  if (discSetId.has_value()) {
    for (const auto &entry : m_library.getEntriesInDiscSet(*discSetId)) {
      EventDispatcher::instance().publish(EntryUpdatedEvent{.entryId = entry.id});
    }
  }
}

} // namespace firelight::library
