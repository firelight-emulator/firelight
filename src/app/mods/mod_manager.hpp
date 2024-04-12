#pragma once

#include <firelight/content_database.hpp>
#include <firelight/library_database.hpp>

namespace firelight::mods {

class ModManager {
public:
  ModManager(db::IContentDatabase &contentDatabase,
             db::ILibraryDatabase &libraryDatabase)
      : m_contentDatabase(contentDatabase), m_libraryDatabase(libraryDatabase) {
  }
  void installLatest(int modId, int romId) const;

private:
  db::IContentDatabase &m_contentDatabase;
  db::ILibraryDatabase &m_libraryDatabase;
};

} // namespace firelight::mods
