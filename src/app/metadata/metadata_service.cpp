#include "metadata/metadata_service.hpp"

#include <firelight/library/entry.hpp>
#include <firelight/library/filename_tags.hpp>
#include <firelight/library/library_events.hpp>
#include <firelight/library/user_library_repository.hpp>
#include <firelight/metadata/art_provider.hpp>
#include <firelight/metadata/game_metadata_source.hpp>
#include <firelight/metadata/media_asset_repository.hpp>
#include <firelight/metadata/media_slot.hpp>

#include <QThread>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <set>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <utility>

namespace firelight::metadata {

namespace {
uint64_t nowMillis() {
  using namespace std::chrono;
  return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}
} // namespace

MetadataService::MetadataService(library::IUserLibraryRepository &library, IGameMetadataSource &metadataSource,
                                 IMediaAssetRepository &mediaAssets, std::string mediaDir, IArtProvider *artProvider)
    : m_library(library), m_metadataSource(metadataSource), m_mediaAssets(mediaAssets), m_artProvider(artProvider),
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

        // A scan adding games gives the sweep something to do again
        startArtSweep();
      });

  m_artPool.setMaxThreadCount(1);
}

MetadataService::~MetadataService() {
  m_shuttingDown = true;
  m_artPool.waitForDone();
  m_pool.waitForDone();
}

void MetadataService::populate(int entryId) {
  const auto entryOpt = m_library.getEntry(entryId);
  if (!entryOpt.has_value()) {
    return;
  }
  auto entry = *entryOpt;

  // TODO
  // The filename is the base layer and runs whether or not the database knows this
  // hash, so a library of unmatched content still gets regions and languages
  GameMetadata incoming;
  std::set<std::string> changed;

  // TODO
  // A field only counts as answered when something is actually there, so a later layer
  // fills a gap rather than blanking what an earlier one found
  const auto overlay = [&](const char *field, auto &destination, const auto &value) {
    if (!value.empty()) {
      destination = value;
      changed.insert(field);
    }
  };

  // TODO
  // What the game is called once the tags are off, kept apart from the display name so a
  // rename does not change which releases this one counts as a version of
  std::string derivedTitle;

  if (!entry.contentPaths.empty()) {
    const auto tags = library::parseFilenameTags(entry.contentPaths.front());
    derivedTitle = tags.title;

    // TODO
    // The basename the entry was created with still carries its set index and every
    // tag group. The parsed title is what the game is actually called, so it becomes
    // the name unless the user picked one
    if (!entry.nameUserSet && !tags.title.empty() && tags.title != entry.displayName) {
      entry.displayName = tags.title;
      m_library.updateEntryMetadata(entry);
    }

    overlay(metadata_fields::REGIONS, incoming.regions, tags.regions);
    overlay(metadata_fields::LANGUAGES, incoming.languages, tags.languages);
    overlay(metadata_fields::REVISION, incoming.revision, tags.revision);
    overlay(metadata_fields::FLAGS, incoming.flags, tags.flags);
  }

  const auto metadata = m_metadataSource.lookup(entry.contentHash);

  if (metadata.has_value()) {
    // The database is authoritative, so anything it knows replaces what the filename
    // guessed at
    if (!metadata->name.empty()) {
      derivedTitle = metadata->name;
    }

    if (!entry.nameUserSet && !metadata->name.empty()) {
      entry.displayName = metadata->name;
      m_library.updateEntryMetadata(entry);
    }

    // TODO
    // Both sides hold the same shape, so what the source knows is laid over what the filename
    // guessed field by field rather than copied across a translation
    overlay(metadata_fields::DESCRIPTION, incoming.description, metadata->metadata.description);
    overlay(metadata_fields::DEVELOPER, incoming.developer, metadata->metadata.developer);
    overlay(metadata_fields::PUBLISHER, incoming.publisher, metadata->metadata.publisher);
    overlay(metadata_fields::RELEASE_DATE, incoming.releaseDate, metadata->metadata.releaseDate);
    overlay(metadata_fields::PLAYERS, incoming.players, metadata->metadata.players);
    overlay(metadata_fields::REVISION, incoming.revision, metadata->metadata.revision);
    overlay(metadata_fields::GENRES, incoming.genres, metadata->metadata.genres);
    overlay(metadata_fields::REGIONS, incoming.regions, metadata->metadata.regions);
    overlay(metadata_fields::LANGUAGES, incoming.languages, metadata->metadata.languages);
    overlay(metadata_fields::FLAGS, incoming.flags, metadata->metadata.flags);

    if (metadata->metadata.releaseYear != 0) {
      incoming.releaseYear = metadata->metadata.releaseYear;
      changed.insert(metadata_fields::RELEASE_YEAR);
    }

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
  }

  if (!changed.empty()) {
    m_library.applyEntryMetadata(entryId, incoming, changed, false);
  }

  entry.normalizedTitle = library::normalizeTitle(derivedTitle.empty() ? entry.displayName : derivedTitle);

  // Project the selected art onto the entry's denormalized columns
  reprojectSelectedMedia(entry);

  m_library.updateEntryMetadata(entry);
}

