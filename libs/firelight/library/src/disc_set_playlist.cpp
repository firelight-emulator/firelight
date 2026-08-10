#include <firelight/library/disc_set_playlist.hpp>
#include <firelight/util/strings.hpp>

#include <algorithm>
#include <string_view>
#include <tuple>

namespace firelight::library {

namespace {
// The characters Windows refuses in a name, plus the separators
constexpr std::string_view ILLEGAL_FILENAME_CHARACTERS = "<>:\"/\\|?*";

std::string directoryOf(const std::string &path) {
  const auto separator = path.find_last_of("/\\");
  return separator == std::string::npos ? "" : path.substr(0, separator);
}

std::string baseNameOf(const std::string &path) {
  const auto separator = path.find_last_of("/\\");
  return separator == std::string::npos ? path : path.substr(separator + 1);
}

std::string stemOf(const std::string &path) {
  const auto name = baseNameOf(path);
  const auto dot = name.find_last_of('.');
  return dot == std::string::npos || dot == 0 ? name : name.substr(0, dot);
}

// The deepest directory holding every disc. Empty when they do not share one at all, which
// on Windows means different drives
std::string commonDirectoryOf(const std::vector<ContentFile> &discs) {
  auto common = directoryOf(discs.front().m_filePath);

  for (const auto &disc : discs) {
    const auto directory = directoryOf(disc.m_filePath);

    while (!common.empty() && !(directory == common || strings::startsWith(directory, common + "/"))) {
      const auto separator = common.find_last_of('/');
      common = separator == std::string::npos ? "" : common.substr(0, separator);
    }
  }

  return common;
}
} // namespace

bool isGeneratedPlaylist(const std::string &contents) { return strings::startsWith(contents, PLAYLIST_MARKER); }

std::string sanitizeFileName(const std::string &name) {
  std::string sanitized;

  for (const auto character : name) {
    if (static_cast<unsigned char>(character) < 0x20) {
      continue;
    }

    if (ILLEGAL_FILENAME_CHARACTERS.find(character) != std::string_view::npos) {
      continue;
    }

    sanitized.push_back(character);
  }

  while (!sanitized.empty() && (sanitized.back() == '.' || sanitized.back() == ' ')) {
    sanitized.pop_back();
  }

  return strings::trim(sanitized);
}

std::optional<PlaylistPlan> planPlaylist(const std::vector<ContentFile> &discs, const int setId,
                                         const std::string &title, const PlaylistLocation location,
                                         const std::string &appDataDirectory) {
  // A one-line playlist buys nothing, and gaining a second disc does not move the identity
  // because the first line is still the same disc
  if (discs.size() < 2) {
    return std::nullopt;
  }

  for (const auto &disc : discs) {
    // A path inside an archive is not a path the core can open
    if (disc.m_inArchive || disc.m_filePath.empty()) {
      return std::nullopt;
    }
  }

  auto ordered = discs;
  std::ranges::sort(ordered, [](const ContentFile &left, const ContentFile &right) {
    return std::tie(left.m_discNumber, left.m_filePath) < std::tie(right.m_discNumber, right.m_filePath);
  });

  auto name = sanitizeFileName(title);

  if (name.empty()) {
    name = sanitizeFileName(stemOf(ordered.front().m_filePath));
  }

  if (name.empty()) {
    return std::nullopt;
  }

  const auto commonDirectory = commonDirectoryOf(ordered);

  PlaylistPlan plan;

  if (location == PlaylistLocation::AppData) {
    // Two sets can be called the same thing, and this file is ours rather than something
    // anybody browses to
    plan.path = appDataDirectory + "/playlists/" + std::to_string(setId) + " - " + name + ".m3u";
  } else if (commonDirectory.empty()) {
    // Nothing holds them all, so there is no relative line that reaches every disc
    plan.path = directoryOf(ordered.front().m_filePath) + "/" + name + ".m3u";
  } else {
    // Beside the discs when they share a folder, and beside their folders when each disc has
    // one of its own. Somebody looking for the game's playlist looks where the game is
    plan.path = commonDirectory + "/" + name + ".m3u";
    plan.isPortable = true;
  }

  plan.contents = std::string(PLAYLIST_MARKER) + "\n";

  for (const auto &disc : ordered) {
    plan.contents += plan.isPortable ? disc.m_filePath.substr(commonDirectory.size() + 1) : disc.m_filePath;
    plan.contents += "\n";
  }

  return plan;
}

} // namespace firelight::library
