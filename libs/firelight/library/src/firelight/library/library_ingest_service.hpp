#pragma once

#include <firelight/event_dispatcher.hpp>

#include <optional>
#include <string>

namespace firelight::library {
class IUserLibraryRepository;

// Owns the scan-time orchestration that turns discovered content files into
// playable library entries: when a content file is added, create its run
// configuration and ensure an entry exists; when the last run configuration for
// a content hash is removed, hide its entry. Keeping this here leaves the
// repository free of domain orchestration. Driven by repository events (see
// library_events.hpp), so it is a plain class — not a QObject
class LibraryIngestService final {
public:
  explicit LibraryIngestService(IUserLibraryRepository &library);

private:
  // TODO
  // Whether an entry's visibility is being written on this thread, so the entry event that write
  // publishes does not come back around into another one
  static thread_local bool s_applying;

  // TODO
  // Tells the entry a content file belongs to, and the set it is part of, that what they can do
  // has changed. Availability is read from the files, so nothing is written here
  void announceContentChange(const std::string &contentHash, std::optional<int> discSetId);

  IUserLibraryRepository &m_library;

  ScopedConnection m_contentFileAddedConnection;
  ScopedConnection m_runConfigurationCreatedConnection;
  ScopedConnection m_runConfigurationDeletedConnection;
  ScopedConnection m_contentFileMissingConnection;
  ScopedConnection m_contentFileRestoredConnection;
};
} // namespace firelight::library
