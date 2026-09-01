// TODO: NEEDS REVIEW
#pragma once

#include <firelight/library/content_file.hpp>
#include <firelight/library/filename_tags.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace firelight::platforms {
class IPlatformService;
}

namespace firelight::library {
struct IdentifiedContent;

/**
 * A file the scan is willing to try to identify, loose on disk or inside an archive.
 *
 * Where the bytes came from is a value here rather than a branch at the call site, which is what
 * lets one pipeline catalogue both. Reading them is deferred, because a disc image is identified
 * from its container rather than from a buffer
 */
struct DiscoveredFile {
  // The entry's name within the archive when archivePath is set, otherwise the path on disk
  std::string path;
  // Empty for a loose file
  std::string archivePath;
  // Lower-cased, no leading dot
  std::string extension;
  size_t sizeBytes = 0;
  bool isDisc = false;

  // TODO
  // What the walk decided this file is. A track or a playlist is recorded like anything else and
  // told apart by this rather than by being left out
  ContentRole role = ContentRole::Dump;
  // What the file name says about the release
  FilenameTags tags;
  // The bytes, read on demand. Empty for a disc, which is identified from its container
  std::function<std::vector<uint8_t>()> readBytes;

  [[nodiscard]] bool isInArchive() const { return !archivePath.empty(); }
};

/**
 * The one mapping from what was discovered and identified to what is stored.
 *
 * A free function because it is a fact about the two records rather than anything the scanner
 * decides, and because one place to assert every field is what keeps a field from being set for a
 * loose file and forgotten for an archived one
 */
[[nodiscard]] ContentFile toContentFile(const DiscoveredFile &file, const IdentifiedContent &identified);

/**
 * Walks a directory and yields the files worth identifying.
 *
 * This is the step that had no home: the earlier split gave identifying, loading, entry creation
 * and launch selection their own types and left walking, triage and the mapping to a catalogue row
 * inline in the scanner, written once per input shape. One triage rule list runs here for a loose
 * file and an archive entry alike, so a field can no longer be set on one path and forgotten on
 * another.
 *
 * Holds no repository: what is found is handed back, and what to record about it is the scanner's
 * decision
 */
class ContentDiscoverer {
public:
  /**
   * What a walk found and did not hand over. Each of these is something the caller records rather
   * than catalogues
   */
  struct Skipped {
    // Whether the walk ran to the end rather than being stopped
    bool completed = false;
    std::vector<std::string> subdirectories;
    // Extension to how many files carried it that nothing accepts
    std::map<std::string, int> unrecognizedExtensions;
    std::vector<DiscoveredFile> tooLarge;
    // Patches, which are catalogued rather than identified
    std::vector<DiscoveredFile> patches;
  };

  // TODO
  // The size past which an unidentified file is recorded rather than read. A disc image can
  // legitimately be larger, so this is only applied to what is neither a disc nor an archive
  static constexpr int64_t MAX_UNRECOGNIZED_FILE_BYTES = 1024LL * 1024 * 1024;

  explicit ContentDiscoverer(const platforms::IPlatformService &platformService,
                             int64_t maxUnrecognizedFileBytes = MAX_UNRECOGNIZED_FILE_BYTES);

  /**
   * Invokes fn once per file worth identifying, in directory order.
   *
   * keepGoing is asked between files, so a shutdown or a requeue stops the walk part way; Skipped
   * says whether it did.
   *
   * includeFiles false still collects subdirectories, because a subdirectory can change without
   * its parent's timestamp moving
   */
  [[nodiscard]] Skipped walk(const std::string &directoryPath, const std::function<void(const DiscoveredFile &)> &fn,
                             const std::function<bool()> &keepGoing, bool includeFiles = true) const;

private:
  /**
   * Every track name the sheets in one directory or archive already speak for.
   *
   * Read once and only when a raw track turns up, because the sheets have to be opened to know
   */
  class TrackSuppressor;

  /**
   * Whether a candidate is worth identifying, and what it is.
   *
   * One rule list, answering for a file on disk and an entry inside an archive alike. Returns
   * nothing for a candidate that belongs in Skipped instead
   */
  [[nodiscard]] std::optional<DiscoveredFile> triage(const std::string &path, const std::string &archivePath,
                                                     size_t sizeBytes, const std::set<std::string> &accepted,
                                                     const std::function<std::vector<uint8_t>()> &readBytes,
                                                     TrackSuppressor &suppressor, Skipped &skipped) const;

  const platforms::IPlatformService &m_platformService;
  int64_t m_maxUnrecognizedFileBytes;
};

} // namespace firelight::library
