#pragma once

#include <firelight/event_dispatcher.hpp>
#include <firelight/metadata/game_metadata.hpp>

#include <QThreadPool>
#include <atomic>
#include <string>

namespace firelight::library {
class IUserLibraryRepository;
struct Entry;
} // namespace firelight::library

namespace firelight::metadata {
class IGameMetadataSource;
class IMediaAssetRepository;
struct ArtCandidate;

// Auto-populates a library entry's name, filterable metadata, and default art
// when it's first created, from the shipped offline metadata source, and owns
// the user's art choices (select/import/apply). Auto-population runs on a
// dedicated background thread; the user-driven methods are synchronous and safe
// to call from the UI thread (both the media store and the library repository
// are internally synchronized). Every path publishes EntryUpdatedEvent so the
// library view refreshes the affected row in place
class MetadataService {
public:
  MetadataService(library::IUserLibraryRepository &library, IGameMetadataSource &metadataSource,
                  IMediaAssetRepository &mediaAssets, std::string mediaDir);
  ~MetadataService();

  // Resolves and writes metadata for one entry. Public for tests / a future
  // manual "refresh metadata"; runs synchronously on the calling thread
  void populate(int entryId);

  // Queues background population for every visible entry that still looks
  // unpopulated (no icon art), so libraries that predate this feature fill in
  void backfillMissing();

  // --- User art management (backs the "change artwork" picker) --------------
  // Makes an existing stored asset the selection for its (game, type)
  void selectAsset(const std::string &contentHash, int assetId);
  // Adds an online candidate to the store, selected
  void applyCandidate(const std::string &contentHash, const ArtCandidate &candidate);
  // Copies a local image into the managed media dir and selects it
  bool importLocalImage(const std::string &contentHash, MediaType type, const std::string &sourcePath);

private:
  // Writes the currently-selected asset per projected type onto the entry's
  // denormalized art columns (what the existing QML binds)
  void reprojectSelectedMedia(library::Entry &entry);
  // Loads the entry for a content hash, reprojects its art, and persists it
  void applyToEntry(const std::string &contentHash);

  library::IUserLibraryRepository &m_library;
  IGameMetadataSource &m_metadataSource;
  IMediaAssetRepository &m_mediaAssets;
  std::string m_mediaDir;
  QThreadPool m_pool;
  std::atomic_bool m_shuttingDown = false;
  ScopedConnection m_entryCreatedConnection;
};

} // namespace firelight::metadata
