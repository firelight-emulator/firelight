#include "metadata/metadata_service.hpp"

#include <firelight/library/entry.hpp>
#include <firelight/library/library_events.hpp>
#include <firelight/library/user_library_repository.hpp>
#include <firelight/metadata/art_provider.hpp>
#include <firelight/metadata/game_metadata_source.hpp>
#include <firelight/metadata/media_asset_repository.hpp>

#include <filesystem>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::metadata {

MetadataService::MetadataService(library::IUserLibraryRepository &library, IGameMetadataSource &metadataSource,
                                 IMediaAssetRepository &mediaAssets, std::string mediaDir)
    : m_library(library), m_metadataSource(metadataSource), m_mediaAssets(mediaAssets),
      m_mediaDir(std::move(mediaDir)) {
  // One worker thread: metadata.db and media.db are single-connection, so all
  // access is serialized here regardless of which thread created the entry
  m_pool.setMaxThreadCount(1);
  m_entryCreatedConnection = EventDispatcher::instance().subscribe<library::EntryCreatedEvent>(
      [this](const library::EntryCreatedEvent &event) {
        const int id = event.entryId;
        if (m_shuttingDown) {
          return;
        }
        m_pool.start([this, id] {
          if (!m_shuttingDown) {
            populate(id);
          }
        });
      });
}

MetadataService::~MetadataService() {
  m_shuttingDown = true;
  m_pool.waitForDone();
}

void MetadataService::populate(int entryId) {
  const auto entryOpt = m_library.getEntry(entryId);
  if (!entryOpt.has_value()) {
    return;
  }
  auto entry = *entryOpt;

  const auto metadata = m_metadataSource.lookup(entry.contentHash);
  if (!metadata.has_value()) {
    // No shipped metadata for this hash: keep the filename and empty art
    return;
  }

  // Text fields. A user rename pins the name (nameUserSet), so never override it
  if (!entry.nameUserSet && !metadata->name.empty()) {
    entry.displayName = metadata->name;
  }
  entry.description = metadata->description;
  entry.developer = metadata->developer;
  entry.publisher = metadata->publisher;
  entry.genres = metadata->genre;
  entry.regionIds = metadata->region;
  entry.releaseYear = metadata->releaseYear;
  entry.retroachievementsSetId = metadata->retroAchievementsId;

  // Seed the default art into the media store; the first of each type becomes
  // the selection when the user hasn't already chosen one
  for (const auto &def : metadata->media) {
    if (def.url.empty()) {
      continue;
    }
    MediaAsset asset;
    asset.contentHash = entry.contentHash;
    asset.type = def.type;
    asset.source = MediaSource::RetroAchievements;
    asset.remoteUrl = def.url;
    asset.selected = !m_mediaAssets.selectedFor(entry.contentHash, def.type).has_value();
    m_mediaAssets.add(asset);
  }

  // Project the selected art onto the entry's denormalized columns
  reprojectSelectedMedia(entry);

  m_library.updateEntryMetadata(entry);
}

void MetadataService::reprojectSelectedMedia(library::Entry &entry) {
  // These columns feed the small library-grid surfaces, so use the provider
  // thumbnail when available (the full original can be low-res, especially for
  // icons). The full URL stays in the media store for future high-res uses
  if (const auto icon = m_mediaAssets.selectedFor(entry.contentHash, MediaType::Icon)) {
    entry.icon1x1SourceUrl = icon->displayThumb();
  }
  if (const auto boxFront = m_mediaAssets.selectedFor(entry.contentHash, MediaType::BoxartFront)) {
    entry.boxartFrontSourceUrl = boxFront->displayThumb();
  }
  if (const auto boxBack = m_mediaAssets.selectedFor(entry.contentHash, MediaType::BoxartBack)) {
    entry.boxartBackSourceUrl = boxBack->displayThumb();
  }
}

void MetadataService::applyToEntry(const std::string &contentHash) {
  auto entry = m_library.getEntryWithContentHash(contentHash);
  if (!entry.has_value()) {
    return;
  }
  reprojectSelectedMedia(*entry);
  m_library.updateEntryMetadata(*entry);
}

void MetadataService::selectAsset(const std::string &contentHash, int assetId) {
  if (m_mediaAssets.setSelected(assetId)) {
    applyToEntry(contentHash);
  }
}

void MetadataService::applyCandidate(const std::string &contentHash, const ArtCandidate &candidate) {
  MediaAsset asset;
  asset.contentHash = contentHash;
  asset.type = candidate.type;
  asset.source = MediaSource::SteamGridDb;
  asset.remoteUrl = candidate.url;
  asset.thumbUrl = candidate.thumbUrl;
  asset.width = candidate.width;
  asset.height = candidate.height;
  asset.externalId = candidate.externalId;
  asset.selected = true;
  if (m_mediaAssets.add(asset)) {
    applyToEntry(contentHash);
  }
}

bool MetadataService::importLocalImage(const std::string &contentHash, const MediaType type,
                                       const std::string &sourcePath) {
  std::error_code ec;
  std::filesystem::create_directories(m_mediaDir, ec);
  const auto ext = std::filesystem::path(sourcePath).extension().string();
  // Stable per (game, type) name: re-importing replaces the user's custom art
  const auto dest =
      (std::filesystem::path(m_mediaDir) / (contentHash + "_" + std::to_string(static_cast<int>(type)) + "_user" + ext))
          .string();
  std::filesystem::copy_file(sourcePath, dest, std::filesystem::copy_options::overwrite_existing, ec);
  if (ec) {
    spdlog::error("Failed to import image {}: {}", sourcePath, ec.message());
    return false;
  }

  MediaAsset asset;
  asset.contentHash = contentHash;
  asset.type = type;
  asset.source = MediaSource::User;
  asset.localPath = dest;
  asset.selected = true;
  if (!m_mediaAssets.add(asset)) {
    return false;
  }
  applyToEntry(contentHash);
  return true;
}

void MetadataService::backfillMissing() {
  m_pool.start([this] {
    if (m_shuttingDown) {
      return;
    }
    for (const auto &entry : m_library.getEntries(0, 0)) {
      if (m_shuttingDown) {
        return;
      }
      // Empty icon art is our "hasn't been populated yet" signal.
      if (entry.icon1x1SourceUrl.empty()) {
        populate(entry.id);
      }
    }
  });
}

} // namespace firelight::metadata
