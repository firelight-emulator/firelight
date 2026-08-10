#include "firelight/library/chd_cdreader.hpp"

#include <firelight/util/strings.hpp>

#include <cstdio>
#include <cstring>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <libchdr/cdrom.h>
#include <libchdr/chd.h>
#include <rcheevos/rc_hash.h>
#include <spdlog/spdlog.h>

namespace firelight::library {

namespace {
// One track as the CHD describes it. The two sector numbers differ: a disc numbers its
// sectors continuously, while a CHD pads every track up to a multiple of four frames
struct ChdTrack {
  uint32_t discFirstSector = 0;
  uint32_t chdFirstFrame = 0;
  uint32_t frames = 0;
  uint32_t pregap = 0;
  uint32_t sectorSize = CD_MAX_SECTOR_DATA;
  uint32_t headerSize = 0;
  uint32_t dataSize = CD_MAX_SECTOR_DATA;
  bool isData = false;
};

struct ChdHandle {
  chd_file *file = nullptr;
  uint32_t hunkBytes = 0;
  std::vector<uint8_t> hunk;
  uint32_t cachedHunk = UINT32_MAX;
  ChdTrack track;
};

// The reader rcheevos came with. Anything that is not a CHD goes back to it
rc_hash_cdreader DEFAULT_READER{};

// Handles this reader opened. read_sector and friends are called with handles from both
// readers, and the other reader's handle is opaque, so ownership is tracked rather than probed
std::set<const void *> OWNED_HANDLES;
std::mutex OWNED_MUTEX;

bool isOurs(const void *handle) {
  std::lock_guard lock(OWNED_MUTEX);
  return OWNED_HANDLES.count(handle) > 0;
}

// What one TYPE field in the track metadata means for reading a sector
void applyTrackType(const std::string &type, ChdTrack &track) {
  track.isData = type != "AUDIO";

  if (type == "MODE1" || type == "MODE2_FORM1") {
    track.sectorSize = 2048;
    track.headerSize = 0;
    track.dataSize = 2048;
    return;
  }

  if (type == "MODE2_FORM2") {
    track.sectorSize = 2324;
    track.headerSize = 0;
    track.dataSize = 2324;
    return;
  }

  if (type == "MODE1_RAW") {
    track.headerSize = 16;
    track.dataSize = 2048;
    return;
  }

  if (type == "MODE2_RAW" || type == "MODE2_FORM_MIX") {
    track.headerSize = 24;
    track.dataSize = 2048;
    return;
  }

  if (type == "MODE2") {
    track.sectorSize = 2336;
    track.headerSize = 8;
    track.dataSize = 2048;
    return;
  }

  // AUDIO, and anything unrecognized, is read whole
  track.headerSize = 0;
  track.dataSize = CD_MAX_SECTOR_DATA;
}

// Reads the CD track table. Every track is stored at a four-frame boundary, so a track's
// position in the file has to be accumulated separately from its position on the disc
bool readTracks(chd_file *file, std::vector<ChdTrack> &tracks) {
  auto discSector = uint32_t{0};
  auto chdFrame = uint32_t{0};

  for (auto index = uint32_t{0}; index < 99; ++index) {
    char metadata[512] = {};
    uint32_t length = 0;
    auto number = 0;
    auto frames = 0;
    auto pregap = 0;
    auto postgap = 0;
    char type[64] = {};
    char subtype[64] = {};
    char pgType[64] = {};
    char pgSub[64] = {};

    if (chd_get_metadata(file, CDROM_TRACK_METADATA2_TAG, index, metadata, sizeof(metadata) - 1, &length, nullptr,
                         nullptr) == CHDERR_NONE) {
      if (std::sscanf(metadata, CDROM_TRACK_METADATA2_FORMAT, &number, type, subtype, &frames, &pregap, pgType, pgSub,
                      &postgap) != 8) {
        break;
      }
    } else if (chd_get_metadata(file, CDROM_TRACK_METADATA_TAG, index, metadata, sizeof(metadata) - 1, &length, nullptr,
                                nullptr) == CHDERR_NONE) {
      if (std::sscanf(metadata, CDROM_TRACK_METADATA_FORMAT, &number, type, subtype, &frames) != 4) {
        break;
      }
    } else {
      break;
    }

    ChdTrack track;
    track.frames = static_cast<uint32_t>(frames);
    track.pregap = static_cast<uint32_t>(pregap);
    track.discFirstSector = discSector;
    track.chdFirstFrame = chdFrame;
    applyTrackType(type, track);
    tracks.push_back(track);

    discSector += track.frames;
    chdFrame += (track.frames + CD_TRACK_PADDING - 1) / CD_TRACK_PADDING * CD_TRACK_PADDING;
  }

  return !tracks.empty();
}

void closeTrack(void *handle) {
  auto *chd = static_cast<ChdHandle *>(handle);

  if (chd == nullptr) {
    return;
  }

  if (!isOurs(handle)) {
    if (DEFAULT_READER.close_track != nullptr) {
      DEFAULT_READER.close_track(handle);
    }

    return;
  }

  {
    std::lock_guard lock(OWNED_MUTEX);
    OWNED_HANDLES.erase(handle);
  }

  if (chd->file != nullptr) {
    chd_close(chd->file);
  }

  delete chd;
}

uint32_t firstTrackSector(void *handle) {
  if (!isOurs(handle)) {
    return DEFAULT_READER.first_track_sector != nullptr ? DEFAULT_READER.first_track_sector(handle) : 0;
  }

  const auto *chd = static_cast<const ChdHandle *>(handle);
  return chd->track.discFirstSector + chd->track.pregap;
}

// Hands back the user data of consecutive sectors, the same shape the bundled reader
// produces: the sync header and any error correction are skipped
size_t readSector(void *handle, uint32_t sector, void *buffer, size_t requestedBytes) {
  if (!isOurs(handle)) {
    return DEFAULT_READER.read_sector != nullptr ? DEFAULT_READER.read_sector(handle, sector, buffer, requestedBytes)
                                                 : 0;
  }

  auto *chd = static_cast<ChdHandle *>(handle);
  const auto &track = chd->track;

  if (sector < track.discFirstSector) {
    return 0;
  }

  auto *out = static_cast<uint8_t *>(buffer);
  auto written = size_t{0};
  auto offsetInTrack = sector - track.discFirstSector;

  while (requestedBytes > 0 && offsetInTrack < track.frames) {
    const auto frame = track.chdFirstFrame + offsetInTrack;
    const auto hunkIndex = frame / CD_FRAMES_PER_HUNK;

    if (hunkIndex != chd->cachedHunk) {
      if (chd_read(chd->file, hunkIndex, chd->hunk.data()) != CHDERR_NONE) {
        return written;
      }

      chd->cachedHunk = hunkIndex;
    }

    const auto frameOffset = (frame % CD_FRAMES_PER_HUNK) * CD_FRAME_SIZE;
    const auto take = std::min(static_cast<size_t>(track.dataSize), requestedBytes);

    if (frameOffset + track.headerSize + take > chd->hunk.size()) {
      return written;
    }

    std::memcpy(out, chd->hunk.data() + frameOffset + track.headerSize, take);
    out += take;
    written += take;
    requestedBytes -= take;
    ++offsetInTrack;
  }

  return written;
}

void *openTrackIterator(const char *path, uint32_t track, const rc_hash_iterator_t *iterator) {
  if (path == nullptr || !strings::endsWithIgnoringCase(path, ".chd")) {
    return DEFAULT_READER.open_track_iterator != nullptr ? DEFAULT_READER.open_track_iterator(path, track, iterator)
                                                         : nullptr;
  }

  chd_file *file = nullptr;

  if (chd_open(path, CHD_OPEN_READ, nullptr, &file) != CHDERR_NONE) {
    spdlog::debug("Not a readable CHD: {}", path);
    return nullptr;
  }

  std::vector<ChdTrack> tracks;

  if (!readTracks(file, tracks)) {
    spdlog::debug("CHD has no CD track table: {}", path);
    chd_close(file);
    return nullptr;
  }

  // rcheevos asks for a track by number, for the largest, or for the first one holding data
  const ChdTrack *chosen = nullptr;

  if (track == RC_HASH_CDTRACK_FIRST_DATA) {
    for (const auto &candidate : tracks) {
      if (candidate.isData) {
        chosen = &candidate;
        break;
      }
    }
  } else if (track == RC_HASH_CDTRACK_LARGEST) {
    for (const auto &candidate : tracks) {
      if (candidate.isData && (chosen == nullptr || candidate.frames > chosen->frames)) {
        chosen = &candidate;
      }
    }
  } else if (track == RC_HASH_CDTRACK_LAST) {
    chosen = &tracks.back();
  } else if (track >= 1 && track <= tracks.size()) {
    chosen = &tracks[track - 1];
  }

  if (chosen == nullptr) {
    chd_close(file);
    return nullptr;
  }

  const auto *header = chd_get_header(file);

  if (header == nullptr || header->hunkbytes == 0) {
    chd_close(file);
    return nullptr;
  }

  auto *handle = new ChdHandle();
  handle->file = file;
  handle->hunkBytes = header->hunkbytes;
  handle->hunk.resize(header->hunkbytes);
  handle->track = *chosen;

  {
    std::lock_guard lock(OWNED_MUTEX);
    OWNED_HANDLES.insert(handle);
  }

  return handle;
}

void *openTrack(const char *path, uint32_t track) { return openTrackIterator(path, track, nullptr); }
} // namespace

void installChdCdReader() {
  static std::once_flag once;

  std::call_once(once, [] {
    rc_hash_get_default_cdreader(&DEFAULT_READER);

    rc_hash_cdreader reader{};
    reader.open_track = openTrack;
    reader.open_track_iterator = openTrackIterator;
    reader.read_sector = readSector;
    reader.close_track = closeTrack;
    reader.first_track_sector = firstTrackSector;

    rc_hash_init_custom_cdreader(&reader);
  });
}

} // namespace firelight::library
