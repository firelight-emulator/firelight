#include <firelight/library/disc_inspector.hpp>

#include <firelight/library/archive_reader.hpp>
#include <firelight/library/file_bytes.hpp>

#include <rcheevos/rc_hash.h>

#include <firelight/platforms/platform_service.hpp>
#include <firelight/library/content_extensions.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <set>
#include <unordered_map>

namespace firelight::library {

DiscInspector::DiscInspector(platforms::IPlatformService &platformService)
    : m_platformService(platformService) {}

namespace {

// TODO
// Upper bound on how much we'll extract from an archive to identify a disc
// CD-based systems all fit well under this; oversized DVD images zipped up are
// skipped rather than risk filling the user's temp drive
constexpr int64_t MAX_IN_ARCHIVE_DISC_EXTRACT_BYTES = 2LL * 1024 * 1024 * 1024;

std::string toLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

std::string baseNameOf(const std::string &path) {
  return std::filesystem::path(path).filename().string();
}

std::string suffixOf(const std::string &name) {
  const auto dot = name.find_last_of('.');
  if (dot == std::string::npos) {
    return {};
  }
  return toLower(name.substr(dot + 1));
}

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

std::vector<std::string>
DiscInspector::sheetFilenameCandidates(const std::vector<uint8_t> &sheetBytes) {
  std::vector<std::string> candidates;
  const std::string text(reinterpret_cast<const char *>(sheetBytes.data()),
                         sheetBytes.size());

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

std::string DiscInspector::roleForBaseName(const std::string &baseNameLower) {
  return firelight::library::isDiscSheetExtension(suffixOf(baseNameLower)) ? "disc"
                                                                        : "track";
}

DiscIdentity DiscInspector::detect(const std::string &discFilePath) const {
  DiscIdentity identity;

  rc_hash_iterator_t iterator;
  std::memset(&iterator, 0, sizeof(iterator));
  rc_hash_initialize_iterator(&iterator, discFilePath.c_str(), nullptr, 0);

  // TODO
  // rcheevos tries each candidate console (chosen by extension) in order,
  // running that console's content fingerprint. The first one that matches is
  // the real platform, and the hash it produces is the canonical RA hash
  char hash[33];
  while (rc_hash_iterate(hash, &iterator)) {
    const int rcConsole = iterator.consoles[iterator.index - 1];
    int platformId = m_platformService.platformIdForRcConsole(rcConsole);
    if (platformId != firelight::platforms::PlatformService::PLATFORM_ID_UNKNOWN) {
      // rcheevos uses the Sega CD console as an umbrella that also matches Sega
      // Saturn discs; disambiguate via the sector-0 magic
      if (rcConsole == RC_CONSOLE_SEGA_CD && isSaturn(iterator)) {
        platformId = firelight::platforms::PlatformService::PLATFORM_ID_SEGA_SATURN;
      }

      identity.valid = true;
      identity.platformId = platformId;
      identity.contentHash = std::string(hash);
      spdlog::debug("Detected disc {} as platform {} (rc console {}), hash {}",
                    discFilePath, platformId, rcConsole, hash);
      break;
    }
  }

  rc_hash_destroy_iterator(&iterator);

  if (!identity.valid) {
    spdlog::debug("Could not identify disc image: {}", discFilePath);
  }
  return identity;
}

std::vector<IdentifiedDiscMember>
DiscInspector::collectLooseMembers(const std::string &sheetPath) const {
  std::vector<IdentifiedDiscMember> members;

  const std::vector<uint8_t> bytes = readAllBytes(sheetPath);
  if (bytes.empty()) {
    return members;
  }

  const std::filesystem::path sheet(sheetPath);
  const std::filesystem::path dir = sheet.parent_path();
  const std::string sheetNameLower = toLower(sheet.filename().string());

  std::set<std::string> seen;
  for (const auto &token : sheetFilenameCandidates(bytes)) {
    const std::string base = baseNameOf(token);
    const std::string baseLower = toLower(base);
    std::error_code ec;
    if (baseLower.empty() || baseLower == sheetNameLower ||
        seen.contains(baseLower) || !std::filesystem::exists(dir / base, ec)) {
      continue;
    }
    seen.insert(baseLower);
    members.push_back({(dir / base).string(), roleForBaseName(baseLower)});
  }

  return members;
}

DiscIdentity
DiscInspector::inspectFile(const std::string &path,
                           std::vector<IdentifiedDiscMember> &outMembers) const {
  const DiscIdentity identity = detect(path);
  if (identity.valid && firelight::library::isDiscSheetExtension(suffixOf(path))) {
    outMembers = collectLooseMembers(path);
  }
  return identity;
}

DiscIdentity DiscInspector::inspectArchiveEntry(
    const std::string &archivePath, const std::string &entryName,
    std::vector<IdentifiedDiscMember> &outMembers) const {
  const ArchiveReader reader(archivePath);
  const std::string targetBaseLower = toLower(baseNameOf(entryName));

  std::unordered_map<std::string, int64_t> sizeByBase;
  for (const auto &entry : reader.listEntries()) {
    sizeByBase[toLower(entry.baseName)] = entry.size;
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
      for (const auto &token :
           sheetFilenameCandidates(reader.readEntryByBaseName(name))) {
        const std::string candidate = toLower(baseNameOf(token));
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
    spdlog::error("Could not create temp dir for in-archive disc: {}",
                  archivePath);
    return {};
  }
  if (!reader.extractEntries(wanted, temp.path())) {
    return {};
  }

  return detect((temp.path() / baseNameOf(entryName)).string());
}

} // namespace firelight::library
