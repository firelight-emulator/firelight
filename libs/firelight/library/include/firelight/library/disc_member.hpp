#pragma once

#include <string>

namespace firelight::library {

// A member file of a multi-file disc set: a data/audio track referenced by a
// cue/gdi sheet, or a disc image listed in an .m3u playlist. The set's primary
// file is the ContentFile identified by m_contentFileId; members capture the
// structure that would otherwise be lost when a sheet is catalogued alone
struct DiscMember {
  int m_id = -1;
  int m_contentFileId = -1;
  std::string m_path; // path on disk, or entry name within an archive
  std::string m_role; // "track" for cue/gdi tracks, "disc" for m3u entries
  int m_sortIndex = 0;
};

} // namespace firelight::library
