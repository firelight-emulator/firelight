#include <firelight/library/content_loader.hpp>

#include <firelight/library/archive_reader.hpp>
#include <firelight/library/content_hasher.hpp>

#include <spdlog/spdlog.h>

#include <fstream>
#include <iterator>

namespace firelight::library {

LoadedContent ContentLoader::load(const ContentFile &info) const {
  // Disc images are opened by the core directly from their file path (the
  // libretro core sets need_fullpath). We don't read the (potentially multi-GB)
  // image into memory, and we reuse the canonical hash computed at scan time --
  // the buffer-based hash used for cartridges does not work for disc systems.
  if (info.m_type == ContentType::Disc) {
    LoadedContent content;
    content.contentHash = info.m_contentHash;
    content.valid = !content.contentHash.empty();
    return content;
  }

  std::vector<uint8_t> rawBytes;
  if (info.m_inArchive) {
    rawBytes = ArchiveReader(info.m_archivePathName).readEntryByPath(info.m_filePath);
  } else {
    std::ifstream file(info.m_filePath, std::ios::binary);
    if (!file) {
      spdlog::error("[ContentLoader] Could not open content file: {}",
                    info.m_filePath);
      return {};
    }
    rawBytes.assign(std::istreambuf_iterator<char>(file),
                    std::istreambuf_iterator<char>());
    file.close();
  }

  const HashedContent hashed = ContentHasher{}.hash(info.m_platformId, rawBytes);

  LoadedContent content;
  content.contentBytes = hashed.contentBytes;
  content.contentHash = hashed.contentHash;
  content.valid = !hashed.contentHash.empty();
  return content;
}

void ContentLoader::applyPatch(LoadedContent &content, const int platformId,
                               const PatchFile &patch) const {
  if (content.contentBytes.empty()) {
    spdlog::warn("[ContentLoader] No content bytes to apply patch to");
    return;
  }

  const std::vector<uint8_t> patched = patch.patch(content.contentBytes);
  const HashedContent hashed = ContentHasher{}.hash(platformId, patched);
  content.contentBytes = hashed.contentBytes;
  content.contentHash = hashed.contentHash;
  content.valid = !hashed.contentHash.empty();
}

} // namespace firelight::library
