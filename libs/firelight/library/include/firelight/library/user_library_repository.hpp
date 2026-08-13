#pragma once

#include <firelight/library/content_directory.hpp>
#include <firelight/library/content_file.hpp>
#include <firelight/library/disc_member.hpp>
#include <firelight/library/disc_set.hpp>
#include <firelight/library/entry.hpp>
#include <firelight/library/folder_entry_info.hpp>
#include <firelight/library/folder_info.hpp>
#include <firelight/library/game_identity.hpp>
#include <firelight/library/patch_file.hpp>
#include <firelight/library/run_configuration.hpp>
#include <firelight/library/scan_drop.hpp>
#include <firelight/library/tag.hpp>
#include <firelight/library/variant_group.hpp>

#include <optional>
#include <set>
#include <string_view>
#include <vector>

namespace firelight::library {
struct EntryCreatedEvent {
  int entryId;
};

struct EntryUpdatedEvent {
  int entryId;
};

// TODO
// An entry that is gone rather than changed. Folding a disc set destroys the entries it absorbs, and
// a row is only taken off the grid by an event naming the id that went
struct EntryDeletedEvent {
  int entryId;
};

// TODO
// Published when a group's title, pin, or the entry standing for it changes, and when the
// group is dissolved. The id outlives the group, so a listener must tolerate it being gone
struct VariantGroupUpdatedEvent {
  int groupId;
};

// TODO
// Published when one entry is folded into another because they turned out to be the same
// game. Anything keyed on the content hash has to follow, and the library cannot do that
// itself because saves and activity live in other modules
struct EntryAbsorbedEvent {
  int survivingEntryId;
  std::string absorbedContentHash;
  std::string survivingContentHash;
};

class IUserLibraryRepository {
public:
  virtual ~IUserLibraryRepository() = default;

  virtual bool create(ContentFile &contentFile) = 0;

  virtual void create(PatchFile &file) = 0;

  virtual bool create(FolderInfo &folder) = 0;

  virtual bool create(FolderEntryInfo &folderEntry) = 0;

  virtual bool create(ContentDirectory &directory) = 0;

  virtual bool create(DiscMember &member) = 0;

  virtual bool createEntry(Entry &entry) = 0;

  /**
   * Records a way to launch a content file. The type is what tells one disc's own way in from
   * the playlist that stands for the whole set
   */
  virtual void createRunConfiguration(int contentFileId, const std::string &path, int platformId,
                                      const std::string &contentHash,
                                      std::string_view type = RunConfiguration::TYPE_ROM) = 0;

  /**
   * Records the way a whole disc set launches, replacing whichever one it had.
   *
   * The anchor is the disc the set reads its identity from; it moves when a lower-numbered disc
   * turns up, so this replaces rather than adds
   */
  virtual void createRunConfigurationForSet(int setId, int anchorContentFileId, const std::string &contentHash) = 0;

  /**
   * Takes away a set's way in
   *
   * @return True when a row went
   */
  virtual bool deleteRunConfigurationsForDiscSet(int setId) = 0;

  virtual bool update(FolderInfo &folder) = 0;

  virtual bool update(Entry &entry) = 0;

  // TODO
  /**
   * Writes whether an entry is hidden.
   *
   * A field of its own rather than part of update(Entry&), which writes a whole row from a copy the
   * caller read earlier and would carry a stale flag back over one written since
   */
  virtual bool setEntryHidden(int entryId, bool hidden) = 0;

  virtual bool update(const ContentDirectory &directory) = 0;

  /**
   * Writes an entry's art projections. The descriptive document is not written here — see
   * applyEntryMetadata
   */
  virtual bool updateEntryMetadata(const Entry &entry) = 0;

  /**
   * Merges metadata into an entry's stored document.
   *
   * Fields the user pinned are skipped unless isUserEdit says this change is theirs, in which case
   * every field in changedFields is written and pinned. The read, merge and write happen in one
   * locked step, so a scrape on a worker cannot lose an edit made while it was running
   *
   * @param changedFields The metadata_fields names incoming carries a value for
   */
  virtual bool applyEntryMetadata(int entryId, const GameMetadata &incoming, const std::set<std::string> &changedFields,
                                  bool isUserEdit) = 0;

  /**
   * Records that art was looked up for an entry, whether or not any was found.
   *
   * Stamped even on an empty result, so a game the provider has nothing for is not
   * asked about again on every startup
   */
  virtual bool markArtFetched(int entryId, uint64_t whenMillis) = 0;

