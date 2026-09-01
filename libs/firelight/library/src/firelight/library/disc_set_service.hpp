// TODO: NEEDS REVIEW
#pragma once

#include <firelight/library/disc_set.hpp>
#include <firelight/library/disc_set_playlist.hpp>
#include <firelight/library/user_library_repository.hpp>

#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace firelight::library {

// TODO
/**
 * Owns which discs make up a game.
 *
 * A disc is a content file, not an entry, so a game is one row however many discs it came on. A
 * disc finds its set as it is catalogued, and the set launches through one way in, on the disc it
 * is identified by
 */
class DiscSetService {
public:
  DiscSetService(IUserLibraryRepository &library, std::string appDataDirectory);

  ~DiscSetService() = default;

  // TODO
  /**
   * Puts a newly catalogued disc in the set it belongs to, creating one when nothing matches.
   *
   * @return The set it landed in, or nothing when the file is not a disc or nothing has named it
   */
  std::optional<int> place(const ContentFile &file);

  // TODO
  /**
   * Records the discs a user's playlist names, in the order it names them.
   *
   * A line whose file is not catalogued becomes a row holding that position until it is, and the
   * line count is written as the set's disc count
   *
   * @return The set the playlist describes
   */
  std::optional<int> claimPlaylist(const std::string &playlistPath, const std::vector<std::string> &discPaths);

  // TODO
  /**
   * Re-keys the set's entry onto its anchor when a lower-numbered disc has taken over
   *
   * @return True when the identity moved
   */
  bool syncSetEntry(int setId);

  // TODO
  /**
   * Leaves the set with one way in, on the disc it is identified by, and one entry keyed on the
   * same disc. Creates that way in when nothing stands for the set yet
   *
   * @return True when the set has an anchor to launch through
   */
  bool syncSetWayIn(int setId);

  /**
   * Writes the set's playlist when it is missing or says the wrong thing.
   *
   * The file is a render of what the database already knows, so it is safe to lose: nothing
   * refers to it and it costs a write to rebuild
   *
   * @return True when the file on disk changed
   */
  bool materializePlaylist(int setId, const std::string &contentHash);

  /**
   * Takes a disc out of its set and gives it its own entry back. Records that a person decided
   * it, so automatic grouping will not undo them
   */
  bool detachDisc(int contentFileId);

  /**
   * Hands an entry back to automatic grouping, undoing an earlier detach
   */
  bool clearUserChoice(int entryId);

private:
  // TODO
  /**
   * Whether a set could take a disc carrying this identity: compatible regions, and nothing
   * already sitting at the number
   */
  bool hasRoomFor(const DiscSet &set, const ContentFile &file, const GameIdentity &identity);

  // TODO
  /**
   * The disc a set is identified by: its lowest-numbered member that has a hash
   */
  std::optional<ContentFile> anchorOf(int setId);

  // TODO
  /**
   * Writes the membership row putting a disc in a set
   */
  // TODO
  // Gives an entry the game's name rather than its filename, read from the entries standing
  // beside it. The overload taking a set names whichever entry that set stands under
  void nameAfterPeers(const std::string &contentHash, const std::vector<Entry> &peers);

  void nameAfterPeers(int setId, const std::vector<Entry> &peers);

  void joinSet(const ContentFile &file, int setId, int discNumber, DiscSource source, const std::string &sourcePath,
               bool isUncertain);

  /** Takes a retired set's playlist off disk */
  void retirePlaylist(const std::string &contentHash);

  IUserLibraryRepository &m_library;

  std::string m_appDataDirectory;

  // TODO
  // Held across the compare and the write, so two threads deciding at once cannot both write
  std::mutex m_playlistMutex;
};

} // namespace firelight::library
