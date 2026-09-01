#pragma once

#include <firelight/library/content_hasher.hpp>
#include <firelight/library/disc_inspector.hpp>
#include <firelight/library/identify_outcome.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace firelight::platforms {
class IPlatformService;
}

namespace firelight::library {

// The result of identifying a content file: its platform, canonical content
// hash, and file-level metadata. Anything but Identified is a file we accepted
// and could not catalogue, which the scanner records rather than dropping
struct IdentifiedContent {
  IdentifyOutcome outcome = IdentifyOutcome::NotRecognized;
  bool isDisc = false;
  int platformId = -1; // PlatformService::PLATFORM_ID_UNKNOWN
  std::string contentHash;
  std::string fileMd5;
  size_t fileSizeBytes = 0;
  // The system the format named, when Firelight models no platform for it
  std::string identifiedAs;
  std::vector<IdentifiedDiscMember> discMembers;

  /** Whether this names a real platform and hash */
  [[nodiscard]] bool isIdentified() const { return outcome == IdentifyOutcome::Identified; }
};

// Identifies the platform and computes the canonical content hash of ROM and
// disc files, on disk or inside archives. The scan-time counterpart to
// ContentLoader: it determines *what* a file is without loading it for play
class ContentIdentifier {
public:
  explicit ContentIdentifier(platforms::IPlatformService &platformService);

  // Identifies a loose file on disk
  [[nodiscard]] IdentifiedContent identify(const std::string &path) const;

  // Identifies an archive entry. Cartridge entries are hashed from the provided
  // bytes; disc entries are re-extracted from the archive internally (`data`
  // may be empty for them, with `sizeBytes` giving the entry size)
  [[nodiscard]] IdentifiedContent identifyInArchive(const std::string &entryName, const std::vector<uint8_t> &data,
                                                    size_t sizeBytes, const std::string &archivePath) const;

  // TODO
  // The files a sheet on disk names, without identifying it. A playlist naming a disc nobody has
  // cannot be hashed, and what it says about the rest is worth reading anyway
  [[nodiscard]] std::vector<IdentifiedDiscMember> membersNamedBy(const std::string &sheetPath) const;

private:
  platforms::IPlatformService &m_platformService;
  ContentHasher m_hasher;
  DiscInspector m_discInspector;
};

} // namespace firelight::library
