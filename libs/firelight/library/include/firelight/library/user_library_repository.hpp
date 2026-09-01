// TODO: NEEDS REVIEW
#pragma once

#include <firelight/library/content_directory.hpp>
#include <firelight/library/content_file.hpp>
#include <firelight/library/content_file_track.hpp>
#include <firelight/library/disc_set.hpp>
#include <firelight/library/disc_set_member.hpp>
#include <firelight/library/entry.hpp>
#include <firelight/library/folder_entry.hpp>
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

class IUserLibraryRepository {
public:
  virtual ~IUserLibraryRepository() = default;

  virtual bool create(ContentFile &contentFile) = 0;

  virtual void create(PatchFile &file) = 0;

  virtual bool create(FolderInfo &folder) = 0;

  virtual bool create(FolderEntry &folderEntry) = 0;

  virtual bool create(ContentDirectory &directory) = 0;

  virtual bool create(ContentFileTrack &track) = 0;

  virtual bool createEntry(Entry &entry) = 0;

  // TODO
  /**
   * Records a way to launch a content file. The first one an entry gets is the one it launches
   * through
   */
  virtual void createRunConfiguration(int contentFileId, const std::string &path, int platformId,
                                      const std::string &contentHash) = 0;

  /**
   * Records the way a whole disc set launches, replacing whichever one it had.
   *
   * The anchor is the disc the set reads its identity from; it moves when a lower-numbered disc
   * turns up, so this replaces rather than adds
   */
  virtual void createRunConfigurationForSet(int setId, int anchorContentFileId, const std::string &contentHash) = 0;

  virtual bool deleteRunConfigurationsForDiscSet(int setId) = 0;

  virtual bool update(FolderInfo &folder) = 0;

  virtual bool update(Entry &entry) = 0;

  virtual bool setEntryHidden(int entryId, bool hidden) = 0;

  // TODO
  /**
   * Points an entry at a different dump, which is what a set does when a lower-numbered disc
   * arrives and takes over as its anchor. Announces the move so everything keyed on the old hash
   * can follow
   */
  virtual bool setEntryContentHash(int entryId, const std::string &contentHash, std::string_view reason) = 0;

  virtual bool update(const ContentDirectory &directory) = 0;

  // TODO
  /**
   * Writes an entry's art projections. The descriptive document is not written here — see
   * applyEntryMetadata
   */
  virtual bool updateEntryMetadata(const Entry &entry) = 0;

  /**
   * Merges metadata into an entry's stored document
   *
   * Fields the user pinned are skipped unless isUserEdit says this change is theirs, in which case
   * every field in changedFields is written and pinned
   *
   * @param changedFields The metadata_fields names incoming carries a value for
   */
  virtual bool applyEntryMetadata(int entryId, const GameMetadata &incoming, const std::set<std::string> &changedFields,
                                  bool isUserEdit) = 0;

  /**
   * Records that art was looked up for an entry, regardless of whether any was found.
   *
   * Stamped even on an empty result, so a game the provider has nothing for is not
   * asked about again on every startup
   */
  virtual bool markArtFetched(int entryId, uint64_t whenMillis) = 0;

  /**
   * The ids of visible entries art has never been looked up for, oldest first
   */
  virtual std::vector<int> getEntryIdsMissingArt(int limit) = 0;

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
   * Counts one more file carrying an extension nothing accepts
   */
  virtual void countUnrecognizedExtension(const std::string &extension) = 0;

  /**
   * The extensions seen on disk that nothing accepts, most common first
   */
  [[nodiscard]] virtual std::vector<UnrecognizedExtension> getUnrecognizedExtensions() = 0;

  /**
   * Creates a multi-disc game, setting the id on the passed set
   */
  virtual bool createDiscSet(DiscSet &set) = 0;

  virtual bool updateDiscSet(const DiscSet &set) = 0;

  virtual bool deleteDiscSet(int setId) = 0;

  [[nodiscard]] virtual std::optional<DiscSet> getDiscSet(int setId) = 0;

  // TODO
  /**
   * The sets a disc carrying this identity could belong to, oldest first so a disc matching more
   * than one joins the same one every time
   */
  [[nodiscard]] virtual std::vector<DiscSet> getCandidateDiscSets(const GameIdentity &identity) = 0;

  [[nodiscard]] virtual std::vector<DiscSet> getDiscSets() = 0;

  /**
   * The set holding a given disc, found from the content file rather than from an entry
   */
  [[nodiscard]] virtual std::optional<DiscSet> getDiscSetForContentFile(int contentFileId) = 0;

  /**
   * Every disc of a set, ordered by disc number
   */
  [[nodiscard]] virtual std::vector<ContentFile> getDiscsInSet(int setId) = 0;

  // TODO
  /**
   * Records which disc of a set a file is, or the position a disc will take once its file is
   * catalogued. Replaces the row already at that path in the same set
   */
  virtual bool create(DiscSetMember &member) = 0;

  // TODO
  /**
   * A set's membership, ordered by disc number. Includes rows whose file is not catalogued yet
   */
  [[nodiscard]] virtual std::vector<DiscSetMember> getDiscSetMembers(int setId) = 0;

