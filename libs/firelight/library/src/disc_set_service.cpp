// TODO: NEEDS REVIEW
#include <firelight/event_dispatcher.hpp>
#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/filename_tags.hpp>
#include <firelight/library/game_identity.hpp>
#include <firelight/library/library_events.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <unordered_set>

namespace firelight::library {

DiscSetService::DiscSetService(IUserLibraryRepository &library, std::string appDataDirectory)
    : m_library(library), m_appDataDirectory(std::move(appDataDirectory)) {}

// TODO
// The set a disc joins, walking the ladder once. A disc already placed stays where it is; a claim
// naming this file binds to it; otherwise the sets carrying the same identity are asked whether
// they have room at this number, and failing that the disc gets a set of its own
std::optional<int> DiscSetService::place(const ContentFile &file) {
  // A cartridge launches through its own file and never joins a set
  if (file.m_type != ContentType::Disc) {
    return std::nullopt;
  }

  // TODO
  // A set is identified by the hash of the disc anchoring it, so content nothing could hash has
  // nothing to be identified by and launches as itself
  if (file.m_contentHash.empty()) {
    return std::nullopt;
  }

  if (const auto placed = m_library.getDiscSetMemberForContentFile(file.m_id)) {
    return placed->m_discSetId;
  }

  const auto path = file.m_inArchive ? file.m_archivePathName : file.m_filePath;

  // A playlist named this file before it was catalogued, so the row is waiting to be filled in
  if (auto claimed = m_library.getPendingDiscSetMember(path)) {
    claimed->m_contentFileId = file.m_id;
    m_library.create(*claimed);

    // TODO
    // A playlist read before any of its discs named a set it could not describe, so the first
    // disc to turn up gives it its platform and title
    if (auto set = m_library.getDiscSet(claimed->m_discSetId); set.has_value() && set->normalizedTitle.empty()) {
      set->platformId = static_cast<unsigned>(file.m_platformId);
      set->title = parseFilenameTags(path).title;
      set->normalizedTitle = file.m_normalizedTitle;
      m_library.updateDiscSet(*set);
    }

    return claimed->m_discSetId;
  }

  auto identity = identityOf(file);

  if (identity.isEmpty()) {
    return std::nullopt;
  }

  // TODO
  // A disc whose name carries no number is the only one anything knows about, so it takes the
  // first place in a set of its own rather than being left out of the model entirely
  if (identity.discNumber == 0) {
    identity.discNumber = 1;
  }

  std::vector<DiscSet> candidates;

  for (const auto &set : m_library.getCandidateDiscSets(identity)) {
    if (hasRoomFor(set, file, identity)) {
      candidates.push_back(set);
    }
  }

  if (!candidates.empty()) {
    // TODO
    // Oldest first, so the same disc lands in the same set however the sets were made. A second
    // candidate means we could not tell them apart, which the row records rather than hides
    const auto setId = candidates.front().id;
    joinSet(file, setId, identity.discNumber, DiscSource::Filename, {}, candidates.size() > 1);
    return setId;
  }

  DiscSet set;
  set.platformId = identity.platformId;
  set.title = parseFilenameTags(path).title;
  set.normalizedTitle = identity.title;

  if (!m_library.createDiscSet(set)) {
    return std::nullopt;
  }

  joinSet(file, set.id, identity.discNumber, DiscSource::Filename, {}, false);
  return set.id;
}

// TODO
// The dump a set is identified by: its lowest-numbered member that has one. A row naming a file
// nobody has yet carries no hash, so it holds a position without being able to anchor anything
std::optional<ContentFile> DiscSetService::anchorOf(const int setId) {
  std::optional<ContentFile> anchor;
  auto anchorNumber = 0;

  for (const auto &member : m_library.getDiscSetMembers(setId)) {
    if (!member.m_contentFileId.has_value()) {
      continue;
    }

    const auto file = m_library.getContentFile(*member.m_contentFileId);

    if (!file.has_value() || file->m_contentHash.empty()) {
      continue;
    }

    if (!anchor.has_value() || member.m_discNumber < anchorNumber) {
      anchor = file;
      anchorNumber = member.m_discNumber;
    }
  }

  return anchor;
}

// TODO
// Keeps the set's entry pointed at the disc the set is identified by. A lower-numbered disc
// arriving moves the identity onto it, which re-keys the entry in place rather than replacing it,
// so its rating, tags, folders and favourite stay where they are
bool DiscSetService::syncSetEntry(const int setId) {
  const auto anchor = anchorOf(setId);

  if (!anchor.has_value()) {
    return false;
  }

  const auto entries = m_library.getEntriesInDiscSet(setId);

  if (entries.empty()) {
    return false;
  }

  auto changed = false;

  for (const auto &entry : entries) {
    if (entry.contentHash == anchor->m_contentHash) {
      continue;
    }

    const auto previous = entry.contentHash;

    if (m_library.setEntryContentHash(entry.id, anchor->m_contentHash, "a lower-numbered disc joined the set")) {
      // TODO
      // The playlist is named after the identity it launches under, so one named after the dump
      // the set has stopped being is a file nothing will open again
      retirePlaylist(previous);
      changed = true;
    }
  }

  return changed;
}

// TODO
// A set launches through one way in, on the disc it is identified by, however many discs it holds.
// The first disc to arrive is what brings the set's entry into being; a lower one arriving later
// moves the identity onto itself rather than standing up a second entry beside the first
bool DiscSetService::syncSetWayIn(const int setId) {
  const auto anchor = anchorOf(setId);
  if (!anchor.has_value()) {
    return false;
  }

  const auto entries = m_library.getEntriesInDiscSet(setId);

  if (entries.empty()) {
    // TODO
    // The anchor's own way in is what brings the set's entry into being, and is replaced by the
    // set's below once it has
    m_library.createRunConfiguration(anchor->m_id, anchor->m_filePath, anchor->m_platformId, anchor->m_contentHash);
  } else {
    // TODO
    // One game is one row, so the set keeps the entry already standing for its anchor and takes
    // over any other. Taking over before re-keying, or two of them end up on one dump
    const auto survivor =
        std::ranges::find_if(entries, [&](const Entry &entry) { return entry.contentHash == anchor->m_contentHash; });
    const auto &kept = survivor != entries.end() ? *survivor : entries.front();

    for (const auto &entry : entries) {
      if (entry.id == kept.id) {
        continue;
      }

      EventDispatcher::instance().publish(EntryAbsorbedEvent{.survivingEntryId = kept.id,
                                                             .absorbedContentHash = entry.contentHash,
                                                             .survivingContentHash = anchor->m_contentHash});
      m_library.deleteEntry(entry.id);
    }

    syncSetEntry(setId);
  }

  // Every disc is reached through the set, so none keeps a way in of its own. Cleared before the
  // set's own is written, or that would be the one removed
  for (const auto &disc : m_library.getDiscsInSet(setId)) {
    m_library.deleteRunConfigurationsForContentFile(disc.m_id);
  }

  m_library.createRunConfigurationForSet(setId, anchor->m_id, anchor->m_contentHash);
  materializePlaylist(setId, anchor->m_contentHash);
  return true;
}

// TODO
// A playlist naming its discs in order. Every line becomes a membership row, whether or not the
// file behind it has been catalogued; a line naming nothing yet keeps the position that disc will
// take when it turns up
std::optional<int> DiscSetService::claimPlaylist(const std::string &playlistPath,
                                                 const std::vector<std::string> &discPaths) {
  if (discPaths.empty()) {
    return std::nullopt;
  }

  std::vector<std::optional<ContentFile>> named;
  named.reserve(discPaths.size());

  for (const auto &path : discPaths) {
    named.push_back(m_library.getContentFileWithPath(path));
  }

  // The set a named disc already sits in, so a playlist read after a scan corrects that grouping
  // rather than building a second one beside it
  std::optional<int> setId;

  for (const auto &file : named) {
    if (!file.has_value()) {
      continue;
    }

    if (const auto placed = m_library.getDiscSetMemberForContentFile(file->m_id)) {
      setId = placed->m_discSetId;
      break;
    }
  }

  if (!setId.has_value()) {
    DiscSet set;

    for (const auto &file : named) {
      if (file.has_value()) {
        set.platformId = static_cast<unsigned>(file->m_platformId);
        set.title = parseFilenameTags(file->m_inArchive ? file->m_archivePathName : file->m_filePath).title;
        set.normalizedTitle = file->m_normalizedTitle;
        break;
      }
    }

    if (!m_library.createDiscSet(set)) {
      return std::nullopt;
    }

    setId = set.id;
  }

  for (size_t index = 0; index < discPaths.size(); ++index) {
    const auto discNumber = static_cast<int>(index) + 1;

    // A disc the playlist claims may already be in another set on a weaker source's word
    if (named[index].has_value()) {
      const auto placed = m_library.getDiscSetMemberForContentFile(named[index]->m_id);

      // TODO
      // A person's own choice outranks a playlist, so a disc somebody took out of a set stays out
      // of it and the line naming it holds a place instead
      if (placed.has_value() && placed->m_source == DiscSource::User && placed->m_discSetId != *setId) {
        continue;
      }

      if (placed.has_value() && placed->m_discSetId != *setId) {
        m_library.deleteDiscSetMember(placed->m_id);
      }
    }

    DiscSetMember member{.m_discSetId = *setId,
                         .m_discNumber = discNumber,
                         .m_contentFileId = named[index].has_value() ? std::optional(named[index]->m_id) : std::nullopt,
                         .m_memberPath = discPaths[index],
                         .m_source = DiscSource::PlaylistFile,
                         .m_sourcePath = playlistPath};

    m_library.create(member);
  }

  // TODO
  // The line count is the one authoritative statement about how many discs the game came on that
  // does not need a metadata database
  if (auto set = m_library.getDiscSet(*setId)) {
    const auto claimed = static_cast<int>(discPaths.size());

    if (set->discCount != claimed) {
      set->discCount = claimed;
      m_library.updateDiscSet(*set);
    }
  }

  return setId;
}

// TODO
// Whether a set could take this disc: the regions its members agree on must be compatible, and
// nothing may already sit at the number
bool DiscSetService::hasRoomFor(const DiscSet &set, const ContentFile &file, const GameIdentity &identity) {
  for (const auto &member : m_library.getDiscSetMembers(set.id)) {
    const auto held = member.m_contentFileId.has_value() ? m_library.getContentFile(*member.m_contentFileId)
                                                         : std::optional<ContentFile>{};

    // TODO
    // The same dump in another container is another copy of a disc the set already holds, not a
    // second disc competing for its place
    const auto isAnotherCopy = held.has_value() && held->m_contentHash == file.m_contentHash;

    if (member.m_discNumber == identity.discNumber && !isAnotherCopy) {
      return false;
    }

    if (held.has_value() && !areDiscsOfOneRelease(identity, identityOf(*held))) {
      return false;
    }
  }

  return true;
}

// TODO
// Writes the membership row putting a disc in a set
void DiscSetService::joinSet(const ContentFile &file, const int setId, const int discNumber, const DiscSource source,
                             const std::string &sourcePath, const bool isUncertain) {
  DiscSetMember member{.m_discSetId = setId,
                       .m_discNumber = discNumber,
                       .m_contentFileId = file.m_id,
                       .m_memberPath = file.m_inArchive ? file.m_archivePathName : file.m_filePath,
                       .m_source = source,
                       .m_sourcePath = sourcePath,
                       .m_isUncertain = isUncertain};

  m_library.create(member);
}

// TODO
// Gives an entry the game's name rather than its filename, taken from whichever of the entries
// standing beside it has been through metadata
void DiscSetService::nameAfterPeers(const std::string &contentHash, const std::vector<Entry> &peers) {
  auto entry = m_library.getEntryWithContentHash(contentHash);

  if (!entry.has_value() || !entry->normalizedTitle.empty()) {
    return;
  }

  const auto named = std::ranges::find_if(peers, [](const Entry &peer) { return !peer.normalizedTitle.empty(); });

  if (named == peers.end()) {
    return;
  }

  entry->displayName = named->displayName;
  entry->normalizedTitle = named->normalizedTitle;
  m_library.updateEntryMetadata(*entry);
}

// TODO
// The same for whichever entry a set is standing under now, which a detach can have moved
void DiscSetService::nameAfterPeers(const int setId, const std::vector<Entry> &peers) {
  for (const auto &entry : m_library.getEntriesInDiscSet(setId)) {
    nameAfterPeers(entry.contentHash, peers);
  }
}

// TODO
// Takes a disc out of the set holding it and gives it one of its own, recorded as the person's
// decision so placement leaves it where they put it
bool DiscSetService::detachDisc(const int contentFileId) {
  const auto file = m_library.getContentFile(contentFileId);

  if (!file.has_value()) {
    return false;
  }

  const auto member = m_library.getDiscSetMemberForContentFile(contentFileId);

  if (!member.has_value()) {
    return false;
  }

  const auto previousSetId = member->m_discSetId;

  // TODO
  // Read while the set it is leaving still has a way in, which is what reaches its entries
  const auto peers = m_library.getEntriesInDiscSet(previousSetId);

  if (!m_library.deleteDiscSetMember(member->m_id)) {
    return false;
  }

  // TODO
  // A set's own way in is spared when a disc's configurations are cleared, so the one anchored on
  // the disc that just left has to go by hand. Each set writes its own again below
  m_library.deleteRunConfigurationsForDiscSet(previousSetId);

  DiscSet set;
  set.platformId = static_cast<unsigned>(file->m_platformId);
  set.title = parseFilenameTags(file->m_inArchive ? file->m_archivePathName : file->m_filePath).title;
  set.normalizedTitle = file->m_normalizedTitle;

  if (!m_library.createDiscSet(set)) {
    return false;
  }

  // TODO
  // Marked uncertain because taking a disc out of a set says where it does not belong, not that
  // it is a game on its own
  joinSet(*file, set.id, std::max(file->m_discNumber, 1), DiscSource::User, {}, true);
  syncSetWayIn(set.id);

  // TODO
  // The entry it just got is named after its file, so it keeps the game's name rather than
  // reading as something else in the library
  nameAfterPeers(file->m_contentHash, peers);

  // A set holding no discs is nothing anybody can act on
  if (m_library.getDiscSetMembers(previousSetId).empty()) {
    m_library.deleteDiscSet(previousSetId);
    return true;
  }

  // The set it left may have been identified by the disc that just went
  syncSetWayIn(previousSetId);

  // TODO
  // Taking the disc a set was identified by out of it stands the rest up under a new hash, and
  // that entry is named after its file for the same reason the detached one was
  nameAfterPeers(previousSetId, peers);
  return true;
}

// TODO
// Hands a disc back to placement by forgetting the row a person put there, undoing an earlier
// detach
bool DiscSetService::clearUserChoice(const int entryId) {
  const auto entry = m_library.getEntry(entryId);

  if (!entry.has_value()) {
    return false;
  }

  auto changed = false;

  for (const auto &file : m_library.getContentFilesWithContentHash(entry->contentHash)) {
    const auto member = m_library.getDiscSetMemberForContentFile(file.m_id);

    if (!member.has_value() || member->m_source != DiscSource::User) {
      continue;
    }

    const auto previousSetId = member->m_discSetId;
    m_library.deleteDiscSetMember(member->m_id);

    if (const auto setId = place(file); setId.has_value() && *setId != previousSetId) {
      syncSetWayIn(*setId);

      // The set it left holds nothing now
      if (m_library.getDiscSetMembers(previousSetId).empty()) {
        m_library.deleteDiscSet(previousSetId);
      }

      changed = true;
    }
  }

  return changed;
}

namespace {
// TODO
// A playlist's bytes, empty when there is nothing there to read
std::string readPlaylist(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}
} // namespace

void DiscSetService::retirePlaylist(const std::string &contentHash) {
  std::lock_guard lock(m_playlistMutex);
  const auto path = playlistPathFor(contentHash, m_appDataDirectory);
  std::error_code ec;

  if (!std::filesystem::exists(path, ec)) {
    return;
  }

  // TODO
  // Only a file that says it is ours, so a playlist somebody wrote at this path is left alone
  // rather than deleted out from under them
  if (!isGeneratedPlaylist(readPlaylist(path))) {
    spdlog::warn("Leaving the playlist at {} alone: it is not one of ours", path);
    return;
  }

  std::filesystem::remove(path, ec);
}

bool DiscSetService::materializePlaylist(const int setId, const std::string &contentHash) {
  const auto plan = planPlaylist(m_library.getPresentDiscsInSet(setId), contentHash, m_appDataDirectory);

  if (!plan.has_value()) {
    return false;
  }

  std::lock_guard lock(m_playlistMutex);
  std::error_code ec;

  if (std::filesystem::exists(plan->path, ec)) {
    const auto existing = readPlaylist(plan->path);

    if (existing == plan->contents) {
      return false;
    }

    // TODO
    // Only a file that says it is ours, so a playlist somebody wrote at this path is left alone
    // rather than overwritten
    if (!isGeneratedPlaylist(existing)) {
      spdlog::warn("Leaving the playlist at {} alone: it is not one of ours", plan->path);
      return false;
    }
  }

  std::filesystem::create_directories(std::filesystem::path(plan->path).parent_path(), ec);

  // TODO
  // Written alongside and moved into place, so a launch reading this file never catches it
  // half-written
  const auto stagedPath = plan->path + ".tmp";

  {
    std::ofstream out(stagedPath, std::ios::binary | std::ios::trunc);

    if (!out) {
      spdlog::warn("Could not write the playlist for set {} at {}", setId, stagedPath);
      return false;
    }

    out << plan->contents;
  }

  std::filesystem::rename(stagedPath, plan->path, ec);

  if (ec) {
    spdlog::warn("Could not put the playlist for set {} in place: {}", setId, ec.message());
    std::filesystem::remove(stagedPath, ec);
    return false;
  }

  spdlog::info("Set {} launches through {}", setId, plan->path);
  return true;
}

} // namespace firelight::library
