// TODO: NEEDS REVIEW
#include "firelight/library/chd_cdreader.hpp"
#include "firelight/library/rc_hash_logging.hpp"

#include <firelight/library/archive_reader.hpp>
#include <firelight/library/content_extensions.hpp>
#include <firelight/library/disc_inspector.hpp>
#include <firelight/library/file_bytes.hpp>
#include <firelight/platforms/platform_service.hpp>
#include <firelight/util/strings.hpp>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <rcheevos/rc_hash.h>
#include <set>
#include <spdlog/spdlog.h>
#include <unordered_map>

namespace firelight::library {

DiscInspector::DiscInspector(platforms::IPlatformService &platformService) : m_platformService(platformService) {
  // rcheevos' own reader cannot open a CHD, so without this every CHD fails to identify
  // and is never catalogued
  installChdCdReader();
  installRcHashLogging();
}

namespace {

// Upper bound on how much we'll extract from an archive to identify a disc
// CD-based systems all fit well under this; oversized DVD images zipped up are
// skipped rather than risk filling the user's temp drive
constexpr int64_t MAX_IN_ARCHIVE_DISC_EXTRACT_BYTES = 2LL * 1024 * 1024 * 1024;

std::string baseNameOf(const std::string &path) { return std::filesystem::path(path).filename().string(); }

// A unique temporary directory removed when this object goes out of scope
class TempDir {
public:
  TempDir() {
    std::error_code ec;
    const auto base = std::filesystem::temp_directory_path(ec);
    if (ec) {
      return;
    }
    std::random_device rd;
    for (int attempt = 0; attempt < 100; ++attempt) {
      auto candidate = base / ("fl_disc_" + std::to_string(rd()));
      if (std::filesystem::create_directory(candidate, ec) && !ec) {
        m_path = candidate;
        m_valid = true;
        return;
      }
    }
  }

  ~TempDir() {
    if (m_valid) {
      std::error_code ec;
      std::filesystem::remove_all(m_path, ec);
    }
  }

  TempDir(const TempDir &) = delete;
  TempDir &operator=(const TempDir &) = delete;

  [[nodiscard]] bool valid() const { return m_valid; }