  /**
   * The ids of visible entries art has never been looked up for, oldest first.
   *
   * A query rather than a filter over getEntries, because the caller wants a handful at a time out of
   * a library of thousands
   */
  virtual std::vector<int> getEntryIdsMissingArt(int limit) = 0;

  //****************
  // what the scan could not catalogue
  //****************

  /**
   * Records a file the scanner accepted and could not catalogue, or refreshes the one already
   * there
   *
   * @return True when this path was not already recorded
   */
  virtual bool recordScanDrop(const ScanDrop &drop) = 0;

  /**
   * Forgets a recorded drop, for a path that has since identified
   */
  virtual bool clearScanDrop(const std::string &filePath, const std::string &archivePath) = 0;

  /**
   * Every file the scan could not catalogue, oldest first
   */
  [[nodiscard]] virtual std::vector<ScanDrop> getScanDrops() = 0;

  /**
   * Counts one more file carrying an extension nothing accepts. The path is deliberately not
   * kept — this answers which formats people own, not which files
   */
  virtual void countUnrecognizedExtension(const std::string &extension) = 0;

  /**
   * The extensions seen on disk that nothing accepts, most common first
   */
  [[nodiscard]] virtual std::vector<UnrecognizedExtension> getUnrecognizedExtensions() = 0;

  //****************
  // disc sets
  //****************

  /**
   * Creates a multi-disc game, setting the id on the passed set
   */
  virtual bool createDiscSet(DiscSet &set) = 0;

  virtual bool updateDiscSet(const DiscSet &set) = 0;

  virtual bool deleteDiscSet(int setId) = 0;

  [[nodiscard]] virtual std::optional<DiscSet> getDiscSet(int setId) = 0;

  [[nodiscard]] virtual std::vector<DiscSet> getDiscSets() = 0;

  /**
   * The set holding a given disc, found from the content file rather than from an entry
   */
  [[nodiscard]] virtual std::optional<DiscSet> getDiscSetForContentFile(int contentFileId) = 0;

  /**
   * Every disc of a set, ordered by disc number
   */
  [[nodiscard]] virtual std::vector<ContentFile> getDiscsInSet(int setId) = 0;

  /**
   * The entries pointed at a set. An absorbed disc has no entry, so a set's discs cannot be
   * walked back to find the one standing for it
   */
  [[nodiscard]] virtual std::vector<Entry> getEntriesInDiscSet(int setId) = 0;

  /**
   * Puts a content file in a set, or takes it out when the set is empty
   */
  virtual bool setContentFileDiscSet(int contentFileId, std::optional<int> setId) = 0;

  /**
   * Points an entry at a set. isUserChoice records that a person decided it, which is what
   * stops automatic grouping undoing them
   */
  virtual bool setEntryDiscSet(int entryId, std::optional<int> setId, bool isUserChoice) = 0;

  /**
   * Which disc a save slot was last on, or nothing when it has never been launched
   */
  [[nodiscard]] virtual std::optional<int> getLastDisc(int entryId, int saveSlot) = 0;

  virtual bool setLastDisc(int entryId, int saveSlot, int discNumber) = 0;

  //****************
  // variant groups
  //****************

  virtual bool createVariantGroup(VariantGroup &group) = 0;

  virtual bool updateVariantGroup(const VariantGroup &group) = 0;

  /**
   * Removes a group and clears it from every entry that was in it
   */
  virtual bool deleteVariantGroup(int groupId) = 0;

  virtual std::vector<VariantGroup> getVariantGroups() = 0;

  virtual std::optional<VariantGroup> getVariantGroup(int groupId) = 0;

  /**
   * Puts an entry in a group, or takes it out of whichever one it was in
   */
  virtual bool setEntryVariantGroup(int entryId, std::optional<int> groupId, bool isUserChoice) = 0;

  /**
   * The entries on a platform that could be the same game as this one.
   *
   * A net, not an answer. An id and a title are each only sometimes known, and SQL cannot
   * cheaply say that an unknown matches anything, so this returns everything either could
   * reach and areSameGame decides which of them count
   *
   * Ascending and without repeats, so a caller that stops at the first peer stops at the same
   * one every time
   */
  virtual std::vector<int> getCandidateEntryIds(const GameIdentity &identity) = 0;

  virtual std::vector<Entry> getEntriesInVariantGroup(int groupId) = 0;

  //****************
  // tags
  //****************

  /**
   * Creates a tag, or fills in the existing one when the name is already taken. Names collide case
   * insensitively, so "sci-fi" and "Sci-Fi" are the same tag
   */
  virtual bool createTag(Tag &tag) = 0;

