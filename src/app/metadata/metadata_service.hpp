#pragma once

#include <firelight/event_dispatcher.hpp>
#include <firelight/metadata/game_metadata.hpp>

#include <QThreadPool>
#include <atomic>
#include <string>
#include <vector>

namespace firelight::library {
class IUserLibraryRepository;
struct Entry;
} // namespace firelight::library

namespace firelight::metadata {
class IGameMetadataSource;
class IMediaAssetRepository;
class IArtProvider;
struct ArtCandidate;
enum class ArtSearchStatus;

// TODO
// One auto-applied art match paired with the entry that got it, so a low score can be
// looked at next to the name that produced it
struct ArtReviewItem {
  int entryId = -1;
  std::string contentHash;
  std::string displayName;
  std::string matchedName;
  int matchScore = 0;
  std::string thumbUrl;
};

// Auto-populates a library entry's name, filterable metadata, and default art
// when it's first created, from the shipped offline metadata source, and owns
// the user's art choices (select/import/apply). Auto-population runs on a
// dedicated background thread; the user-driven methods are synchronous and safe
// to call from the UI thread (both the media store and the library repository
// are internally synchronized). Every path publishes EntryUpdatedEvent so the
// library view refreshes the affected row in place
class MetadataService {
public:
  // How long to wait after a failed lookup before trying again, and the ceiling that
  // backoff doubles up to. Success resets it, so a healthy provider is never slowed
  static constexpr int ART_BACKOFF_START_MS = 1000;
  static constexpr int ART_BACKOFF_MAX_MS = 5 * 60 * 1000;

  MetadataService(library::IUserLibraryRepository &library, IGameMetadataSource &metadataSource,
                  IMediaAssetRepository &mediaAssets, std::string mediaDir, IArtProvider *artProvider = nullptr);
  ~MetadataService();

  // Looks art up for one entry when it has none, using the best title available.
  // Records the attempt only when the provider actually answered, so a request that
  // failed is asked again later rather than being taken for "no art exists".
  // Runs synchronously on the calling thread
  ArtSearchStatus fetchArt(int entryId);

  // Fills in missing art as fast as the provider allows, backing off when it stops
  // answering, and stops once nothing is left to look up. The attempt marker is
  // persisted, so a restart resumes rather than starting over
  void startArtSweep();

  // TODO
  // Every art match that was made on a title rather than a content hash, worst-scoring
  // first. The assets and the entries are in different databases, so they are paired here
  // rather than in SQL
  [[nodiscard]] std::vector<ArtReviewItem> getArtReviewItems();

  // Resolves and writes metadata for one entry. Public for tests / a future
  // manual "refresh metadata"; runs synchronously on the calling thread
  void populate(int entryId);

  // TODO
  // Queues background re-derivation for every entry. Which file an entry is named from
  // depends on what is catalogued, and that changes between runs
  void refreshAll();

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
  // Works the queue until it is empty or the service is going away
  void runArtSweep();

  // Waits in slices so shutdown does not have to wait out a long backoff
  void backOff(int millis);

  library::IUserLibraryRepository &m_library;
  IGameMetadataSource &m_metadataSource;
  IMediaAssetRepository &m_mediaAssets;
  IArtProvider *m_artProvider;
  std::string m_mediaDir;
  // Its own thread, because the sweep sleeps while backing off and must not hold up
  // metadata population behind it
  QThreadPool m_artPool;
  std::atomic_bool m_artSweepRunning = false;

  QThreadPool m_pool;
  std::atomic_bool m_shuttingDown = false;
  ScopedConnection m_entryCreatedConnection;
};

} // namespace firelight::metadata
