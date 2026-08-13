#pragma once

#include <firelight/library/metadata_overrides.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace firelight::library {
struct Entry {
  int id = -1;
  std::string displayName;
  bool nameUserSet = false; // So we don't overwrite a user-edited name with metadata
  std::string contentHash;
  unsigned platformId = 0;
  unsigned activeSaveSlot = 1;
  // TODO
  // Whether the user put this away. Nothing derives it; a game whose files are gone stays on the
  // shelf and says so
  bool hidden = false;
  // TODO
  // Whether any copy of this content is still on disk. Derived when the entry is read and never
  // stored. An entry with nothing catalogued has nothing to launch, so it is not available
  bool isContentAvailable = false;
  bool favorite = false;
  unsigned rating = 0; // user's 1-5 star rating; 0 = unrated
  std::string icon1x1SourceUrl;
  std::string boxartFrontSourceUrl;
  std::string boxartBackSourceUrl;

  // TODO
  // Everything descriptive, stored as one document. The name is not here: that is
  // user state, and a scrape guards it with nameUserSet instead
  GameMetadata metadata{};

  // TODO
  // Which of the document's fields the user edited, so a scrape leaves them alone
  MetadataOverrides metadataOverrides{};

  // TODO
  // The derived title folded to the form other releases of the same game share. Indexed,
  // and not the display name: a rename must not move an entry between groups
  std::string normalizedTitle;

  // TODO
  // Which disc of a set this is, 0 when it is not one. Read from the entry's content file
  // rather than stored again here
  int discNumber = 0;

  // TODO
  // The content database's id for the game this is a copy of, 0 when nothing has resolved one.
  // Read from the entry's content file, which outlives the entry when a set folds
  int gameId = 0;

  // TODO
  // The multi-disc game this entry is, when it is one
  std::optional<int> discSetId{};

  // TODO
  // Whether a person decided this entry's disc set. Automatic grouping leaves it alone
  bool discSetUserSet = false;

  // TODO
  // The set of releases this entry is one of
  std::optional<int> variantGroupId{};

  // TODO
  // Whether a person decided this entry's membership. Automatic grouping never touches an
  // entry that carries this, so taking one out of a group is not undone on the next scan
  bool variantGroupUserSet = false;

  // TODO
  // When art was last looked up. Empty means never tried, so configuring a provider
  // later still reaches entries a previous run skipped
  std::optional<uint64_t> artFetchedAt{};

  std::vector<int> folderIds{};

  // TODO
  // The tags the user put on this entry, joined from entry_tags
  std::vector<int> tagIds{};

  // The content directories that contain this entry's content file(s). Used for smart filtering and for
  // determining whether an entry is in any content directories
  std::vector<int> contentDirectoryIds{};
  std::vector<std::string> contentPaths{};

  // TODO
  // The subset of contentPaths whose file has not been marked missing. Whether one of these can be
  // opened is a separate question the folder answers, so both are needed to say what is wrong
  std::vector<std::string> readableContentPaths{};

  uint64_t createdAt = 0;
};
} // namespace firelight::library
