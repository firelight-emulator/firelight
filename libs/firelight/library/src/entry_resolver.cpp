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

int EntryResolver::scoreConfig(const RunConfiguration &config,
                               const ContentFile &contentFile) {
  int score = 0;
  if (contentFileExists(contentFile)) {
    score += 2;
  }
  if (config.type == RunConfiguration::TYPE_PATCH && config.patchId != -1) {
    score += 1;
  }
  return score;
}

EntryResolver::EntryResolver(IUserLibraryRepository &library) : m_library(library) {}

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
    const auto contentFile = m_library.getContentFile(config.contentFileId);
    if (!contentFile.has_value()) {
      continue;
    }

    const int score = scoreConfig(config, *contentFile);
    if (score > bestScore ||
        (score == bestScore && best && config.id > best->id)) {
      bestScore = score;
      best = &config;
      bestContentFile = *contentFile;
    }
  }

  if (!best) {
    spdlog::warn("[EntryResolver] No usable content for entry {} ({})", entry.id,
                 entry.contentHash);
    return {};
  }

  ResolvedContent resolved;
  resolved.valid = true;
  resolved.contentFile = bestContentFile;
  if (best->type == RunConfiguration::TYPE_PATCH && best->patchId != -1) {
    resolved.patch = m_library.getPatchFile(best->patchId);
  }
  return resolved;
}

} // namespace firelight::library