void MetadataService::reprojectSelectedMedia(library::Entry &entry) {
  // These columns feed the small library-grid surfaces, so use the provider
  // thumbnail when available (the full original can be low-res, especially for
  // icons). The full URL stays in the media store for future high-res uses
  //
  // TODO
  // Resolved by slot rather than by exact type, so a game with a title screen and
  // no icon shows the title screen instead of an empty tile
  const auto assets = m_mediaAssets.listForContent(entry.contentHash);

  if (const auto tile = resolveSlot(assets, MediaSlot::TileSquare)) {
    entry.icon1x1SourceUrl = tile->displayThumb();
  }
  if (const auto boxFront = resolveSlot(assets, MediaSlot::BoxartFront)) {
    entry.boxartFrontSourceUrl = boxFront->displayThumb();
  }
  if (const auto boxBack = resolveSlot(assets, MediaSlot::BoxartBack)) {
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

ArtSearchStatus MetadataService::fetchArt(const int entryId) {
  if (m_artProvider == nullptr || !m_artProvider->isConfigured()) {
    // TODO
    // Deliberately not marked as tried. No key means not yet attempted, so adding
    // one later must not find the whole library already accounted for
    return ArtSearchStatus::Failed;
  }

  const auto entryOpt = m_library.getEntry(entryId);

  if (!entryOpt.has_value()) {
    return ArtSearchStatus::Ok;
  }

  const auto &entry = *entryOpt;

  if (entry.artFetchedAt.has_value()) {
    return ArtSearchStatus::Ok;
  }

  if (!m_mediaAssets.listForContent(entry.contentHash).empty()) {
    m_library.markArtFetched(entryId, nowMillis());
    return ArtSearchStatus::Ok;
  }

  // TODO
  // The display name already holds the database's title when it resolved one, and
  // the file's basename otherwise. Running the tag parser over it strips (USA) and
  // (Rev 1) from the second case without spoiling the first
  auto title = library::parseFilenameTags(entry.displayName).title;

  if (title.empty()) {
    title = entry.displayName;
  }

  const auto result = m_artProvider->search(title, static_cast<int>(entry.platformId), MediaType::GridSquare);

  if (!result.ok()) {
    // TODO
    // Left unmarked on purpose: the provider never answered, so "no art" is not a
    // thing that has been established about this game
    return result.status;
  }

  if (!result.candidates.empty()) {
    // TODO
    // Only the winning game's art. The search ranks games but still returns every one
    // it looked at, and filing another game's pictures under this hash would be wrong
    const auto &winner = result.candidates.front().gameName;
    auto firstOfEachType = std::set<MediaType>{};
    auto storedFirst = false;

    for (const auto &candidate : result.candidates) {
      if (candidate.gameName != winner) {
        continue;
      }

      MediaAsset asset;
      asset.contentHash = entry.contentHash;
      // TODO
      // The candidate's own shape, not the one that was asked for: a game with no art
      // of the wanted shape still answers, with something differently cropped
      asset.type = candidate.type;
      asset.source = MediaSource::SteamGridDb;
      asset.remoteUrl = candidate.url;
      asset.thumbUrl = candidate.thumbUrl;
      asset.width = candidate.width;
      asset.height = candidate.height;
      asset.externalId = candidate.externalId;
      // Matched on a title rather than the content hash, so it is a guess until
      // somebody says otherwise
      asset.unconfirmed = true;
      asset.matchedName = candidate.gameName;
      asset.matchScore = candidate.matchScore;

      if (!m_mediaAssets.add(asset)) {
        continue;
      }

      // The best of each shape becomes that shape's selection; the rest are stored so
      // the picker has something to show without going back to the network
      if (firstOfEachType.insert(candidate.type).second &&
          !m_mediaAssets.selectedFor(entry.contentHash, candidate.type).has_value()) {
        m_mediaAssets.setSelected(asset.id);
        storedFirst = true;
      }
    }

    if (storedFirst) {
      applyToEntry(entry.contentHash);
    }
  }

  // TODO
  // Stamped even when the search found nothing, or a game the provider does not
  // have is looked up again on every startup forever
  m_library.markArtFetched(entryId, nowMillis());
  return ArtSearchStatus::Ok;
}

void MetadataService::startArtSweep() {
  if (m_artProvider == nullptr || !m_artProvider->isConfigured() || m_shuttingDown) {
    return;
  }

  // One sweep at a time; a second request while one is working is already covered
  // because the running loop re-queries for what is left
  auto expected = false;

  if (!m_artSweepRunning.compare_exchange_strong(expected, true)) {
    return;
  }

  m_artPool.start([this] {
    runArtSweep();
    m_artSweepRunning = false;
  });
}

void MetadataService::backOff(const int millis) {
  constexpr int SLICE_MS = 100;

  for (auto waited = 0; waited < millis && !m_shuttingDown; waited += SLICE_MS) {
    QThread::msleep(SLICE_MS);
  }
}

void MetadataService::runArtSweep() {
  auto backoffMs = ART_BACKOFF_START_MS;

  while (!m_shuttingDown) {
    const auto pending = m_library.getEntryIdsMissingArt(1);

    if (pending.empty()) {
      return;
    }

    switch (fetchArt(pending.front())) {
    case ArtSearchStatus::Ok:
      // Straight on to the next one: the provider is answering, so the only thing
      // pacing this is how fast it does
      backoffMs = ART_BACKOFF_START_MS;
      break;
    case ArtSearchStatus::RateLimited:
    case ArtSearchStatus::Failed:
      spdlog::debug("Art lookup failed; waiting {}ms before the next", backoffMs);
      backOff(backoffMs);
      backoffMs = std::min(backoffMs * 2, ART_BACKOFF_MAX_MS);
      break;
    }
  }
}

std::vector<ArtReviewItem> MetadataService::getArtReviewItems() {
  const auto assets = m_mediaAssets.listUnconfirmedSelections();
  if (assets.empty()) {
    return {};
  }

  std::unordered_map<std::string, const library::Entry *> entriesByHash;
  const auto entries = m_library.getEntries(0, 0);
  for (const auto &entry : entries) {
    entriesByHash.emplace(entry.contentHash, &entry);
  }

  std::vector<ArtReviewItem> items;
  items.reserve(assets.size());
  for (const auto &asset : assets) {
    const auto found = entriesByHash.find(asset.contentHash);
    if (found == entriesByHash.end()) {
      continue;
    }

    items.push_back({.entryId = found->second->id,
                     .contentHash = asset.contentHash,
                     .displayName = found->second->displayName,
                     .matchedName = asset.matchedName,
                     .matchScore = asset.matchScore,
                     .thumbUrl = asset.displayThumb()});
  }
  return items;
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
      // TODO
      // An empty document is the "hasn't been populated yet" signal. Missing art is
      // not: metadata now comes from the filename too, so an entry can be fully
      // populated and still have no picture
      if (entry.metadata.isEmpty()) {
        populate(entry.id);
      }
    }
  });
}

} // namespace firelight::metadata
