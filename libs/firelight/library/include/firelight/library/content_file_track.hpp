// TODO: NEEDS REVIEW
#pragma once

#include <string>

namespace firelight::library {
// TODO
/**
 * A raw track a sheet names, reached through the sheet rather than opened on its own.
 *
 * The sheet is the ContentFile identified by m_contentFileId; the tracks capture the structure
 * that would otherwise be lost when a sheet is catalogued alone
 */
struct ContentFileTrack {
  int m_id = -1;
  int m_contentFileId = -1;
  std::string m_path; // path on disk, or entry name within an archive
  int m_sortIndex = 0;
};

} // namespace firelight::library
