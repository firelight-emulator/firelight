#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/filename_tags.hpp>
#include <firelight/library/library_events.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <unordered_set>
#include <utility>

namespace firelight::library {

namespace {
// Holds the re-entrancy flag for the length of a write, restoring whatever it was so a nested
// call does not clear it on the way out
class ApplyGuard {
public:
  explicit ApplyGuard(std::atomic_bool &flag) : m_flag(flag), m_previous(flag.exchange(true)) {}

  ~ApplyGuard() { m_flag = m_previous; }

private:
  std::atomic_bool &m_flag;
  bool m_previous;
};
} // namespace

DiscSetService::DiscSetService(IUserLibraryRepository &library, std::string appDataDirectory)
    : m_library(library), m_appDataDirectory(std::move(appDataDirectory)) {
  // The title a set is matched on is written by metadata population, so a decision made when
  // the content file arrived would have nothing to match on yet
  m_entryUpdatedConnection =
      EventDispatcher::instance().subscribe<EntryUpdatedEvent>([this](const EntryUpdatedEvent &event) {
        if (m_applying) {
          return;
        }

        autoGroupDiscs(event.entryId);
      });

  // A set's only way in is its playlist, so losing that file hides the game. This is what
  // notices, since the scan removes the catalogued playlist before the entry is hidden
  m_runConfigurationDeletedConnection =
      EventDispatcher::instance().subscribe<RunConfigurationDeletedEvent>([this](const RunConfigurationDeletedEvent &) {
        if (m_applying) {
          return;
        }

        reconcilePlaylists();
      });
}

void DiscSetService::reconcilePlaylists() {
  const ApplyGuard guard(m_applying);

  for (const auto &set : m_library.getDiscSets()) {
    if (m_library.getDiscsInSet(set.id).size() < 2) {
      continue;
    }

    if (const auto survivor = survivorOf(set.id)) {
      writePlaylist(set.id, *survivor);
    }
  }
}

bool DiscSetService::autoGroupDiscs(const int entryId) {
  const ApplyGuard guard(m_applying);

  const auto entry = m_library.getEntry(entryId);

  if (!entry.has_value() || entry->discNumber == 0 || entry->normalizedTitle.empty() || entry->discSetUserSet) {
    return false;
  }

  std::vector<Entry> members{*entry};

  for (const auto peerId : m_library.getEntryIdsWithNormalizedTitle(entry->platformId, entry->normalizedTitle)) {
    if (peerId == entryId) {
      continue;
    }

    const auto peer = m_library.getEntry(peerId);

    // A disc somebody took out by hand is left where they put it, and a game that is not a
    // disc of anything has nothing to join
    if (!peer.has_value() || peer->discSetUserSet || peer->discNumber == 0) {
      continue;
    }

    // A USA release and a JP one fold to the same title, and their discs are not each
    // other's. Region is the only thing on hand that tells the two runs apart
    if (peer->metadata.regions != entry->metadata.regions) {
      continue;
    }

    members.push_back(*peer);
  }

  if (members.size() < 2 || !areDiscNumbersUnambiguous(members)) {
    return false;
  }

  std::ranges::sort(members, [](const Entry &left, const Entry &right) {
    return std::tie(left.discNumber, left.id) < std::tie(right.discNumber, right.id);
  });

  const auto &survivor = members.front();
  const auto setId = resolveSet(members, survivor);

  if (!setId.has_value()) {
    return false;
  }

  auto changed = false;

  for (const auto &member : members) {
    if (member.id == survivor.id) {
      continue;
    }

    changed = absorb(member, survivor, *setId) || changed;
  }

  for (const auto &file : m_library.getContentFilesWithContentHash(survivor.contentHash)) {
    // The set's own playlist carries the same hash as its first disc. It addresses the discs
    // rather than being one, so it must not join them
    if (file.m_discNumber > 0) {
      m_library.setContentFileDiscSet(file.m_id, *setId);
    }
  }

  if (survivor.discSetId != setId) {
    m_library.setEntryDiscSet(survivor.id, *setId, false);
    changed = true;
  }

  if (writePlaylist(*setId, survivor)) {
    changed = true;
  }

  return changed;
}

void DiscSetService::setPlaylistLocation(const PlaylistLocation location) { m_playlistLocation = location; }

bool DiscSetService::detachDisc(const int contentFileId) {
  const auto file = m_library.getContentFile(contentFileId);

  if (!file.has_value() || !file->m_discSetId.has_value()) {
    return false;
  }

  const auto setId = *file->m_discSetId;
  const auto set = m_library.getDiscSet(setId);
  const ApplyGuard guard(m_applying);

  restoreOwnEntry(*file, set.has_value() ? set->title : "", true);

  if (!dissolveIfUndersized(setId)) {
    // The playlist still names the disc that just left
    if (const auto survivor = survivorOf(setId)) {
      writePlaylist(setId, *survivor);
    }
  }

  return true;
}

bool DiscSetService::clearUserChoice(const int entryId) {
  const auto entry = m_library.getEntry(entryId);

  if (!entry.has_value() || !entry->discSetUserSet) {
    return false;
  }

  if (!m_library.setEntryDiscSet(entryId, entry->discSetId, false)) {
    return false;
  }

  autoGroupDiscs(entryId);
  return true;
}

bool DiscSetService::areDiscNumbersUnambiguous(const std::vector<Entry> &members) {
  std::unordered_set<int> discNumbers;
  std::unordered_set<std::string> memberHashes;
  std::unordered_set<int> setIds;

  for (const auto &member : members) {
    memberHashes.insert(member.contentHash);

    if (member.discSetId.has_value()) {
      setIds.insert(*member.discSetId);
    }

    if (!discNumbers.insert(member.discNumber).second) {
      spdlog::info("Not forming a disc set: disc {} is here more than once", member.discNumber);
      return false;
    }
  }

  // The discs already folded into a set have no entry left to be found by title, so
  // without this the same disc number walks back in and whichever release was scanned
  // first decides the answer
  for (const auto setId : setIds) {
    for (const auto &disc : m_library.getDiscsInSet(setId)) {
      if (memberHashes.count(disc.m_contentHash) > 0) {
        continue;
      }

      if (!discNumbers.insert(disc.m_discNumber).second) {
        spdlog::info("Not forming a disc set: disc {} is already in set {}", disc.m_discNumber, setId);
        return false;
      }
    }
  }

  return true;
}

std::optional<int> DiscSetService::resolveSet(const std::vector<Entry> &members, const Entry &survivor) {
  if (survivor.discSetId.has_value()) {
    return survivor.discSetId;
  }

  // Keeping the set a late-arriving disc 1 walks into means its title and its playlist do not
  // start over just because the entry standing for it moved
  for (const auto &member : members) {
    if (member.discSetId.has_value()) {
      return member.discSetId;
    }
  }

  DiscSet set;
  set.title = survivor.displayName;

  if (!m_library.createDiscSet(set)) {
    return std::nullopt;
  }

  return set.id;
}

bool DiscSetService::absorb(const Entry &absorbed, const Entry &survivor, const int setId) {
  if (!m_library.deleteEntry(absorbed.id)) {
    return false;
  }

  for (const auto &file : m_library.getContentFilesWithContentHash(absorbed.contentHash)) {
    if (file.m_discNumber == 0) {
      continue;
    }

    m_library.setContentFileDiscSet(file.m_id, setId);

    // The set launches through the entry that survived, so a second way in would be one more
    // thing for the resolver to pick between
    m_library.deleteRunConfigurationsForContentFile(file.m_id);
  }

  spdlog::info("Disc {} folded into entry {} as part of set {}", absorbed.discNumber, survivor.id, setId);

  EventDispatcher::instance().publish(EntryAbsorbedEvent{.survivingEntryId = survivor.id,
                                                         .absorbedContentHash = absorbed.contentHash,
                                                         .survivingContentHash = survivor.contentHash});
  return true;
}

bool DiscSetService::restoreOwnEntry(const ContentFile &file, const std::string &title, const bool isUserChoice) {
  m_library.setContentFileDiscSet(file.m_id, std::nullopt);

  const auto configurations = m_library.getRunConfigurations(file.m_contentHash);
  const auto isLaunchable = std::ranges::any_of(
      configurations, [&](const RunConfiguration &configuration) { return configuration.contentFileId == file.m_id; });

  // A disc that was absorbed lost its way in along with its entry, and creating one is what
  // gives it an entry back
  if (!isLaunchable) {
    m_library.createRunConfiguration(file.m_id, file.m_filePath, file.m_platformId, file.m_contentHash);
  }

  auto entry = m_library.getEntryWithContentHash(file.m_contentHash);

  if (!entry.has_value()) {
    return false;
  }

  // A recreated entry is named after its file and has no title to match on, so without this
  // it reads as a different game from the one it just came out of
  if (entry->normalizedTitle.empty() && !title.empty()) {
    entry->displayName = title;
    // Folded the same way metadata population folds it, or this entry's title matches nothing
    // and the disc can never rejoin the set it came out of
    entry->normalizedTitle = normalizeTitle(title);
    m_library.updateEntryMetadata(*entry);
  }

  return m_library.setEntryDiscSet(entry->id, std::nullopt, isUserChoice);
}

bool DiscSetService::dissolveIfUndersized(const int setId) {
  const auto discs = m_library.getDiscsInSet(setId);

  if (discs.size() > 1) {
    return false;
  }

  const auto set = m_library.getDiscSet(setId);

  // Before the playlist's way in goes, so the entry is never left with none and hidden
  for (const auto &disc : discs) {
    restoreOwnEntry(disc, set.has_value() ? set->title : "", false);
  }

  if (set.has_value()) {
    retirePlaylist(*set);
  }

  return m_library.deleteDiscSet(setId);
}

void DiscSetService::retirePlaylist(const DiscSet &set) {
  if (set.playlistPath.empty()) {
    return;
  }

  std::error_code ec;
  const auto size = std::filesystem::file_size(set.playlistPath, ec);

  if (!ec) {
    if (const auto file = m_library.getContentFileWithPathAndSize(set.playlistPath, size, false)) {
      m_library.deleteContentFile(file->m_id);
    }
  }

  // Only the one we wrote. Removing a file somebody made themselves is not ours to do
  if (set.playlistOwned) {
    std::filesystem::remove(set.playlistPath, ec);
  }
}

std::optional<Entry> DiscSetService::survivorOf(const int setId) {
  for (const auto &disc : m_library.getDiscsInSet(setId)) {
    if (const auto entry = m_library.getEntryWithContentHash(disc.m_contentHash);
        entry.has_value() && entry->discSetId == setId) {
      return entry;
    }
  }

  return std::nullopt;
}

bool DiscSetService::writePlaylist(const int setId, const Entry &survivor) {
  auto set = m_library.getDiscSet(setId);

  if (!set.has_value()) {
    return false;
  }

  auto plan = planPlaylist(m_library.getDiscsInSet(setId), setId, set->title, m_playlistLocation, m_appDataDirectory);

  if (!plan.has_value()) {
    return false;
  }

  // Two releases of one game land on the same title in the same folder, and adopting the other
  // one's playlist would launch its discs under this set's identity
  if (const auto claimed = m_library.getContentFileWithPath(plan->path);
      claimed.has_value() && claimed->m_contentHash != survivor.contentHash && set->playlistPath != plan->path) {
    plan = planPlaylist(m_library.getDiscsInSet(setId), setId, set->title + " (" + std::to_string(setId) + ")",
                        m_playlistLocation, m_appDataDirectory);

    if (!plan.has_value()) {
      return false;
    }
  }

  std::error_code ec;
  const auto alreadyThere = std::filesystem::exists(plan->path, ec);

  // Somebody's own playlist is the one they take to other frontends, so it is adopted whole
  // rather than rewritten to say the same thing in our words. Read from the file rather than
  // from the set, so a playlist of ours still reads as ours after the library was thrown away
  auto isOurs = !alreadyThere;
  auto isUpToDate = false;

  if (alreadyThere) {
    std::ifstream existingPlaylist(plan->path, std::ios::binary);
    const std::string existing((std::istreambuf_iterator<char>(existingPlaylist)), std::istreambuf_iterator<char>());
    isOurs = isGeneratedPlaylist(existing);
    isUpToDate = isOurs && existing == plan->contents;
  }

  if (isOurs && !isUpToDate) {
    std::filesystem::create_directories(std::filesystem::path(plan->path).parent_path(), ec);
    std::ofstream out(plan->path, std::ios::binary | std::ios::trunc);

    if (!out) {
      spdlog::warn("Could not write the playlist for set {} at {}", setId, plan->path);
      return false;
    }

    out << plan->contents;
    out.close();

    if (!out) {
      spdlog::warn("Could not finish writing the playlist for set {}", setId);
      return false;
    }
  }

  const auto size = std::filesystem::file_size(plan->path, ec);

  if (ec) {
    return false;
  }

  // The set moved to a different playlist, so the one it used to launch through is not its
  // way in any more
  if (!set->playlistPath.empty() && set->playlistPath != plan->path) {
    retirePlaylist(*set);
  }

  const auto existing = m_library.getContentFileWithPath(plan->path);
  auto contentFileId = existing.has_value() ? existing->m_id : -1;
  auto isAlreadyRegistered = false;

  if (existing.has_value()) {
    // A rewrite changes the size, and a late disc 1 changes whose identity it carries. Keeping
    // the row means the way in survives with it
    isAlreadyRegistered = existing->m_contentHash == survivor.contentHash;
    m_library.setContentFileIdentity(existing->m_id, survivor.contentHash, size);
  } else {
    ContentFile playlist;
    playlist.m_type = ContentType::Disc;
    playlist.m_filePath = plan->path;
    playlist.m_fileSizeBytes = size;
    playlist.m_platformId = static_cast<int>(survivor.platformId);
    // The identity of a playlist is its first disc's, so this attaches to the entry that
    // already exists rather than making a second one
    playlist.m_contentHash = survivor.contentHash;
    playlist.m_contentDirectoryId = -1;

    if (!m_library.create(playlist)) {
      return false;
    }

    contentFileId = playlist.m_id;
  }

  const auto configurations = m_library.getRunConfigurations(survivor.contentHash);
  const auto hasPlaylistWayIn =
      isAlreadyRegistered && std::ranges::any_of(configurations, [&](const RunConfiguration &configuration) {
        return configuration.contentFileId == contentFileId && configuration.type == RunConfiguration::TYPE_PLAYLIST;
      });

  // Cataloguing a file gives it an ordinary way in of its own, and the set wants the one that
  // says it is a playlist. Left alone when it is already that, so the entry is never briefly
  // left with no way in at all
  if (!hasPlaylistWayIn) {
    m_library.deleteRunConfigurationsForContentFile(contentFileId);
    m_library.createRunConfiguration(contentFileId, plan->path, static_cast<int>(survivor.platformId),
                                     survivor.contentHash, RunConfiguration::TYPE_PLAYLIST);
  }

  // Every disc is reached through the playlist, so a disc's own way in would be a second thing
  // for the resolver to choose between and a second memory card for the core to write
  for (const auto &disc : m_library.getDiscsInSet(setId)) {
    m_library.deleteRunConfigurationsForContentFile(disc.m_id);
  }

  // Removing the last way in hides an entry, and the replacement arriving afterwards does not
  // bring it back on its own
  if (auto entry = m_library.getEntryWithContentHash(survivor.contentHash); entry.has_value() && entry->hidden) {
    entry->hidden = false;
    m_library.update(*entry);
  }

  set->playlistPath = plan->path;
  set->playlistOwned = isOurs;
  m_library.updateDiscSet(*set);

  spdlog::info("Set {} launches through {}", setId, plan->path);
  return true;
}

} // namespace firelight::library
