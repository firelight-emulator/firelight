#include <firelight/library/content_loader.hpp>

#include <firelight/library/archive_reader.hpp>
#include <firelight/library/content_hasher.hpp>
#include <firelight/library/file_bytes.hpp>

#include <spdlog/spdlog.h>

namespace firelight::library {

LoadedContent ContentLoader::load(const ContentFile &info) const {
  // Disc images are opened by the core directly from their file path (the
  // libretro core sets need_fullpath). We don't read the (potentially multi-GB)
  // image into memory, and we reuse the canonical hash computed at scan time --
  // the buffer-based hash used for cartridges does not work for disc systems
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
    rawBytes = readAllBytes(info.m_filePath);
    if (rawBytes.empty()) {
      spdlog::error("[ContentLoader] Could not read content file: {}",
                    info.m_filePath);
      return {};
    }
  }

  HashedContent hashed = ContentHasher{}.hash(info.m_platformId, rawBytes);

  LoadedContent content;
  content.contentHash = hashed.contentHash;
  content.contentBytes = std::move(hashed.contentBytes);
  content.valid = !content.contentHash.empty();
  return content;
}

void ContentLoader::applyPatch(LoadedContent &content, const int platformId,
                               const PatchFile &patch) const {
  if (content.contentBytes.empty()) {
    spdlog::warn("[ContentLoader] No content bytes to apply patch to");
    return;
  }

  const std::vector<uint8_t> patched = patch.patch(content.contentBytes);
  HashedContent hashed = ContentHasher{}.hash(platformId, patched);
  content.contentHash = hashed.contentHash;
  content.contentBytes = std::move(hashed.contentBytes);
  content.valid = !content.contentHash.empty();
}

} // namespace firelight::library
