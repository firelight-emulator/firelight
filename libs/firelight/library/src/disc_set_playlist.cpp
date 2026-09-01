// TODO: NEEDS REVIEW
#include <firelight/library/disc_set_playlist.hpp>
#include <firelight/util/strings.hpp>

#include <algorithm>
#include <tuple>
#include <unordered_set>

namespace firelight::library {

bool isGeneratedPlaylist(const std::string &contents) { return strings::startsWith(contents, PLAYLIST_MARKER); }

std::string playlistPathFor(const std::string &contentHash, const std::string &appDataDirectory) {
  return appDataDirectory + "/playlists/" + contentHash + ".m3u";
}

std::optional<PlaylistPlan> planPlaylist(const std::vector<ContentFile> &discs, const std::string &contentHash,
                                         const std::string &appDataDirectory) {
  if (contentHash.empty()) {
    return std::nullopt;
  }

  auto ordered = discs;
  std::ranges::sort(ordered, [](const ContentFile &left, const ContentFile &right) {
    return std::tie(left.m_discNumber, left.m_inArchive, left.m_filePath) <
           std::tie(right.m_discNumber, right.m_inArchive, right.m_filePath);
  });

  // TODO
  // One disc dumped twice is two files sharing a hash, and listing both would offer the core
  // the same disc under two indices. Ordering puts the openable copy first, and an unhashed
  // file matches nothing rather than matching every other unhashed one
  std::unordered_set<std::string> seenHashes;
  std::erase_if(ordered, [&seenHashes](const ContentFile &disc) {
    return !disc.m_contentHash.empty() && !seenHashes.insert(disc.m_contentHash).second;
  });

  // TODO
  // A set launches through its playlist whatever its size, so the floor is having something to
  // name rather than having two of them
  if (ordered.empty()) {
    return std::nullopt;
  }

  for (const auto &disc : ordered) {
    // A path inside an archive is not a path the core can open
    if (disc.m_inArchive || disc.m_filePath.empty()) {
      return std::nullopt;
    }
  }

  PlaylistPlan plan;
  plan.path = playlistPathFor(contentHash, appDataDirectory);

  // TODO
  // Says whose file this is, so overwriting or deleting it is never done to something somebody
  // else put at the same path
  plan.contents += PLAYLIST_MARKER;
  plan.contents += "\n";

  // TODO
  // Absolute, because the file lives nowhere near the discs and is rewritten from the database
  // whenever it is missing rather than carried anywhere
  for (const auto &disc : ordered) {
    plan.contents += disc.m_filePath;
    plan.contents += "\n";
  }

  return plan;
}

} // namespace firelight::library
