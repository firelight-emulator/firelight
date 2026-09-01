// TODO: NEEDS REVIEW
#include <firelight/library/disc_set_playlist.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/user_library_repository.hpp>

#include <algorithm>
#include <filesystem>
#include <optional>
#include <spdlog/spdlog.h>
#include <tuple>

namespace firelight::library {

bool EntryResolver::contentFileExists(const ContentFile &info) {
  const auto &path = info.m_inArchive ? info.m_archivePathName : info.m_filePath;
  std::error_code ec;
  return !path.empty() && std::filesystem::exists(path, ec);
}

EntryResolver::EntryResolver(IUserLibraryRepository &library, std::string appDataDirectory)
    : m_library(library), m_appDataDirectory(std::move(appDataDirectory)) {}

ResolvedContent EntryResolver::resolve(const Entry &entry) const {
  auto runConfigs = m_library.getRunConfigurations(entry.contentHash);

  if (runConfigs.empty()) {
    return {};
  }

  // TODO
  // The one the entry launches through comes first, and the rest are ordered by id so a way in
  // being recorded before another cannot decide which is picked
  std::ranges::sort(runConfigs, [](const RunConfiguration &left, const RunConfiguration &right) {
    return std::tie(right.isDefault, left.id) < std::tie(left.isDefault, right.id);
  });

  // TODO
  // A file the library still believes in but that is not on disk is worth resolving to: the loader
  // says what is wrong with it, where refusing here would only say the game does not exist
  std::optional<ResolvedContent> fallback;

  for (const auto &config : runConfigs) {
    auto contentFile = m_library.getContentFile(config.contentFileId);

    if (!contentFile.has_value()) {
      continue;
    }

    // TODO
    // Whether this way in launches through the set's playlist rather than the file it points at
    const auto usesPlaylist = config.discSetId.has_value() && m_library.getDiscsInSet(*config.discSetId).size() > 1;

    // TODO
    // A set launches through the playlist naming every disc, while the row is anchored on the
    // disc the identity comes from. The hash stays the anchor's; only the path moves
    if (usesPlaylist) {
      contentFile->m_filePath = playlistPathFor(config.contentHash, m_appDataDirectory);
      contentFile->m_inArchive = false;
      contentFile->m_type = ContentType::Disc;
    } else if (contentFile->m_missingSince != 0) {
      // TODO
      // The way in outlives the file going away, so without this a second copy being gone is enough
      // to hand the core a path that is not there. A set is judged on its playlist instead, which
      // names only the discs still on disk
      continue;
    }

    ResolvedContent resolved;
    resolved.valid = true;
    resolved.contentFile = *contentFile;
    resolved.discSetId = usesPlaylist ? config.discSetId : std::nullopt;

    if (config.patchId != -1) {
      resolved.patch = m_library.getPatchFile(config.patchId);
    }

    if (usesPlaylist || contentFileExists(*contentFile)) {
      return resolved;
    }

    if (!fallback.has_value()) {
      fallback = resolved;
    }
  }

  if (fallback.has_value()) {
    return *fallback;
  }

  spdlog::warn("[EntryResolver] No usable content for entry {} ({})", entry.id, entry.contentHash);
  return {};
}

} // namespace firelight::library
