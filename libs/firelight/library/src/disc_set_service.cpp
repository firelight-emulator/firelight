#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/filename_tags.hpp>
#include <firelight/library/game_identity.hpp>
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
  explicit ApplyGuard(bool &flag) : m_flag(flag), m_previous(std::exchange(flag, true)) {}

  ~ApplyGuard() { m_flag = m_previous; }

private:
  bool &m_flag;
  bool m_previous;
};
} // namespace

thread_local bool DiscSetService::s_applying = false;

DiscSetService::DiscSetService(IUserLibraryRepository &library, std::string appDataDirectory)
    : m_library(library), m_appDataDirectory(std::move(appDataDirectory)) {
  // The title a set is matched on is written by metadata population, so a decision made when
  // the content file arrived would have nothing to match on yet
  m_entryUpdatedConnection =
      EventDispatcher::instance().subscribe<EntryUpdatedEvent>([this](const EntryUpdatedEvent &event) {
        if (s_applying) {
          return;
        }

        autoGroupDiscs(event.entryId);
      });
}

bool DiscSetService::autoGroupDiscs(const int entryId) {
  const ApplyGuard guard(s_applying);

  const auto entry = m_library.getEntry(entryId);

  if (!entry.has_value() || entry->discNumber == 0 || entry->discSetUserSet) {
    return false;
  }

  const auto identity = identityOf(*entry);

  if (identity.isEmpty()) {
    return false;
  }

  // Before grouping gives up: a set that has already formed reaches no further, and the count
  // arrives with metadata long after
  recordDiscCount(*entry);

  std::vector<Entry> members{*entry};

  for (const auto peerId : m_library.getCandidateEntryIds(identity)) {
    if (peerId == entryId) {
      continue;
    }

    const auto peer = m_library.getEntry(peerId);

    // A disc somebody took out by hand is left where they put it, and a game that is not a
    // disc of anything has nothing to join
    if (!peer.has_value() || peer->discSetUserSet || peer->discNumber == 0) {
      continue;
    }

    if (!areDiscsOfOneRelease(identity, identityOf(*peer))) {
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

  auto anchorId = -1;

  for (const auto &file : m_library.getContentFilesWithContentHash(survivor.contentHash)) {
    // The set's own playlist carries the same hash as its first disc. It addresses the discs
    // rather than being one, so it must not join them
    if (file.m_discNumber == 0) {
      continue;
    }

    m_library.setContentFileDiscSet(file.m_id, *setId);

    if (anchorId < 0) {
      anchorId = file.m_id;
    }
  }

  // TODO
  // The set's way in, anchored on the disc it takes its identity from. A database row rather
  // than a consequence of a file write, so a folder we cannot write to still leaves the game
  // launchable
  if (anchorId >= 0) {
    m_library.createRunConfigurationForSet(*setId, anchorId, survivor.contentHash);
  }

  // Every disc is reached through the set, so a disc's own way in would be a second thing for
  // the resolver to choose between and a second memory card for the core to write
  for (const auto &disc : m_library.getDiscsInSet(*setId)) {
    m_library.deleteRunConfigurationsForContentFile(disc.m_id);
  }

  if (survivor.discSetId != setId) {
    m_library.setEntryDiscSet(survivor.id, *setId, false);
    changed = true;
  }

  if (materializePlaylist(*setId, survivor.contentHash)) {
    changed = true;
  }

  return changed;
}

bool DiscSetService::detachDisc(const int contentFileId) {
  const auto file = m_library.getContentFile(contentFileId);

  if (!file.has_value() || !file->m_discSetId.has_value()) {
    return false;
  }

  const auto setId = *file->m_discSetId;
  const auto set = m_library.getDiscSet(setId);
  const ApplyGuard guard(s_applying);

  restoreOwnEntry(*file, set.has_value() ? set->title : "", true);

  if (!dissolveIfUndersized(setId)) {
    // The playlist still names the disc that just left
    if (const auto survivor = survivorOf(setId)) {
      materializePlaylist(setId, survivor->contentHash);
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

void DiscSetService::recordDiscCount(const Entry &entry) {
  if (!entry.discSetId.has_value() || entry.metadata.discCount <= 0) {
    return;
  }

  if (auto set = m_library.getDiscSet(*entry.discSetId);
      set.has_value() && set->discCount != entry.metadata.discCount) {
    set->discCount = entry.metadata.discCount;
    m_library.updateDiscSet(*set);
  }
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
  set.discCount = survivor.metadata.discCount;

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

  // TODO
  // A playlist is named after the identity it launches under, so one named after an entry that
  // no longer exists is a file nothing will ever open again
  retirePlaylist(absorbed.contentHash);

  spdlog::info("Disc {} folded into entry {} as part of set {}", absorbed.discNumber, survivor.id, setId);

  EventDispatcher::instance().publish(EntryAbsorbedEvent{.survivingEntryId = survivor.id,
                                                         .absorbedContentHash = absorbed.contentHash,
                                                         .survivingContentHash = survivor.contentHash});
  return true;
}

bool DiscSetService::restoreOwnEntry(const ContentFile &file, const std::string &title, const bool isUserChoice) {
  m_library.setContentFileDiscSet(file.m_id, std::nullopt);

  const auto configurations = m_library.getRunConfigurations(file.m_contentHash);

  // A set's way in is anchored on one of its discs, so without the type this reads that as the
  // disc already having one of its own
  const auto isLaunchable = std::ranges::any_of(configurations, [&](const RunConfiguration &configuration) {
    return configuration.contentFileId == file.m_id && configuration.type == RunConfiguration::TYPE_ROM;
  });

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
  const auto survivor = survivorOf(setId);

  // Before the discs get their own entries back, so the identity the playlist was named after
  // is still there to be read
  for (const auto &disc : discs) {
    restoreOwnEntry(disc, set.has_value() ? set->title : "", false);
  }

  if (survivor.has_value()) {
    retirePlaylist(survivor->contentHash);
  }

  return m_library.deleteDiscSet(setId);
}

void DiscSetService::retirePlaylist(const std::string &contentHash) {
  std::lock_guard lock(m_playlistMutex);
  std::error_code ec;
  std::filesystem::remove(playlistPathFor(contentHash, m_appDataDirectory), ec);
}

std::optional<Entry> DiscSetService::survivorOf(const int setId) {
  // Asked of the entries rather than walked back from the discs: an absorbed disc has no entry,
  // so a set whose surviving disc went missing would otherwise look like it had nobody at all
  auto entries = m_library.getEntriesInDiscSet(setId);

  if (entries.empty()) {
    return std::nullopt;
  }

  std::ranges::sort(entries, [](const Entry &left, const Entry &right) {
    return std::tie(left.discNumber, left.id) < std::tie(right.discNumber, right.id);
  });

  return entries.front();
}

bool DiscSetService::materializePlaylist(const int setId, const std::string &contentHash) {
  const auto plan = planPlaylist(m_library.getPresentDiscsInSet(setId), contentHash, m_appDataDirectory);

  if (!plan.has_value()) {
    return false;
  }

  std::lock_guard lock(m_playlistMutex);
  std::error_code ec;

  if (std::filesystem::exists(plan->path, ec)) {
    std::ifstream existingPlaylist(plan->path, std::ios::binary);
    const std::string existing((std::istreambuf_iterator<char>(existingPlaylist)), std::istreambuf_iterator<char>());

    if (existing == plan->contents) {
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
