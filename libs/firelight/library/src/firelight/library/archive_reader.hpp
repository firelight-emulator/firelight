#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <set>
#include <string>
#include <vector>

namespace firelight::library {

// A thin std-typed wrapper over libarchive, centralizing the archive handling
// that was previously reimplemented across the scanner, identifier, and loader
class ArchiveReader {
public:
  struct Entry {
    std::string pathName; // full entry path within the archive
    std::string baseName; // file name only
    int64_t size = 0;
  };

  explicit ArchiveReader(std::string archivePath);

  // Lists every entry (reads headers only, no payload)
  [[nodiscard]] std::vector<Entry> listEntries() const;

  // Reads the full bytes of the entry with the exact path name (empty if none)
  [[nodiscard]] std::vector<uint8_t>
  readEntryByPath(const std::string &entryPath) const;

  // Reads the full bytes of the first entry whose base name matches
  // (case-insensitive); empty if none
  [[nodiscard]] std::vector<uint8_t>
  readEntryByBaseName(const std::string &baseNameLower) const;

  // Streams every entry whose lower-cased base name is in `wantedBaseNamesLower`
  // to `destDir/<baseName>`. Returns false if the archive could not be opened
  bool extractEntries(const std::set<std::string> &wantedBaseNamesLower,
                      const std::filesystem::path &destDir) const;

  // TODO
  // Single-pass iteration: invokes `fn` for each entry with a lazy reader that,
  // when called, returns the current entry's bytes. Entries whose reader is not
  // invoked are skipped without reading their payload
  void forEachEntry(
      const std::function<void(const Entry &,
                               const std::function<std::vector<uint8_t>()> &)>
          &fn) const;

private:
  std::string m_archivePath;
};

} // namespace firelight::library
