// TODO: NEEDS REVIEW
#pragma once

#include <optional>
#include <string>

namespace firelight::library {

// TODO
/**
 * Who decided a disc belongs where, worst-informed first. A stronger source may replace a weaker
 * row; a weaker one never touches a stronger
 */
enum class DiscSource {
  Filename = 1,     // The disc number parsed out of the file's own name
  Database = 2,     // The metadata database, answering per dump
  PlaylistFile = 3, // A playlist the user wrote, which names the discs and their order
  User = 4,         // A person said so
};

// TODO
/**
 * One disc of a set: which number it sits at, and the file that fills it.
 *
 * The file is optional because a playlist can name a disc that has not been catalogued yet. Such a
 * row remembers the position that disc will take when its file turns up, and carries no hash until
 * then, which is why it can never be the set's anchor
 */
struct DiscSetMember {
  int m_id = -1;
  int m_discSetId = -1;
  int m_discNumber = 0;

  std::optional<int> m_contentFileId{};

  // What the row names, and how a pending row finds its file later
  std::string m_memberPath;

  DiscSource m_source = DiscSource::Filename;

  // Which playlist said so, when the source is PlaylistFile
  std::string m_sourcePath;

  // Placement matched more than one candidate set and picked one
  bool m_isUncertain = false;
};

} // namespace firelight::library
