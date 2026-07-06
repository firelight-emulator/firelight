#pragma once

#include <string>

namespace firelight::library {
class UserLibraryService;
}

namespace firelight::platforms {
class IPlatformService;
}

namespace firelight::cli {

// Resolves a ROM file path to a library entry id: hashes the file with the same
// identifier the scanner uses, then looks up the entry with that content hash.
// Returns -1 (and logs why) if the path is missing, unrecognized, or not yet in
// the library (it must be scanned/imported first).
int resolveRomEntryId(const std::string &romPath,
                      library::UserLibraryService &library,
                      platforms::IPlatformService &platformService);

} // namespace firelight::cli
