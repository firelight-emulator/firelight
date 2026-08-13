#include "cli/rom_launch.hpp"

#include <firelight/library/content_identifier.hpp>
#include <firelight/library/user_library_service.hpp>

#include <QFileInfo>
#include <QString>
#include <spdlog/spdlog.h>

namespace firelight::cli {

int resolveRomEntryId(const std::string &romPath, library::UserLibraryService &library,
                      platforms::IPlatformService &platformService) {
  if (romPath.empty()) {
    return -1;
  }

  const auto qPath = QString::fromStdString(romPath);
  if (!QFileInfo::exists(qPath)) {
    spdlog::warn("ROM path does not exist: {}", romPath);
    return -1;
  }

  const library::ContentIdentifier identifier(platformService);
  const auto identified = identifier.identify(romPath);
  if (!identified.isIdentified()) {
    spdlog::warn("Could not identify content at: {}", romPath);
    return -1;
  }

  const auto entry = library.getEntryWithContentHash(identified.contentHash);
  if (!entry) {
    spdlog::warn("ROM is not in the library yet (scan or import it first): {}", romPath);
    return -1;
  }

  spdlog::info("Launching library entry {} for ROM {}", entry->id, romPath);
  return entry->id;
}

} // namespace firelight::cli
