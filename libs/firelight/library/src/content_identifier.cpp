#include <firelight/library/content_identifier.hpp>

#include <platform_metadata.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>

namespace firelight::library {

std::string ContentIdentifier::suffixOf(const std::string &name) {
  const auto dot = name.find_last_of('.');
  if (dot == std::string::npos) {
    return {};
  }
  std::string ext = name.substr(dot + 1);
  std::transform(ext.begin(), ext.end(), ext.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return ext;
}

IdentifiedContent ContentIdentifier::identify(const std::string &path) const {
  IdentifiedContent content;
  const std::string suffix = suffixOf(path);

  // Disc images share extensions across many consoles, so the platform can only
  // be determined from the contents (delegated to DiscInspector).
  if (PlatformMetadata::isPossibleDiscExtension(suffix)) {
    content.isDisc = true;
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    content.fileSizeBytes = ec ? 0 : static_cast<size_t>(size);

    const DiscIdentity identity =
        m_discInspector.inspectFile(path, content.discMembers);
    content.valid = identity.valid;
    content.platformId = identity.platformId;
    content.contentHash = identity.contentHash;
    return content;
  }

  content.platformId = PlatformMetadata::getIdFromFileExtension(suffix);
  if (content.platformId == PlatformMetadata::PLATFORM_ID_UNKNOWN) {
    return content;
  }

  std::ifstream file(path, std::ios::binary);
  if (!file) {
    return content;
  }
  const std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
  file.close();

  content.fileSizeBytes = bytes.size();
  content.fileMd5 = ContentHasher::md5(bytes.data(), bytes.size());
  content.contentHash = m_hasher.hash(content.platformId, bytes).contentHash;
  content.valid = !content.contentHash.empty();
  return content;
}

IdentifiedContent ContentIdentifier::identifyInArchive(
    const std::string &entryName, const std::vector<uint8_t> &data,
    const size_t sizeBytes, const std::string &archivePath) const {
  IdentifiedContent content;
  const std::string suffix = suffixOf(entryName);

  if (PlatformMetadata::isPossibleDiscExtension(suffix)) {
    content.isDisc = true;
    content.fileSizeBytes = sizeBytes;

    const DiscIdentity identity =
        archivePath.empty()
            ? m_discInspector.inspectFile(entryName, content.discMembers)
            : m_discInspector.inspectArchiveEntry(archivePath, entryName,
                                                  content.discMembers);
    content.valid = identity.valid;
    content.platformId = identity.platformId;
    content.contentHash = identity.contentHash;
    return content;
  }

  content.platformId = PlatformMetadata::getIdFromFileExtension(suffix);

  content.fileSizeBytes = data.size();
  content.fileMd5 = ContentHasher::md5(data.data(), data.size());
  content.contentHash = m_hasher.hash(content.platformId, data).contentHash;
  content.valid = !content.contentHash.empty();
  return content;
}

} // namespace firelight::library
