#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace firelight::library {

// What kind of content a file holds. Cartridge content is a single self-
// contained ROM; Disc content may be a single image or the primary file (a
// cue/gdi/m3u sheet) of a multi-file disc set whose members live in disc_members
enum class ContentType {
  Cartridge = 0,
  Disc = 1,
};

// A managed content file: a ROM or disc image that has been identified and
// catalogued. The launchable identity is m_contentHash; m_filePath/m_inArchive
// locate the bytes on disk
struct ContentFile {
  int m_id = -1;
  ContentType m_type = ContentType::Cartridge;
  size_t m_fileSizeBytes = 0;
  std::string m_filePath;
  std::string m_fileMd5;
  std::string m_fileCrc32;
  bool m_inArchive = false;
  std::string m_archivePathName;
  int m_platformId = -1;
  std::string m_contentHash;
  // Which disc of a set this file is, 0 when it is not one. Parsed from the name at scan
  // time, so it is known before anything asynchronous runs
  int m_discNumber = 0;

  // TODO
  // Set when somebody said which disc this is by hand, so re-deriving leaves it alone
  bool m_discNumberUserSet = false;

  // TODO
  // Where this dump was released, empty when nothing has said. Ordered most authoritative
  // first. Held here rather than on the entry because folding a set deletes the entries of
  // every disc but one
  std::vector<std::string> m_regions;

  // TODO
  // The content database's id for the game this is a copy of, 0 when nothing has resolved
  // one. One id spans a game's regional releases, so it never stands alone as identity
  int m_gameId = 0;

  // The multi-disc game this file belongs to. This is the membership edge: the set is found
  // from its files, not from the entry
  std::optional<int> m_discSetId{};
  // The content directory (content_directoriesv1.id) this file was found under,
  // resolved by longest path-prefix match. -1 when it belongs to no known
  // directory (e.g. imported before source tracking, and unmatched by the
  // backfill). This is the folder source for smart folders
  int m_contentDirectoryId = -1;

  // TODO
  // When the file stopped being on disk, 0 while it is there. The row outlives the file so the
  // path, the hand-set disc number and the set membership survive it coming back
  int64_t m_missingSince = 0;
};

} // namespace firelight::library