  // TODO
  /**
   * Takes a disc out of whatever set holds it
   */
  virtual bool deleteDiscSetMember(int memberId) = 0;

  // TODO
  /**
   * The membership row holding a catalogued file, or nothing when no set has claimed it
   */
  [[nodiscard]] virtual std::optional<DiscSetMember> getDiscSetMemberForContentFile(int contentFileId) = 0;

  // TODO
  /**
   * One membership row by id
   */
  [[nodiscard]] virtual std::optional<DiscSetMember> getDiscSetMember(int memberId) = 0;

  // TODO
  /**
   * A row naming this path whose file has not been catalogued yet, which is a claim waiting for
   * the file to turn up
   */
  [[nodiscard]] virtual std::optional<DiscSetMember> getPendingDiscSetMember(const std::string &memberPath) = 0;

  /**
   * The entries pointed at a set. An absorbed disc has no entry, so a set's discs cannot be
   * walked back to find the one standing for it
   */
  [[nodiscard]] virtual std::vector<Entry> getEntriesInDiscSet(int setId) = 0;

  /**
   * Which disc a save slot was last on, or nothing when it has never been launched
   */
  [[nodiscard]] virtual std::optional<int> getLastDisc(int entryId, int saveSlot) = 0;

  virtual bool setLastDisc(int entryId, int saveSlot, int discNumber) = 0;

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
   * A net, not an answer. A title is only sometimes known and can be carried by the entry or
   * by the dumps behind it, so this returns everything either could reach and areSameGame
   * decides which of them count
   *
   * Ascending and without repeats, so a caller that stops at the first peer stops at the same
   * one every time
   */
  virtual std::vector<int> getCandidateEntryIds(const GameIdentity &identity) = 0;

  virtual std::vector<Entry> getEntriesInVariantGroup(int groupId) = 0;

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

  virtual bool deleteFolderEntry(FolderEntry &info) = 0;

  virtual bool reorderFolders(int parentId, const std::vector<int> &orderedFolderIds) = 0;

  virtual bool setFolderParent(int folderId, int newParentId) = 0;

  virtual bool deleteContentDirectory(int id) = 0;

  virtual std::optional<ContentFile> getContentFileWithPathAndSize(const std::string &filePath, size_t fileSizeBytes,
                                                                   bool inArchive) = 0;

  virtual std::vector<ContentFile> getContentFiles() = 0;

  /**
   * Every file the walk recognised, whatever it turned out to be.
   *
   * The other listings answer what stands for a game; this one answers what is on disk, so a track
   * or a playlist going missing is noticed too
   */
  [[nodiscard]] virtual std::vector<ContentFile> getRecordedFiles() = 0;

  virtual std::optional<ContentFile> getContentFile(int id) = 0;

  [[nodiscard]] virtual std::vector<ContentFile> getContentFilesWithContentHash(const std::string &contentHash) = 0;

  [[nodiscard]] virtual std::optional<ContentFile> getContentFileWithPath(const std::string &filePath) = 0;

  /**
   * Re-stamps what a rewrite changed. The row keeps its id, so the ways to launch it survive
   */
  virtual bool setContentFileIdentity(int contentFileId, const std::string &contentHash, size_t fileSizeBytes) = 0;

  /**
   * Every catalogued file whose bytes are on disk right now.
   *
   * The plain listing answers what the library knows about, which includes what has gone missing;
   * this answers what can be opened
   */
  [[nodiscard]] virtual std::vector<ContentFile> getPresentContentFiles() = 0;

  /**
   * The discs of a set whose bytes are on disk right now
   */
  [[nodiscard]] virtual std::vector<ContentFile> getPresentDiscsInSet(int setId) = 0;

  virtual bool markContentFileMissing(int id) = 0;

  /**
   * Records that a file marked missing is back
   */
  virtual bool reviveContentFile(int id) = 0;

  /**
   * Forgets a content file outright, along with its ways in and its disc members
   */
  virtual bool deleteContentFile(int id) = 0;

  virtual std::optional<PatchFile> getPatchFile(int id) = 0;

  /**
   * Every entry, with its folders, tags and content files attached. Reads and hydrates the whole
   * entries table across four queries
   */
  virtual std::vector<Entry> getEntries() = 0;

  virtual std::optional<Entry> getEntry(int entryId) = 0;

  /**
   * Removes an entry and everything hanging off it in this database
   */
  virtual bool deleteEntry(int entryId) = 0;

  virtual std::optional<Entry> getEntryWithContentHash(const std::string &contentHash) = 0;

  virtual std::vector<RunConfiguration> getRunConfigurations(const std::string &contentHash) = 0;

  /**
   * Takes away the ways in a file has of its own, leaving any belonging to a disc set.
   *
   * A set anchors its way in on one of its discs, so removing everything pointed at that disc
   * would take the whole set's way in with it
   */
  // TODO
  virtual bool deleteRunConfigurationsForContentFile(int contentFileId) = 0;

  virtual std::vector<ContentDirectory> getContentDirectories() = 0;
};
} // namespace firelight::library
