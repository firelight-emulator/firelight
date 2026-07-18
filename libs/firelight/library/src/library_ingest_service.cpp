#include <firelight/library/library_ingest_service.hpp>

#include <firelight/library/library_events.hpp>
#include <firelight/library/user_library_repository.hpp>

namespace firelight::library {

LibraryIngestService::LibraryIngestService(IUserLibraryRepository &library)
    : m_library(library) {
  // TODO
  // A newly added content file gets a run configuration. These events are
  // published on the scanner thread, so the handlers run synchronously there
  // (the per-thread database connection makes this safe)
  m_contentFileAddedConnection =
      EventDispatcher::instance().subscribe<ContentFileAddedEvent>(
          [this](const ContentFileAddedEvent &event) {
            m_library.createRunConfiguration(event.id, event.filePath,
                                             event.platformId,
                                             event.contentHash);
          });

  // A run configuration implies a playable entry: create it, or unhide an
  // existing one for the same content
  m_runConfigurationCreatedConnection =
      EventDispatcher::instance().subscribe<RunConfigurationCreatedEvent>(
          [this](const RunConfigurationCreatedEvent &event) {
            if (auto entry =
                    m_library.getEntryWithContentHash(event.contentHash)) {
              if (entry->hidden) {
                entry->hidden = false;
                m_library.update(*entry);
              }
            } else {
              // Display name is the final path segment (basename)
              const auto slash = event.filePath.find_last_of('/');
              auto newEntry = Entry{
                  .displayName = slash == std::string::npos
                                     ? event.filePath
                                     : event.filePath.substr(slash + 1),
                  .contentHash = event.contentHash,
                  .platformId = static_cast<unsigned>(event.platformId),
              };
              m_library.createEntry(newEntry);
            }
          });

  // When the last run configuration for a content hash is removed, hide its
  // entry so it disappears from the library without losing user state
  m_runConfigurationDeletedConnection =
      EventDispatcher::instance().subscribe<RunConfigurationDeletedEvent>(
          [this](const RunConfigurationDeletedEvent &event) {
            if (const auto runConfigs =
                    m_library.getRunConfigurations(event.contentHash);
                runConfigs.empty()) {
              if (auto entry =
                      m_library.getEntryWithContentHash(event.contentHash)) {
                entry->hidden = true;
                m_library.update(*entry);
              }
            }
          });
}

} // namespace firelight::library
