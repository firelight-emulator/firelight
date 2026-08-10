#include <firelight/library/content_extensions.hpp>
#include <firelight/library/content_identifier.hpp>
#include <firelight/library/file_bytes.hpp>
#include <firelight/platforms/platform_service.hpp>
#include <firelight/util/strings.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace firelight::library {

ContentIdentifier::ContentIdentifier(platforms::IPlatformService &platformService)
    : m_platformService(platformService), m_discInspector(platformService) {}

std::string ContentIdentifier::suffixOf(const std::string &name) {
  const auto dot = name.find_last_of('.');
  if (dot == std::string::npos) {
    return {};
  }
  return strings::toLower(name.substr(dot + 1));
}

IdentifiedContent ContentIdentifier::identify(const std::string &path) const {
  IdentifiedContent content;
  const std::string suffix = suffixOf(path);

  // Disc images share extensions across many consoles, so the platform can only
  // be determined from the contents (delegated to DiscInspector)
  if (firelight::library::isDiscExtension(suffix)) {
    content.isDisc = true;
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    content.fileSizeBytes = ec ? 0 : static_cast<size_t>(size);

    const DiscIdentity identity = m_discInspector.inspectFile(path, content.discMembers);
    content.valid = identity.valid;
    content.platformId = identity.platformId;
    content.contentHash = identity.contentHash;
    return content;
  }

  content.platformId = m_platformService.platformIdForExtension(suffix);
  if (content.platformId == firelight::platforms::PlatformService::PLATFORM_ID_UNKNOWN) {
    return content;
  }

  const std::vector<uint8_t> bytes = readAllBytes(path);
  if (bytes.empty()) {
    return content;
  }

  content.fileSizeBytes = bytes.size();
  content.fileMd5 = ContentHasher::md5(bytes.data(), bytes.size());
  content.contentHash = m_hasher.hash(content.platformId, bytes).contentHash;
  content.valid = !content.contentHash.empty();
  return content;
}

IdentifiedContent ContentIdentifier::identifyInArchive(const std::string &entryName, const std::vector<uint8_t> &data,
                                                       const size_t sizeBytes, const std::string &archivePath) const {
  IdentifiedContent content;
  const std::string suffix = suffixOf(entryName);

  if (firelight::library::isDiscExtension(suffix)) {
    content.isDisc = true;
    content.fileSizeBytes = sizeBytes;

    const DiscIdentity identity =
        archivePath.empty() ? m_discInspector.inspectFile(entryName, content.discMembers)
                            : m_discInspector.inspectArchiveEntry(archivePath, entryName, content.discMembers);
    content.valid = identity.valid;
    content.platformId = identity.platformId;
    content.contentHash = identity.contentHash;
    return content;
  }

  content.platformId = m_platformService.platformIdForExtension(suffix);

  content.fileSizeBytes = data.size();
  content.fileMd5 = ContentHasher::md5(data.data(), data.size());
  content.contentHash = m_hasher.hash(content.platformId, data).contentHash;
  content.valid = !content.contentHash.empty();
  return content;
}

} // namespace firelight::library