  virtual bool renameTag(int tagId, const std::string &name) = 0;

  /**
   * Moves every entry carrying sourceTagId onto targetTagId and removes the source. An entry that
   * carried both keeps one
   */
  virtual bool mergeTags(int sourceTagId, int targetTagId) = 0;

  /**
   * Removes a tag and takes it off every entry carrying it
   */
  virtual bool deleteTag(int tagId) = 0;

  /**
   * Every tag, with how many entries carry each
   */
  virtual std::vector<Tag> getTags() = 0;

  /**
   * Replaces an entry's tags with exactly the ones given
   */
  virtual bool setEntryTags(int entryId, const std::vector<int> &tagIds) = 0;

  virtual std::vector<FolderInfo> listFolders() = 0;

  virtual bool deleteFolder(int folderId) = 0;

  virtual bool deleteFolderEntry(FolderEntryInfo &info) = 0;

  virtual bool reorderFolders(int parentId, const std::vector<int> &orderedFolderIds) = 0;

  // Moves a folder under a new parent (-1 = root), appended at the end
  virtual bool setFolderParent(int folderId, int newParentId) = 0;

  virtual bool deleteContentDirectory(int id) = 0;

  virtual std::optional<ContentFile> getContentFileWithPathAndSize(const std::string &filePath, size_t fileSizeBytes,
                                                                   bool inArchive) = 0;

  virtual std::vector<ContentFile> getContentFiles() = 0;

  virtual std::optional<ContentFile> getContentFile(int id) = 0;

  /**
   * Every catalogued file holding the same content, which for a disc set member is its disc
   */
  [[nodiscard]] virtual std::vector<ContentFile> getContentFilesWithContentHash(const std::string &contentHash) = 0;

  /**
   * The catalogued file at a path, whatever size it is now. A file we rewrite changes size,
   * which is what the path-and-size lookup keys on
   */
  [[nodiscard]] virtual std::optional<ContentFile> getContentFileWithPath(const std::string &filePath) = 0;

  /**
   * Re-stamps what a rewrite changed. The row keeps its id, so the ways to launch it survive
   */
  virtual bool setContentFileIdentity(int contentFileId, const std::string &contentHash, size_t fileSizeBytes) = 0;

  // TODO
  /**
   * Records that a catalogued file is no longer on disk, keeping the row.
   *
   * The ways to launch it are left alone, so the patch a run configuration names survives the file
   * going away and coming back
   */
  // TODO
  /**
   * Every catalogued file whose bytes are on disk right now.
   *
   * The plain listing answers what the library knows about, which includes what has gone missing;
   * this answers what can be opened
   */
  [[nodiscard]] virtual std::vector<ContentFile> getPresentContentFiles() = 0;

  // TODO
  /**
   * The discs of a set whose bytes are on disk right now.
   *
   * Membership outlives a disc going away, so counting what is there and writing the playlist ask
   * this while anything deciding what the set is asks for every member
   */
  [[nodiscard]] virtual std::vector<ContentFile> getPresentDiscsInSet(int setId) = 0;

  virtual bool markContentFileMissing(int id) = 0;

  // TODO
  /**
   * Records that a file marked missing is back
   */
  virtual bool reviveContentFile(int id) = 0;

  // TODO
  /**
   * Forgets a content file outright, along with its ways in and its disc members. This is the
   * purge; a file that merely went away is marked missing instead
   */
  virtual bool deleteContentFile(int id) = 0;

  virtual std::optional<PatchFile> getPatchFile(int id) = 0;

  virtual std::vector<Entry> getEntries(int offset, int limit) = 0;

  virtual std::optional<Entry> getEntry(int entryId) = 0;

  /**
   * Removes an entry and everything hanging off it in this database. Saves and playtime are
   * keyed on the content hash and are not touched
   */
  virtual bool deleteEntry(int entryId) = 0;

  virtual std::optional<Entry> getEntryWithContentHash(const std::string &contentHash) = 0;

  virtual std::vector<RunConfiguration> getRunConfigurations(const std::string &contentHash) = 0;

  /**
   * Removes the ways to launch one file, leaving the file itself catalogued
   */
  /**
   * Takes away the ways in a file has of its own, leaving any belonging to a disc set.
   *
   * A set anchors its way in on one of its discs, so removing everything pointed at that disc
   * would take the whole set's way in with it
   */
  virtual bool deleteRunConfigurationsForContentFile(int contentFileId) = 0;

  virtual std::vector<ContentDirectory> getContentDirectories() = 0;
};
} // namespace firelight::library
