#pragma once

#include <firelight/event_dispatcher.hpp>

namespace firelight::library {
class IUserLibraryRepository;

// Owns the scan-time orchestration that turns discovered content files into
// playable library entries: when a content file is added, create its run
// configuration and ensure an entry exists; when the last run configuration for
// a content hash is removed, hide its entry. Keeping this here leaves the
// repository free of domain orchestration. Driven by repository events (see
// library_events.hpp), so it is a plain class — not a QObject.
class LibraryIngestService final {
public:
  explicit LibraryIngestService(IUserLibraryRepository &library);

private:
  IUserLibraryRepository &m_library;

  ScopedConnection m_contentFileAddedConnection;
  ScopedConnection m_runConfigurationCreatedConnection;
  ScopedConnection m_runConfigurationDeletedConnection;
};
} // namespace firelight::library
