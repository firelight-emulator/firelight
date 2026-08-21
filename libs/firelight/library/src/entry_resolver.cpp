#include <firelight/library/disc_set_playlist.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/user_library_repository.hpp>

#include <filesystem>
#include <spdlog/spdlog.h>

namespace firelight::library {

bool EntryResolver::contentFileExists(const ContentFile &info) {
  const auto &path = info.m_inArchive ? info.m_archivePathName : info.m_filePath;
  std::error_code ec;
  return !path.empty() && std::filesystem::exists(path, ec);
}

int EntryResolver::scoreConfig(const RunConfiguration &config, const ContentFile &contentFile) {
  int score = 0;
  if (contentFileExists(contentFile)) {
    score += 2;
  }
  if (config.type == RunConfiguration::TYPE_PATCH && config.patchId != -1) {
    score += 1;
  }

  // A playlist reaches every disc of the set, so leaving this to the tie-break would let the
  // order files were scanned in decide whether the game launches whole or as disc one alone
  if (config.type == RunConfiguration::TYPE_PLAYLIST) {
    score += 4;
  }

  // TODO
  // A way in the set owns outranks one that happens to point at a playlist file, so which of
  // the two the resolver picks does not depend on which was recorded first
  if (config.discSetId.has_value()) {
    score += 1;
  }

  return score;
}

EntryResolver::EntryResolver(IUserLibraryRepository &library, std::string appDataDirectory)
    : m_library(library), m_appDataDirectory(std::move(appDataDirectory)) {}

ResolvedContent EntryResolver::resolve(const Entry &entry) const {
  const auto runConfigs = m_library.getRunConfigurations(entry.contentHash);
  if (runConfigs.empty()) {
    return {};
  }

  // Pick the highest-scoring configuration, breaking ties toward the most
  // recently added (largest id) for determinism
  const RunConfiguration *best = nullptr;
  ContentFile bestContentFile;
  int bestScore = -1;

  for (const auto &config : runConfigs) {
    auto contentFile = m_library.getContentFile(config.contentFileId);
    if (!contentFile.has_value()) {
      continue;
    }

    // TODO
    // A set launches through the playlist naming every disc, while the row is anchored on the
    // disc the identity comes from. The hash stays the anchor's; only the path moves
    if (config.discSetId.has_value()) {
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

    const int score = scoreConfig(config, *contentFile);
    if (score > bestScore || (score == bestScore && best && config.id > best->id)) {
      bestScore = score;
      best = &config;
      bestContentFile = *contentFile;
    }
  }

  if (!best) {
    spdlog::warn("[EntryResolver] No usable content for entry {} ({})", entry.id, entry.contentHash);
    return {};
  }

  ResolvedContent resolved;
  resolved.valid = true;
  resolved.contentFile = bestContentFile;
  resolved.discSetId = best->discSetId;
  if (best->type == RunConfiguration::TYPE_PATCH && best->patchId != -1) {
    resolved.patch = m_library.getPatchFile(best->patchId);
  }
  return resolved;
}

} // namespace firelight::library