  [[nodiscard]] const std::filesystem::path &path() const { return m_path; }

private:
  std::filesystem::path m_path;
  bool m_valid = false;
};
} // namespace

namespace {
// TODO
// rcheevos ends some candidate lists with a console that hashes the whole file rather than
// reading the format, so it matches anything. Taking one files the content under a hash that
// means nothing, which is worse than not identifying it: the hash is what saves are keyed on
bool isWholeFileFallback(const int rcConsole) {
  return rcConsole == RC_CONSOLE_GAMEBOY || rcConsole == RC_CONSOLE_MEGA_DRIVE;
}

// Enough to reach the ISO9660 descriptor at 0x8001
constexpr size_t HEADER_BYTES = 0x8010;

std::vector<uint8_t> readHeader(const std::string &path) {
  std::ifstream file(path, std::ios::binary);

  if (!file) {
    return {};
  }

  std::vector<uint8_t> header(HEADER_BYTES);
  file.read(reinterpret_cast<char *>(header.data()), static_cast<std::streamsize>(header.size()));
  header.resize(static_cast<size_t>(file.gcount()));
  return header;
}

bool matchesAt(const std::vector<uint8_t> &bytes, const size_t offset, const std::string_view text) {
  return bytes.size() >= offset + text.size() && std::memcmp(bytes.data() + offset, text.data(), text.size()) == 0;
}

// The 12-byte sync pattern every raw 2352-byte sector opens with
bool hasRawSectorSync(const std::vector<uint8_t> &bytes) {
  static constexpr uint8_t SYNC[12] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
  return bytes.size() >= sizeof(SYNC) && std::memcmp(bytes.data(), SYNC, sizeof(SYNC)) == 0;
}
} // namespace

DiscIdentity DiscInspector::classifyByContent(const std::string &path, const IdentifyOutcome discOutcome) const {
  DiscIdentity identity;
  identity.outcome = discOutcome;

  const auto header = readHeader(path);

  // Disc structure, so the dump is a disc nothing could read rather than a cartridge. Checked
  // first because a Mega CD boot sector also carries a cartridge header
  if (hasRawSectorSync(header) || matchesAt(header, 0x000, "SEGADISCSYSTEM") ||
      matchesAt(header, 0x010, "SEGADISCSYSTEM") || matchesAt(header, 0x8001, "CD001")) {
    return identity;
  }

  if (!matchesAt(header, 0x100, "SEGA")) {
    return identity;
  }

  identity.isCartridge = true;
  identity.outcome = IdentifyOutcome::Identified;
  identity.platformId = matchesAt(header, 0x100, "SEGA 32X")
                            ? firelight::platforms::PlatformService::PLATFORM_ID_SEGA_32X
                            : firelight::platforms::PlatformService::PLATFORM_ID_SEGA_GENESIS;

  spdlog::debug("Read {} as a cartridge dump for platform {}", path, identity.platformId);
  return identity;
}

bool DiscInspector::isSaturn(rc_hash_iterator &iterator) {
  auto &cdreader = iterator.callbacks.cdreader;
  if (!cdreader.open_track_iterator || !cdreader.read_sector) {
    return false;
  }

  // Mirror rcheevos' rc_hash_sega_cd: open the first track and read absolute
  // sector 0, where the 16-byte volume magic lives
  void *track = cdreader.open_track_iterator(iterator.path, 1, &iterator);
  if (!track) {
    return false;
  }

  uint8_t buffer[16] = {0};
  cdreader.read_sector(track, 0, buffer, sizeof(buffer));
  cdreader.close_track(track);

  return std::memcmp(buffer, "SEGA SEGASATURN ", 16) == 0;
}

std::vector<std::string> DiscInspector::sheetFilenameCandidates(const std::vector<uint8_t> &sheetBytes) {
  std::vector<std::string> candidates;
  const std::string text(reinterpret_cast<const char *>(sheetBytes.data()), sheetBytes.size());

  size_t start = 0;
  while (start <= text.size()) {
    const size_t newline = text.find('\n', start);
    const size_t end = (newline == std::string::npos) ? text.size() : newline;
    std::string line = text.substr(start, end - start);
    start = (newline == std::string::npos) ? text.size() + 1 : newline + 1;

    // Trim surrounding whitespace (including any \r)
    size_t b = 0;
    size_t e = line.size();
    while (b < e && std::isspace(static_cast<unsigned char>(line[b]))) {
      ++b;
    }
    while (e > b && std::isspace(static_cast<unsigned char>(line[e - 1]))) {
      --e;
    }
    line = line.substr(b, e - b);
    if (line.empty()) {
      continue;
    }

    candidates.push_back(line); // whole line (m3u entries, may contain spaces)

    // Quoted strings (cue FILE "name.bin")
    size_t p = 0;
    bool inQuote = false;
    std::string quoted;
    while (p < line.size()) {
      if (line[p] == '"') {
        if (inQuote) {
          candidates.push_back(quoted);
          quoted.clear();
        }
        inQuote = !inQuote;
      } else if (inQuote) {
        quoted += line[p];
      }
      ++p;
    }

    // Whitespace-separated fields (gdi track rows)
    std::string field;
    for (const char c : line) {
      if (std::isspace(static_cast<unsigned char>(c))) {
        if (!field.empty()) {
          candidates.push_back(field);
          field.clear();
        }
      } else {
        field += c;
      }
    }
    if (!field.empty()) {
      candidates.push_back(field);
    }
  }

  return candidates;
}

// TODO
// The discs a playlist names, one per line. Blank lines and comments are not discs, and rc_hash
// skips them the same way when it resolves a playlist to the disc it stands for
std::vector<std::string> DiscInspector::playlistLines(const std::vector<uint8_t> &bytes) {
  std::vector<std::string> lines;
  const std::string text(reinterpret_cast<const char *>(bytes.data()), bytes.size());

  size_t start = 0;
  while (start <= text.size()) {
    const size_t newline = text.find('\n', start);
    const size_t end = newline == std::string::npos ? text.size() : newline;
    const auto line = strings::trim(text.substr(start, end - start));
    start = newline == std::string::npos ? text.size() + 1 : newline + 1;

    if (line.empty() || line.front() == '#') {
      continue;
    }

    lines.push_back(line);
  }

  return lines;
}

std::string DiscInspector::roleForBaseName(const std::string &baseNameLower) {
  return firelight::library::isDiscSheetExtension(suffixOf(baseNameLower)) ? "disc" : "track";
}

DiscIdentity DiscInspector::detect(const std::string &discFilePath) const {
  DiscIdentity identity;

  rc_hash_iterator_t iterator;
  std::memset(&iterator, 0, sizeof(iterator));
  rc_hash_initialize_iterator(&iterator, discFilePath.c_str(), nullptr, 0);

  // rcheevos tries each candidate console (chosen by extension) in order,
  // running that console's content fingerprint. The first one that matches is
  // the real platform, and the hash it produces is the canonical RA hash
  char hash[33];
  while (rc_hash_iterate(hash, &iterator)) {
    const int rcConsole = iterator.consoles[iterator.index - 1];

    // Game Boy is what rcheevos falls back to for an extension it has no handler for, and Mega
    // Drive is what it falls back to for a .bin it could not read. Both hash the whole file
    // rather than reading the format, so they match anything and mean nothing
    if (isWholeFileFallback(rcConsole)) {
      spdlog::debug("Ignoring the whole-file fallback (rc console {}) for disc image: {}", rcConsole, discFilePath);
      continue;
    }

    int platformId = m_platformService.platformIdForRcConsole(rcConsole);

    // TODO
    // The format was read and named a system, so the file is fine and the dump is fine. Keeping
    // the console's name is what turns "could not catalogue this" into "you own GameCube discs"
    if (platformId == firelight::platforms::PlatformService::PLATFORM_ID_UNKNOWN) {
      identity.outcome = IdentifyOutcome::PlatformNotSupported;
      identity.identifiedAs = rc_console_name(rcConsole);
      spdlog::debug("Detected disc {} as {} (rc console {}), which has no platform", discFilePath,
                    identity.identifiedAs, rcConsole);
      continue;
    }

    // rcheevos uses the Sega CD console as an umbrella that also matches Sega
    // Saturn discs; disambiguate via the sector-0 magic
    if (rcConsole == RC_CONSOLE_SEGA_CD && isSaturn(iterator)) {
      platformId = firelight::platforms::PlatformService::PLATFORM_ID_SEGA_SATURN;
    }

    identity.outcome = IdentifyOutcome::Identified;
    identity.platformId = platformId;
    identity.contentHash = std::string(hash);
    // An earlier candidate may have been a console with no platform; this one won
    identity.identifiedAs.clear();
    spdlog::debug("Detected disc {} as platform {} (rc console {}), hash {}", discFilePath, platformId, rcConsole,
                  hash);
    break;
  }

  // rcheevos offers a .iso to PS2, PSP, 3DO, Sega CD, GameCube and Wii, and to no PlayStation
  // at all, so a PS1 disc dumped as a plain .iso matches nothing and disappears. Asking here
  // rather than adding a candidate, because the list is rebuilt on the first iteration
  if (!identity.isIdentified() && strings::endsWithIgnoringCase(discFilePath, ".iso")) {
    char playstationHash[33] = {0};

    if (rc_hash_generate(playstationHash, RC_CONSOLE_PLAYSTATION, &iterator) != 0) {
      identity.outcome = IdentifyOutcome::Identified;
      identity.platformId = m_platformService.platformIdForRcConsole(RC_CONSOLE_PLAYSTATION);
      identity.contentHash = std::string(playstationHash);
      spdlog::debug("Detected iso {} as a PlayStation disc, hash {}", discFilePath, playstationHash);
    }
  }

  // TODO
  // .img and .mdf hold the same raw sector data a .bin does, and rcheevos has no handler for
  // either, so each console has to be asked outright. RC_CONSOLE_MEGA_DRIVE is deliberately
  // absent: it hashes the whole file and therefore cannot fail, which would file every
  // unreadable image as a Genesis game under a hash that means nothing
  if (!identity.isIdentified() &&
      (strings::endsWithIgnoringCase(discFilePath, ".img") || strings::endsWithIgnoringCase(discFilePath, ".mdf"))) {
    for (const auto rcConsole : RAW_SECTOR_CONSOLES) {
      char rawHash[33] = {0};

      if (rc_hash_generate(rawHash, rcConsole, &iterator) == 0) {
        continue;
      }

      auto platformId = m_platformService.platformIdForRcConsole(rcConsole);

      if (rcConsole == RC_CONSOLE_SEGA_CD && isSaturn(iterator)) {
        platformId = firelight::platforms::PlatformService::PLATFORM_ID_SEGA_SATURN;
      }

      identity.outcome = IdentifyOutcome::Identified;
      identity.platformId = platformId;
      identity.contentHash = std::string(rawHash);
      spdlog::debug("Detected raw image {} as platform {}, hash {}", discFilePath, platformId, rawHash);
      break;
    }
  }

  // TODO
  // A console with no platform already says more than either of these: something read the format
  // and named the system, so neither "nothing could read it" nor "the bytes matched nothing" is true
  const auto isConsoleWithoutPlatform = identity.outcome == IdentifyOutcome::PlatformNotSupported;

  // TODO
  // rcheevos puts a lone Game Boy in the candidate list when it has no handler for the extension,
  // so this says nothing could ever have read the file rather than that the dump did not match
  if (!identity.isIdentified() && !isConsoleWithoutPlatform) {
    const auto hasNoHandler = iterator.consoles[0] == RC_CONSOLE_GAMEBOY && iterator.consoles[1] == 0;
    identity.outcome = hasNoHandler ? IdentifyOutcome::NoIdentifier : IdentifyOutcome::NotRecognized;
  }

  rc_hash_destroy_iterator(&iterator);

  // Nothing read it as a disc, so the bytes get to say what they are. Only now, because a Mega
  // CD boot sector carries a Mega Drive cartridge header at 0x100 by design and would claim a
  // disc if it were asked first
  if (!identity.isIdentified() && !isConsoleWithoutPlatform) {
    identity = classifyByContent(discFilePath, identity.outcome);
  }

  if (!identity.isIdentified()) {
    spdlog::debug("Could not identify disc image: {}", discFilePath);
  }

  return identity;
}

std::vector<IdentifiedDiscMember> DiscInspector::collectLooseMembers(const std::string &sheetPath) const {
  std::vector<IdentifiedDiscMember> members;

  const std::vector<uint8_t> bytes = readAllBytes(sheetPath);
  if (bytes.empty()) {
    return members;
  }

  const std::filesystem::path sheet(sheetPath);
  const std::filesystem::path dir = sheet.parent_path();
  const std::string sheetNameLower = strings::toLower(sheet.filename().string());

  // TODO
  // Every line of a playlist is a disc, so one naming a file nobody has yet is kept: dropping it
  // would slide the discs after it up a number. A sheet is read the other way round, from every
  // field of every line, so there the file having to exist is what tells a path from the words
  // around it
  const auto isPlaylist = suffixOf(sheetNameLower) == "m3u";
  const auto candidates = isPlaylist ? playlistLines(bytes) : sheetFilenameCandidates(bytes);

  std::set<std::string> seen;
  for (const auto &token : candidates) {
    const std::string base = baseNameOf(token);
    const std::string baseLower = strings::toLower(base);

    if (baseLower.empty() || baseLower == sheetNameLower) {
      continue;
    }

    // TODO
    // A line is tried as written before falling back to the name alone, because reducing to the
    // name first loses every set laid out one folder per disc
    auto written = token;
    std::ranges::replace(written, '\\', '/');

    std::error_code ec;
    auto resolved = (dir / std::filesystem::path(written)).lexically_normal();

    if (!std::filesystem::exists(resolved, ec)) {
      const auto byName = dir / base;

      if (std::filesystem::exists(byName, ec)) {
        resolved = byName;
      } else if (!isPlaylist) {
        continue;
      }
    }

    // TODO
    // Keyed on where the line landed rather than what it was called, so one folder per disc with
    // the same name in each stays several discs
    if (!seen.insert(strings::toLower(resolved.generic_string())).second) {
      continue;
    }

    members.push_back({resolved.generic_string(), roleForBaseName(baseLower)});
  }

  return members;
}

DiscIdentity DiscInspector::inspectFile(const std::string &path, std::vector<IdentifiedDiscMember> &outMembers) const {
  const DiscIdentity identity = detect(path);
  if (identity.isIdentified() && firelight::library::isDiscSheetExtension(suffixOf(path))) {
    outMembers = collectLooseMembers(path);
  }
  return identity;
}

DiscIdentity DiscInspector::inspectArchiveEntry(const std::string &archivePath, const std::string &entryName,
                                                std::vector<IdentifiedDiscMember> &outMembers) const {
  const ArchiveReader reader(archivePath);
  const std::string targetBaseLower = strings::toLower(baseNameOf(entryName));

  std::unordered_map<std::string, int64_t> sizeByBase;
  for (const auto &entry : reader.listEntries()) {
    sizeByBase[strings::toLower(entry.baseName)] = entry.size;
  }
  if (!sizeByBase.contains(targetBaseLower)) {
    return {};
  }

  // Minimal set: the target plus the files a cue/gdi/m3u sheet references
  // (resolved transitively, since an m3u can point at cue sheets)
  std::set<std::string> wanted;
  std::vector<std::string> worklist{targetBaseLower};
  while (!worklist.empty()) {
    const std::string name = worklist.back();
    worklist.pop_back();
    if (wanted.contains(name)) {
      continue;
    }
    wanted.insert(name);

    if (firelight::library::isDiscSheetExtension(suffixOf(name))) {
      for (const auto &token : sheetFilenameCandidates(reader.readEntryByBaseName(name))) {
        const std::string candidate = strings::toLower(baseNameOf(token));
        if (!candidate.empty() && sizeByBase.contains(candidate)) {
          worklist.push_back(candidate);
        }
      }
    }
  }

  int64_t totalBytes = 0;
  for (const auto &name : wanted) {
    totalBytes += sizeByBase[name];
  }
  if (totalBytes > MAX_IN_ARCHIVE_DISC_EXTRACT_BYTES) {
    spdlog::warn("Skipping in-archive disc {}: needs {} bytes extracted, over "
                 "the {} byte cap",
                 entryName, totalBytes, MAX_IN_ARCHIVE_DISC_EXTRACT_BYTES);
    return {};
  }

  for (const auto &name : wanted) {
    if (name != targetBaseLower) {
      outMembers.push_back({name, roleForBaseName(name)});
    }
  }

  TempDir temp;
  if (!temp.valid()) {
    spdlog::error("Could not create temp dir for in-archive disc: {}", archivePath);
    return {};
  }
  if (!reader.extractEntries(wanted, temp.path())) {
    return {};
  }

  return detect((temp.path() / baseNameOf(entryName)).string());
}

} // namespace firelight::library
