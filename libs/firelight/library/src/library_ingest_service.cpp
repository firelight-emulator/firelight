#include <firelight/library/library_ingest_service.hpp>

#include <firelight/library/library_events.hpp>
#include <firelight/library/user_library_repository.hpp>

namespace firelight::library {

LibraryIngestService::LibraryIngestService(IUserLibraryRepository &library)
    : m_library(library) {
  // A newly added content file gets a run configuration. These events are
  // published on the scanner thread, so the handlers run synchronously there
  // (the per-thread database connection makes this safe).
  m_contentFileAddedConnection =
      EventDispatcher::instance().subscribe<ContentFileAddedEvent>(
          [this](const ContentFileAddedEvent &event) {
            m_library.createRunConfiguration(
                event.id, QString::fromStdString(event.filePath),
                event.platformId,
                QString::fromStdString(event.contentHash));
          });

  // A run configuration implies a playable entry: create it, or unhide an
  // existing one for the same content.
  m_runConfigurationCreatedConnection =
      EventDispatcher::instance().subscribe<RunConfigurationCreatedEvent>(
          [this](const RunConfigurationCreatedEvent &event) {
            const auto contentHash = QString::fromStdString(event.contentHash);
            if (auto entry = m_library.getEntryWithContentHash(contentHash)) {
              if (entry->hidden) {
                entry->hidden = false;
                m_library.update(*entry);
              }
            } else {
              auto newEntry = Entry{
                  .displayName =
                      QString::fromStdString(event.filePath).split("/").last(),
                  .contentHash = contentHash,
                  .platformId = static_cast<unsigned>(event.platformId),
              };
              m_library.createEntry(newEntry);
            }
          });

  // When the last run configuration for a content hash is removed, hide its
  // entry so it disappears from the library without losing user state.
  m_runConfigurationDeletedConnection =
      EventDispatcher::instance().subscribe<RunConfigurationDeletedEvent>(
          [this](const RunConfigurationDeletedEvent &event) {
            const auto contentHash = QString::fromStdString(event.contentHash);
            if (const auto runConfigs =
                    m_library.getRunConfigurations(contentHash);
                runConfigs.empty()) {
              if (auto entry = m_library.getEntryWithContentHash(contentHash)) {
                entry->hidden = true;
                m_library.update(*entry);
              }
            }
          });
}

} // namespace firelight::library
